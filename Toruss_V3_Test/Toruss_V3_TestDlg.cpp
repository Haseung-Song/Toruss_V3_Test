
// Toruss_V3_TestDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Toruss_V3_Test.h"
#include "Toruss_V3_TestDlg.h"

#include "afxdialogex.h"
#include "RtspBuilder.h"

#include <iostream>
#include <atlconv.h>

#include <opencv2/core/utils/logger.hpp>


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

	DDX_Control(pDX, IDC_COMBO_CAMERA, m_comboCamera); // 카메라 선택 ComboBox
	DDX_Control(pDX, IDC_STATIC_CAMERA_LABEL, m_labelCamera); // "장비 선택" Static Text 라벨
}


BEGIN_MESSAGE_MAP(CTorussV3TestDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()

	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CTorussV3TestDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CTorussV3TestDlg::OnBnClickedButtonDisconnect)
END_MESSAGE_MAP()


// CTorussV3TestDlg 메시지 처리기

BOOL CTorussV3TestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// OpenCV 로그 출력 완전 비활성화
	cv::utils::logging::setLogLevel(
		cv::utils::logging::LOG_LEVEL_SILENT);

	// FFmpeg 로그 레벨 최소화
	_putenv_s("OPENCV_FFMPEG_DEBUG", "0");
	_putenv_s("OPENCV_FFMPEG_LOGLEVEL", "quiet");

	// 콘솔 한글 출력 코드페이지 설정
	SetConsoleOutputCP(949);
	SetConsoleCP(949);

	// Debug 모드에서만 콘솔 창 생성 및 연결
#ifdef _DEBUG

	// 콘솔 창 생성
	AllocConsole();

	// [stdout] / [stderr] / [stdin] 연결
	FILE* fpOut = nullptr;
	FILE* fpErr = nullptr;
	FILE* fpIn = nullptr;

	// [stdout] → 콘솔 출력
	freopen_s(&fpOut, "CONOUT$", "w", stdout);

	// [stderr] → 버림 (FFmpeg/OpenCV Warning 숨김)
	freopen_s(&fpErr, "NUL", "w", stderr);

	// [stdin] → 콘솔 입력
	freopen_s(&fpIn, "CONIN$", "r", stdin);

	// 콘솔 제목 설정
	SetConsoleTitle(_T("Toruss_V3 Debug Console"));

	std::cout << "=== TORUSS_V3 Console Start ===" << std::endl;

#endif

	// 이 대화 상자의 아이콘을 설정합니다. 
	// 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	// 프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	m_fontCombo.CreatePointFont(100, _T("맑은 고딕")); // 10pt
	m_comboCamera.SetFont(&m_fontCombo);
	m_comboCamera.SetWindowPos(
		NULL,
		0, 0,
		150, 50,   // Width, Height
		SWP_NOMOVE | SWP_NOZORDER);

	m_fontLabel.CreatePointFont(100, _T("맑은 고딕")); // 10pt
	m_labelCamera.SetFont(&m_fontLabel);

	// EO Static Control과 출력 View 연결
	m_ColorCamView.Attach(&m_ColorCam);

	// IR Static Control과 출력 View 연결
	m_ThermalCamView.Attach(&m_ThermalCam);

	// 테스트용 [RTSP] 카메라 (RTSP 주소 사용)
	m_comboCamera.AddString(_T(" Camera_00"));

	// EO/IR 장비 Camera_01 (지역: 가거도)
	m_comboCamera.AddString(_T(" Camera_01"));

	// EO/IR 장비 Camera_02 (지역: 울릉도)
	m_comboCamera.AddString(_T(" Camera_02"));

	// EO/IR 장비 Camera_03 (지역: 제주도)
	m_comboCamera.AddString(_T(" Camera_03"));

	m_comboCamera.SetCurSel(0); // 기본 선택값: Camera_00

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

// 소멸자 및 리소스 정리
void CTorussV3TestDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	m_isPlaying = false; // 재생 상태 OFF

	// 영상 스레드 종료 대기
	if (m_videoThread.joinable())
	{
		m_videoThread.join();
	}

	m_eoDecoder.Close(); // EO 디코더 종료
	m_irDecoder.Close(); // IR 디코더 종료

#ifdef _DEBUG
	FreeConsole(); // 현재 프로그램에 연결된 콘솔 창 해제(닫기)
#endif

}


// CONNECT 버튼 클릭
void CTorussV3TestDlg::OnBnClickedButtonConnect()
{
	std::cout << "========================================" << std::endl;

	std::cout << "[CONNECT BUTTON CLICK]" << std::endl;

	std::cout << "" << std::endl;

	// 이미 연결/재생 중이면 중복 Connect 방지
	if (m_isPlaying)
	{
		std::cout << "[VIDEO] Already Connecting Or Playing." << std::endl;
		return;
	}

	nSel = m_comboCamera.GetCurSel(); // 현재 ComboBox에서 선택된 카메라 인덱스 가져오기

	m_isPlaying = true; // 새 재생 시작

	std::cout << "[Camera Select] nSel = "
		<< nSel
		<< std::endl
		<< std::endl;

	// 영상 재생은 UI 스레드가 아니라 별도 스레드에서 실행

	// 1. 테스트용 MP4 영상 출력 코드 (현재 비활성화)
	//m_videoThread = std::thread(&CTorussV3TestDlg::PlayTestVideo, this);

	// 2. 현재 선택된 카메라 모드에 따라 실행!
	// 1) 테스트 RTSP 카메라 모드
	//   - 하드코딩된 [RTSP] 주소 사용
	//   - 뒤쪽 테스트 카메라 / 단순 RTSP 연결 테스트용
	if (nSel == 0)
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
		m_isPlaying = false; // 재생 상태 OFF
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

	CString strCameraName;

	nSel = m_comboCamera.GetCurSel(); // 현재 ComboBox에서 선택된 카메라 인덱스 가져오기

	switch (nSel)
	{
	case 1:
		strCameraName = _T("Camera_01");
		break;

	case 2:
		strCameraName = _T("Camera_02");
		break;

	case 3:
		strCameraName = _T("Camera_03");
		break;

	default:
		AfxMessageBox(_T("카메라를 반드시 선택해주세요."));

		m_isPlaying = false; // 재생 상태 OFF

		// 현재 스레드에서 사용한 COM 시스템 정리(해제)
		::CoUninitialize();
		return;
	}

	// CString(strCameraName)을
	// ANSI(char*) 문자열로 변환해서
	// selectedCamera에 저장
	CT2A selectedCamera(strCameraName, CP_ACP);

	// 현재 선택된 Camera 이름 로그 출력
	std::cout << "[Selected Camera] "
		<< selectedCamera
		<< std::endl;

	std::cout << "========================================" << std::endl;

	// 선택된 Camera_xx 장비 정보 조회
	if (!m_sqlManager.LoadDeviceProfile(strCameraName, profile))
	{
		AfxMessageBox(_T("장비 프로파일 조회 실패"));

		// 현재 스레드에서 사용한 COM 시스템 정리(해제)
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

	bool bEOOpened = m_eoDecoder.IsOpened(); // EO RTSP 연결 성공 여부 확인
	bool bIROpened = m_irDecoder.IsOpened(); // IR RTSP 연결 성공 여부 확인

	// EO 또는 IR 중 하나라도 연결 실패 시,
	// 전체 RTSP 연결 실패로 판단하여 재생 종료 처리
	if (!bEOOpened || !bIROpened)
	{
		AfxMessageBox(_T("EO/IR 카메라 연결 실패"));

		std::cout << "========================================" << std::endl;

		std::cout << "[RTSP CONNECTION FAILED]" << std::endl;

		StopEOIRVideo();
		ClearEOIRView();

		return;
	}

	std::cout << "========================================" << std::endl;

	std::cout << "[EO / IR Device Profile Information]" << std::endl;

	std::cout << " " << std::endl;

	CT2A cameraId(profile.camera_Id, CP_ACP);
	CT2A siteName(profile.site_Name, CP_ACP);
	CT2A deviceModel(profile.device_Model, CP_ACP);
	CT2A ccbIp(profile.ccb_Ip, CP_ACP);
	CT2A ccbPort(profile.ccb_Port, CP_ACP);

	CT2A colorMaker(profile.color.maker, CP_ACP);
	CT2A colorIp(profile.color.ip, CP_ACP);

	CT2A thermalMaker(profile.thermal.maker, CP_ACP);
	CT2A thermalIp(profile.thermal.ip, CP_ACP);

	std::cout << "1. Camera_Id     : " << cameraId << std::endl;
	std::cout << "2. Site_Name     : " << siteName << std::endl;
	std::cout << "3. Device_Model  : " << deviceModel << std::endl;
	std::cout << "4. Ccb_Ip        : " << ccbIp << std::endl;
	std::cout << "5. Ccb_Port      : " << ccbPort << std::endl;
	std::cout << "6. Color_Maker   : " << colorMaker << std::endl;
	std::cout << "7. Color_Ip      : " << colorIp << std::endl;
	std::cout << "8. Thermal_Maker : " << thermalMaker << std::endl;
	std::cout << "9. Thermal_Ip    : " << thermalIp << std::endl;

	std::cout << " " << std::endl;

	std::cout << "[CONNECTED COMPLETE!]" << std::endl;

	std::cout << "========================================" << std::endl;

	VideoFrame eoFrame; // EO 최신 프레임 저장 변수

	VideoFrame irFrame; // IR 최신 프레임 저장 변수

	// 재생 중이면 반복
	while (m_isPlaying)
	{
		// EO 최신 프레임 가져오기
		if (m_eoDecoder.IsOpened() && m_eoDecoder.GetLatestFrame(eoFrame))
		{
			m_ColorCamView.DrawFrame(eoFrame.bgr);
		}

		// IR 최신 프레임 가져오기
		if (m_irDecoder.IsOpened() && m_irDecoder.GetLatestFrame(irFrame))
		{
			m_ThermalCamView.DrawFrame(irFrame.bgr);
		}
		Sleep(1); // 화면 출력 과부하 방지
	}
	StopEOIRVideo(); // [EOIR] 영상 종료 함수

	ClearEOIRView(); // [EOIR] 및 [RTSP] 영상 초기화 및 검은 화면 출력 함수
}

// 테스트 카메라 => [RTSP] 영상 재생
void CTorussV3TestDlg::PlayRTSPVideo()
{
	// 테스트 카메라1 RTSP 주소
	std::string test1Rtsp =
		"rtsp://service:Xhddlf1!@192.168.0.107:554/rtsp_tunnel";

	// 테스트 카메라2 RTSP 주소
	std::string test2Rtsp =
		"rtsp://service:Xhddlf1!@192.168.0.107:554/rtsp_tunnel";

	std::cout << "[Test Camera 1 & 2 RTSP Connect Try...]" << std::endl;

	std::cout << " " << std::endl;

	// 테스트 카메라1 RTSP 디코더 시작
	m_eoDecoder.Open(test1Rtsp);

	// 테스트 카메라2 RTSP 디코더 시작
	m_irDecoder.Open(test2Rtsp);

	// 연결 대기 // 최대 5초 대기 필요
	for (int i = 0; i < 50; i++)
	{
		if (m_eoDecoder.IsOpened() &&
			m_irDecoder.IsOpened())
			break;

		Sleep(100);
	}

	// 테스트 카메라 1 RTSP 연결 상태 확인
	if (m_eoDecoder.IsOpened())
	{
		std::cout << "[Test Camera 1 RTSP Connect Success!]" << std::endl;
	}
	else
	{
		std::cout << "[Test Camera 1 RTSP Connect Fail! Check it out.]" << std::endl;
	}

	// 테스트 카메라 2 RTSP 연결 상태 확인
	if (m_irDecoder.IsOpened())
	{
		std::cout << "[Test Camera 2 RTSP Connect Success!]" << std::endl;
	}
	else
	{
		std::cout << "[Test Camera 2 RTSP Connect Fail! Check it out.]" << std::endl;
	}

	// test1 프레임
	VideoFrame test1Frame;

	// test2 프레임
	VideoFrame test2Frame;

	// 재생 루프
	while (m_isPlaying)
	{
		if (m_eoDecoder.IsOpened() && m_eoDecoder.GetLatestFrame(test1Frame))
		{
			m_ColorCamView.DrawFrame(test1Frame.bgr);
		}

		if (m_irDecoder.IsOpened() && m_irDecoder.GetLatestFrame(test2Frame))
		{
			m_ThermalCamView.DrawFrame(test2Frame.bgr);
		}
		Sleep(1);
	}
	// 종료 처리
	m_eoDecoder.Close();
	m_irDecoder.Close();

	m_isPlaying = false; // 재생 상태 OFF

	ClearEOIRView(); // [EOIR] 및 [RTSP] 영상 초기화 및 검은 화면 출력 함수
}


// DISCONNECT 버튼 클릭
void CTorussV3TestDlg::OnBnClickedButtonDisconnect()
{
	std::cout << "========================================" << std::endl;

	std::cout << "[DISCONNECT BUTTON CLICK]" << std::endl;

	std::cout << " " << std::endl;

	m_isPlaying = false; // 재생 상태 OFF

	// 영상 스레드 종료 대기
	if (m_videoThread.joinable())
	{
		m_videoThread.join();
	}
	std::cout << "========================================" << std::endl;
}


void CTorussV3TestDlg::ClearEOIRView()
{
	// [EO] 화면 검정색 초기화
	{
		CClientDC dc(&m_ColorCam);
		CRect rect;

		m_ColorCam.GetClientRect(&rect);
		dc.FillSolidRect(rect, RGB(0, 0, 0));

		std::cout << "[EO VIEW] Clear Complete" << std::endl;
	}

	// [IR] 화면 검정색 초기화
	{
		CClientDC dc(&m_ThermalCam);
		CRect rect;

		m_ThermalCam.GetClientRect(&rect);
		dc.FillSolidRect(rect, RGB(0, 0, 0));

		std::cout << "[IR VIEW] Clear Complete" << std::endl;
	}
	std::cout << " " << std::endl;

	std::cout << "[DISCONNECT COMPLETE]" << std::endl;
}


// [EOIR] 영상 종료 함수
void CTorussV3TestDlg::StopEOIRVideo()
{
	m_eoDecoder.Close(); // EO 디코더 종료
	m_irDecoder.Close(); // IR 디코더 종료

	m_isPlaying = false; // 재생 상태 OFF

	// 현재 스레드에서 사용한 COM 시스템 정리(해제)
	::CoUninitialize();
}