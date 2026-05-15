#pragma once

#include "pch.h"
#include "VideoFrame.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <string>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

// RTSP 영상 디코딩 클래스
class CRTSPDecoder
{
public:
	CRTSPDecoder();
	~CRTSPDecoder();

	// RTSP 열기
	bool Open(const std::string& url);

	// RTSP 닫기
	void Close();

	// FFmpeg 관련 내부 자원 정리 함수
	void Cleanup();

	// 최신프레임 가져오기
	bool GetLatestFrame(VideoFrame& outFrame);

	// 연결 여부
	bool IsOpened() const;

private:
	// 디코딩 스레드
	void DecodeThreadProc();

	// 디코딩된 프레임 처리
	bool ProcessDecodedFrame(AVFrame* frame);

private:
	// RTSP 주소
	std::string m_url;

	// 디코딩 스레드
	std::thread m_decodeThread;

	// 스레드 동작 여부
	std::atomic<bool> m_running = false;

	// 연결 여부
	std::atomic<bool> m_opened = false;

	// 최신 프레임 보호 mutex
	std::mutex m_frameMtx;

	// 최신 프레임
	VideoFrame m_latestFrame;

	// FFmpeg 객체
	AVFormatContext* m_fmtCtx = nullptr;
	AVCodecContext* m_codecCtx = nullptr;
	const AVCodec* m_codec = nullptr;

	AVPacket* m_packet = nullptr;
	AVFrame* m_decodedFrame = nullptr;
	AVFrame* m_bgrFrame = nullptr;

	SwsContext* m_swsCtx = nullptr;
	uint8_t* m_bgrBuffer = nullptr;
	int m_videoStreamIndex = -1;
	int64_t m_frameCounter = 0;
};