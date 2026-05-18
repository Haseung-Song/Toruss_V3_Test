
// CRTSPDecoder.cpp: 구현 파일
//

#include "pch.h"
#include "CRTSPDecoder.h"

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avutil.lib")

// 생성자
CRTSPDecoder::CRTSPDecoder()
{
	// FFmpeg 네트워크 기능 초기화
	avformat_network_init();
}


// 소멸자
CRTSPDecoder::~CRTSPDecoder()
{
	Close();

	// FFmpeg 네트워크 기능 해제
	avformat_network_deinit();
}


// RTSP 열기
bool CRTSPDecoder::Open(const std::string& url)
{
	// 이전 [Decode Thread] 객체가 아직 정리되지 않은 상태이면 새 [Open] 방지
	// [join() 함수]를 여기서 호출하면 [UI]가 멈출 수 있으므로 [false]만 반환
	if (m_decodeThread.joinable())
	{
		std::cout << "[DECODER] Previous Decode Thread Exists." << std::endl;

		std::cout << "" << std::endl;
		return false;
	}
	m_url = url;
	m_running = true;
	m_opened = false;
	m_frameCounter = 0;

	// 디코딩 스레드 시작
	m_decodeThread = std::thread(&CRTSPDecoder::DecodeThreadProc, this);

	return true;
}


// RTSP 닫기
void CRTSPDecoder::Close()
{
	// 스레드 루프 종료 요청
	m_running = false;

	// 디코딩 스레드가 [FFmpeg] 내부 [read]에서 대기 중일 수 있으므로
	// [UI] 멈춤 방지를 위해 [join] 대신 [detach] 처리 (임시 방편!!!)
	if (m_decodeThread.joinable())
	{
		m_decodeThread.detach();
	}

	// 최신 프레임 초기화
	{
		std::lock_guard<std::mutex> lock(m_frameMtx);
		m_latestFrame.Clear();
	}
	m_opened = false;
}


// FFmpeg 내부 자원 정리
void CRTSPDecoder::Cleanup()
{
	if (m_bgrBuffer)
	{
		av_free(m_bgrBuffer);
		m_bgrBuffer = nullptr;
	}

	if (m_bgrFrame)
	{
		av_frame_free(&m_bgrFrame);
	}

	if (m_decodedFrame)
	{
		av_frame_free(&m_decodedFrame);
	}

	if (m_packet)
	{
		av_packet_free(&m_packet);
	}

	if (m_swsCtx)
	{
		sws_freeContext(m_swsCtx);
		m_swsCtx = nullptr;
	}

	if (m_codecCtx)
	{
		avcodec_free_context(&m_codecCtx);
	}

	if (m_fmtCtx)
	{
		avformat_close_input(&m_fmtCtx);
	}
	m_codec = nullptr;
	m_videoStreamIndex = -1;
	m_opened = false;
}


// 최신 프레임 가져오기
bool CRTSPDecoder::GetLatestFrame(VideoFrame& outFrame)
{
	std::lock_guard<std::mutex> lock(m_frameMtx);

	if (!m_latestFrame.IsValid())
		return false;

	// 외부에서 안전하게 쓰기 위해 clone
	outFrame.bgr = m_latestFrame.bgr.clone();
	outFrame.width = m_latestFrame.width;
	outFrame.height = m_latestFrame.height;
	outFrame.pts = m_latestFrame.pts;
	outFrame.frameNumber = m_latestFrame.frameNumber;

	return true;
}


// 연결 여부
bool CRTSPDecoder::IsOpened() const
{
	return m_opened;
}


// 디코딩 스레드
void CRTSPDecoder::DecodeThreadProc()
{
	AVDictionary* options = nullptr;

	// RTSP는 TCP 방식으로 연결
	av_dict_set(&options, "rtsp_transport", "tcp", 0);

	// 저지연 옵션
	av_dict_set(&options, "fflags", "nobuffer", 0);
	av_dict_set(&options, "flags", "low_delay", 0);
	av_dict_set(&options, "max_delay", "0", 0);

	// 연결 timeout: microseconds
	av_dict_set(&options, "stimeout", "3000000", 0);
	av_dict_set(&options, "rw_timeout", "3000000", 0);

	// Format Context 생성
	m_fmtCtx = avformat_alloc_context();

	if (m_fmtCtx == nullptr)
	{
		av_dict_free(&options);
		m_running = false;
		m_opened = false;

		return;
	}

	// RTSP 열기
	int ret = avformat_open_input(&m_fmtCtx, m_url.c_str(), nullptr, &options);

	av_dict_free(&options);

	if (ret < 0)
	{
		OutputDebugString(L"[CRTSPDecoder] avformat_open_input 실패\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	// 스트림 정보 읽기
	ret = avformat_find_stream_info(m_fmtCtx, nullptr);

	if (ret < 0)
	{
		OutputDebugString(L"[CRTSPDecoder] avformat_find_stream_info 실패\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	// 비디오 스트림 찾기
	m_videoStreamIndex = -1;

	for (unsigned int i = 0; i < m_fmtCtx->nb_streams; i++)
	{
		if (m_fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_videoStreamIndex = static_cast<int>(i);
			break;
		}
	}

	if (m_videoStreamIndex < 0)
	{
		OutputDebugString(L"[CRTSPDecoder] 비디오 스트림 없음\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	// 코덱 파라미터 가져오기
	AVCodecParameters* codecParam =
		m_fmtCtx->streams[m_videoStreamIndex]->codecpar;

	// 디코더 찾기
	m_codec = avcodec_find_decoder(codecParam->codec_id);

	if (m_codec == nullptr)
	{
		OutputDebugString(L"[CRTSPDecoder] 디코더 찾기 실패\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	// 코덱 컨텍스트 생성
	m_codecCtx = avcodec_alloc_context3(m_codec);

	if (m_codecCtx == nullptr)
	{
		OutputDebugString(L"[CRTSPDecoder] 코덱 컨텍스트 생성 실패\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	// 코덱 파라미터를 코덱 컨텍스트로 복사
	ret = avcodec_parameters_to_context(m_codecCtx, codecParam);

	if (ret < 0)
	{
		OutputDebugString(L"[CRTSPDecoder] 코덱 파라미터 복사 실패\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	// 디코더 열기
	ret = avcodec_open2(m_codecCtx, m_codec, nullptr);

	if (ret < 0)
	{
		OutputDebugString(L"[CRTSPDecoder] 디코더 열기 실패\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	// 패킷 / 프레임 할당
	m_packet = av_packet_alloc();
	m_decodedFrame = av_frame_alloc();
	m_bgrFrame = av_frame_alloc();

	if (m_packet == nullptr || m_decodedFrame == nullptr || m_bgrFrame == nullptr)
	{
		OutputDebugString(L"[CRTSPDecoder] 패킷/프레임 할당 실패\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	int width = m_codecCtx->width;
	int height = m_codecCtx->height;

	if (width <= 0 || height <= 0)
	{
		OutputDebugString(L"[CRTSPDecoder] 영상 크기 오류\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	// BGR24 버퍼 크기 계산
	int bgrBufferSize = av_image_get_buffer_size(
		AV_PIX_FMT_BGR24,
		width,
		height,
		1);

	if (bgrBufferSize <= 0)
	{
		OutputDebugString(L"[CRTSPDecoder] BGR 버퍼 크기 계산 실패\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	// BGR24 버퍼 할당
	m_bgrBuffer = static_cast<uint8_t*>(av_malloc(bgrBufferSize));

	if (m_bgrBuffer == nullptr)
	{
		OutputDebugString(L"[CRTSPDecoder] BGR 버퍼 할당 실패\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	// BGR 프레임에 버퍼 연결
	ret = av_image_fill_arrays(
		m_bgrFrame->data,
		m_bgrFrame->linesize,
		m_bgrBuffer,
		AV_PIX_FMT_BGR24,
		width,
		height,
		1);

	if (ret < 0)
	{
		OutputDebugString(L"[CRTSPDecoder] av_image_fill_arrays 실패\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	// 색상 변환 Context 생성
	m_swsCtx = sws_getContext(
		width,
		height,
		m_codecCtx->pix_fmt,
		width,
		height,
		AV_PIX_FMT_BGR24,
		SWS_FAST_BILINEAR,
		nullptr,
		nullptr,
		nullptr);

	if (m_swsCtx == nullptr)
	{
		OutputDebugString(L"[CRTSPDecoder] sws_getContext 실패\r\n");

		Cleanup();
		m_running = false;
		m_opened = false;

		return;
	}

	m_opened = true;

	// 디코딩 루프
	while (m_running)
	{
		// 패킷 읽기
		ret = av_read_frame(m_fmtCtx, m_packet);

		if (ret < 0)
		{
			Sleep(1);
			continue;
		}

		// 비디오 스트림이 아니면 버림
		if (m_packet->stream_index != m_videoStreamIndex)
		{
			av_packet_unref(m_packet);
			continue;
		}

		// 패킷을 디코더로 전달
		ret = avcodec_send_packet(m_codecCtx, m_packet);

		av_packet_unref(m_packet);

		if (ret < 0)
			continue;

		// 디코딩된 프레임 수신
		while (ret >= 0)
		{
			ret = avcodec_receive_frame(m_codecCtx, m_decodedFrame);

			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				break;

			if (ret < 0)
				break;

			// 디코딩된 프레임 처리
			ProcessDecodedFrame(m_decodedFrame);
		}

	}
	Cleanup(); // 루프 종료 후 자원 해제
}


// 디코딩된 프레임 처리
bool CRTSPDecoder::ProcessDecodedFrame(AVFrame* frame)
{
	if (frame == nullptr || m_swsCtx == nullptr)
		return false;

	// 원본 픽셀 포맷 → BGR24 변환
	sws_scale(
		m_swsCtx,
		frame->data,
		frame->linesize,
		0,
		m_codecCtx->height,
		m_bgrFrame->data,
		m_bgrFrame->linesize);

	// FFmpeg BGR 버퍼를 OpenCV Mat으로 감싸기
	cv::Mat temp(
		m_codecCtx->height,
		m_codecCtx->width,
		CV_8UC3,
		m_bgrFrame->data[0],
		m_bgrFrame->linesize[0]);

	VideoFrame videoFrame;

	// 최신 프레임 저장용 clone
	videoFrame.bgr = temp.clone();
	videoFrame.width = temp.cols;
	videoFrame.height = temp.rows;
	videoFrame.pts = frame->pts;
	videoFrame.frameNumber = ++m_frameCounter;

	// 최신 프레임만 갱신
	{
		std::lock_guard<std::mutex> lock(m_frameMtx);
		m_latestFrame = videoFrame;
	}
	return true;
}