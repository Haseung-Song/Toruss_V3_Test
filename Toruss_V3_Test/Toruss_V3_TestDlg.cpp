
// Toruss_V3_TestDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Toruss_V3_Test.h"
#include "Toruss_V3_TestDlg.h"
#include "afxdialogex.h"

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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
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
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

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
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// 이미 재생 중이면 중복 실행 방지
	if (m_isPlaying) return;

	// 재생 상태 ON
	m_isPlaying = true;

	// 영상 재생은 UI 스레드가 아니라 별도 스레드에서 실행
	//m_videoThread = std::thread(&CTorussV3TestDlg::PlayTestVideo, this);
	m_videoThread = std::thread(&CTorussV3TestDlg::PlayEOIRVideo, this);
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

// [EO/IR] [RTSP] 영상 재생
void CTorussV3TestDlg::PlayEOIRVideo()
{
	// EO 주간 카메라 RTSP 주소
	std::string eoRtsp =
		"rtsp://service:Xhddlf1!@192.168.0.107:554/rtsp_tunnel";

	// IR 열화상 카메라 RTSP 주소
	std::string irRtsp =
		"rtsp://service:Xhddlf1!@192.168.0.107:554/rtsp_tunnel";

	// OpenCV VideoCapture 객체
	cv::VideoCapture eoCap;
	cv::VideoCapture irCap;

	// EO RTSP 연결
	eoCap.open(eoRtsp, cv::CAP_FFMPEG);
	// EO 버퍼 크기 최소화
	eoCap.set(cv::CAP_PROP_BUFFERSIZE, 1);

	// IR RTSP 연결
	irCap.open(irRtsp, cv::CAP_FFMPEG);
	// IR 버퍼 크기 최소화
	irCap.set(cv::CAP_PROP_BUFFERSIZE, 1);

	// EO 연결 실패
	if (!eoCap.isOpened())
	{
		AfxMessageBox(_T("EO 카메라 연결 실패"));

		m_isPlaying = false;
		return;
	}

	// IR 연결 실패
	if (!irCap.isOpened())
	{
		AfxMessageBox(_T("IR 카메라 연결 실패"));

		m_isPlaying = false;
		return;
	}

	// EO 프레임 저장 변수
	cv::Mat eoFrame;

	// IR 프레임 저장 변수
	cv::Mat irFrame;

	// 재생 중일 때 반복
	while (m_isPlaying)
	{
		// 최신 프레임 쪽으로 최대한 따라가기
		for (int i = 0; i < 3; i++)
		{
			eoCap.grab();
			irCap.grab();
		}

		if (eoCap.retrieve(eoFrame))
			m_ColorCamView.DrawFrame(eoFrame);

		if (irCap.retrieve(irFrame))
			m_ThermalCamView.DrawFrame(irFrame);

		Sleep(1);
	}

	// EO 영상 해제
	eoCap.release();

	// IR 영상 해제
	irCap.release();

	// 재생 상태 종료
	m_isPlaying = false;
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
