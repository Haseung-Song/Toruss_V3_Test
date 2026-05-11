
// Toruss_V3_TestDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Toruss_V3_Test.h"
#include "Toruss_V3_TestDlg.h"
#include "afxdialogex.h"
#include "RtspBuilder.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTorussV3TestDlg 대화 상자

CTorussV3TestDlg::CTorussV3TestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TORUSS_V3_TEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CTorussV3TestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_COLOR_CAM, m_ColorCam); // 왼쪽 화면: EO 주간 카메라
	DDX_Control(pDX, IDC_STATIC_THERMAL_CAM, m_ThermalCam); // 오른쪽 화면: IR 열화상 카메라
}


BEGIN_MESSAGE_MAP(CTorussV3TestDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CTorussV3TestDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CTorussV3TestDlg::OnBnClickedButtonDisconnect)
END_MESSAGE_MAP()


// CTorussV3TestDlg 메시지 처리기

BOOL CTorussV3TestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다. 
	// 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	// 프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	// EO Static Control과 출력 View 연결
	m_ColorCamView.Attach(&m_ColorCam);

	// IR Static Control과 출력 View 연결
	m_ThermalCamView.Attach(&m_ThermalCam);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
// 아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
// 프레임워크에서 이 작업을 자동으로 수행합니다.

void CTorussV3TestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CTorussV3TestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTorussV3TestDlg::OnBnClickedButtonConnect()
{
	// 이미 재생 중이면 중복 실행 방지
	if (m_isPlaying) return;

	// 재생 상태 ON
	m_isPlaying = true;

	// 영상 재생은 UI 스레드가 아니라 별도 스레드에서 실행

	// 1. 테스트용 MP4 영상 출력 코드 (현재 비활성화)
	//m_videoThread = std::thread(&CTorussV3TestDlg::PlayTestVideo, this);

	// 2. 현재 선택된 카메라 모드에 따라 실행!
	// 1) 테스트 RTSP 카메라 모드
	//   - 하드코딩된 [RTSP] 주소 사용
	//   - 뒤쪽 테스트 카메라 / 단순 RTSP 연결 테스트용
	if (m_cameraMode == CameraMode::TEST_RTSP)
	{
		m_videoThread =
			std::thread(&CTorussV3TestDlg::PlayRTSPVideo, this);
	}
	// 2) 실제 장비(DeviceProfile) 기반 카메라 모드
	//   - MSSQL에서 장비 정보 조회
	//   - 제조사별 RTSP 주소 생성
	//   - [EO]/[IR] 장비 카메라 연동용
	else
	{
		m_videoThread =
			std::thread(&CTorussV3TestDlg::PlayEOIRVideo, this);
	}
}


void CTorussV3TestDlg::PlayTestVideo()
{
	// 테스트 영상(sample_h264.mp4) 파일 열기
	cv::VideoCapture cap(
		"D:\\Project\\1. C++\\Main_Project\\Toruss_V3_Test\\TestVideo\\sample_h264.mp4");

	if (!cap.isOpened())
	{
		m_isPlaying = false;
		AfxMessageBox(_T("영상 열기 실패"));
		return;
	}

	cv::Mat frame;

	// 재생 상태가 true일 때만 반복
	while (m_isPlaying)
	{
		// 프레임 읽기
		cap >> frame;

		// 영상 끝
		if (frame.empty()) break;

		// EO 화면 출력
		m_ColorCamView.DrawFrame(frame);

		// 프레임 출력 간격 조정
		Sleep(1);
	}
	cap.release(); // 영상 해제

	m_isPlaying = false; // 재생 상태 OFF
}


// 현재는 [Legacy Code]
// 단일 고정 RTSP 주소 생성 함수
// 현재는 제조사별 RTSP 경로 대응을 위해
// CRtspBuilder 구조 사용 중
#if false
CString CTorussV3TestDlg::MakeRtspUrl(const DeviceCamInfo& camInfo)
{
	CString rtspUrl;

	// RTSP 주소 생성
	// 형식:rtsp://ID:PW@IP:554/rtsp_tunnel
	rtspUrl.Format(
		_T("rtsp://%s:%s@%s:554/rtsp_tunnel"),
		camInfo.id.GetString(),   // 카메라 ID
		camInfo.pw.GetString(),   // 카메라 PW
		camInfo.ip.GetString());  // 카메라 IP

	return rtspUrl;
}

// DB에서 조회한 EO 카메라 정보로 RTSP 주소 생성
CString eoRtsp = MakeRtspUrl(profile.color);

// DB에서 조회한 EO 카메라 정보로 RTSP 주소 생성
CString irRtsp = MakeRtspUrl(profile.thermal);
// CRTSPDecoder는 std::string을 받으므로 CString -> CStringA -> std::string 변환
#endif


// [EO/IR] [RTSP] 영상 재생
void CTorussV3TestDlg::PlayEOIRVideo()
{
	// 현재 영상 스레드에서 ADO COM 사용을 위한 초기화
	HRESULT hrCo = ::CoInitialize(NULL);

	if (FAILED(hrCo))
	{
		AfxMessageBox(_T("COM 초기화 실패"));
		m_isPlaying = false; // 재생 상태 OFF
		return;
	}

	// MSSQL에서 장비 프로파일 조회
	DeviceProfile profile;

	// UNIT_0006 장비 정보 조회
	if (!m_sqlManager.LoadDeviceProfile(_T("UNIT_0006"), profile))
	{
		AfxMessageBox(_T("장비 프로파일 조회 실패"));

		::CoUninitialize();

		m_isPlaying = false; // 재생 상태 OFF
		return;
	}

	// DB에서 조회한 장비 정보 기반으로 EO RTSP 주소 생성
	CString eoRtsp = CRtspBuilder::BuildColorRtsp(profile);

	// DB에서 조회한 장비 정보 기반으로 IR RTSP 주소 생성
	CString irRtsp = CRtspBuilder::BuildThermalRtsp(profile);

	CStringA eoRtspA(eoRtsp);
	CStringA irRtspA(irRtsp);

	// EO RTSP 디코더 시작
	m_eoDecoder.Open(std::string(eoRtspA.GetString()));

	// IR RTSP 디코더 시작
	m_irDecoder.Open(std::string(irRtspA.GetString()));

	// Open()은 내부 디코딩 스레드를 시작하는 함수이므로,
	// 실제 RTSP 연결 완료까지 시간이 걸릴 수 있음.
	// 따라서 최대 3초 동안 IsOpened() 상태를 확인하며 대기 필요
	for (int i = 0; i < 30; i++)
	{
		if (m_eoDecoder.IsOpened() && m_irDecoder.IsOpened())
			break;

		Sleep(100);
	}

	// 최대 대기 시간 이후에도 열리지 않은 경우 연결 실패로 판단

	// 1. [EO] 카메라 연결 실패!
	if (!m_eoDecoder.IsOpened())
	{
		AfxMessageBox(_T("EO 카메라 연결 실패"));

		m_eoDecoder.Close();
		m_irDecoder.Close();

		m_isPlaying = false; // 재생 상태 OFF
		return;
	}

	// 2. [IR] 카메라 연결 실패!
	if (!m_irDecoder.IsOpened())
	{
		AfxMessageBox(_T("IR 카메라 연결 실패"));

		m_eoDecoder.Close();
		m_irDecoder.Close();

		m_isPlaying = false; // 재생 상태 OFF
		return;
	}

	VideoFrame eoFrame; // EO 최신 프레임 저장 변수

	VideoFrame irFrame; // IR 최신 프레임 저장 변수

	// 재생 중이면 반복
	while (m_isPlaying)
	{
		// EO 최신 프레임 가져오기
		if (m_eoDecoder.GetLatestFrame(eoFrame))
		{
			m_ColorCamView.DrawFrame(eoFrame.bgr);
		}

		// IR 최신 프레임 가져오기
		if (m_irDecoder.GetLatestFrame(irFrame))
		{
			m_ThermalCamView.DrawFrame(irFrame.bgr);
		}
		Sleep(1); // 화면 출력 과부하 방지
	}
	// EO 디코더 종료
	m_eoDecoder.Close();

	// IR 디코더 종료
	m_irDecoder.Close();

	m_isPlaying = false; // 재생 상태 OFF

	// 현재 스레드의 COM 해제
	::CoUninitialize();
}


void CTorussV3TestDlg::PlayRTSPVideo()
{
	// EO 주간 카메라 RTSP 주소
	std::string eoRtsp =
		"rtsp://service:Xhddlf1!@192.168.0.107:554/rtsp_tunnel";

	// IR 열화상 카메라 RTSP 주소
	std::string irRtsp =
		"rtsp://service:Xhddlf1!@192.168.0.107:554/rtsp_tunnel";

	// EO RTSP 디코더 시작
	m_eoDecoder.Open(eoRtsp);

	// IR RTSP 디코더 시작
	m_irDecoder.Open(irRtsp);

	// 연결 대기
	for (int i = 0; i < 30; i++)
	{
		if (m_eoDecoder.IsOpened() &&
			m_irDecoder.IsOpened())
			break;

		Sleep(100);
	}

	// EO 프레임
	VideoFrame eoFrame;

	// IR 프레임
	VideoFrame irFrame;

	// 재생 루프
	while (m_isPlaying)
	{
		if (m_eoDecoder.GetLatestFrame(eoFrame))
		{
			m_ColorCamView.DrawFrame(eoFrame.bgr);
		}

		if (m_irDecoder.GetLatestFrame(irFrame))
		{
			m_ThermalCamView.DrawFrame(irFrame.bgr);
		}
		Sleep(1);
	}
	// 종료 처리
	m_eoDecoder.Close();
	m_irDecoder.Close();

	m_isPlaying = false; // 재생 상태 OFF
}


void CTorussV3TestDlg::OnBnClickedButtonDisconnect()
{
	// 영상 재생 중이 아니면 종료
	if (!m_isPlaying) return;

	m_isPlaying = false; // 재생 상태 OFF

	// 영상 재생 스레드 종료 대기
	if (m_videoThread.joinable())
	{
		m_videoThread.join();
	}

	// EO 화면 검정색 초기화
	{
		CClientDC dc(&m_ColorCam);
		CRect rect;
		m_ColorCam.GetClientRect(&rect);
		dc.FillSolidRect(rect, RGB(0, 0, 0));
	}

	// IR 화면 검정색 초기화
	{
		CClientDC dc(&m_ThermalCam);
		CRect rect;
		m_ThermalCam.GetClientRect(&rect);
		dc.FillSolidRect(rect, RGB(0, 0, 0));
	}

}
