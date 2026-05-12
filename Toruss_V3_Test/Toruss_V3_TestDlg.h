
// Toruss_V3_TestDlg.h: 헤더 파일
//

#pragma once

#include "CVideoView.h"
#include <thread>
#include <atomic>
#include "CRTSPDecoder.h"
#include "CMSSQLManager.h"

// [카메라] 연결 모드
//enum class CameraMode
//{
//	TEST_RTSP,		 // 단순 테스트 RTSP
//	DEVICE_PROFILE	 // MSSQL DeviceProfile 기반
//};

// CTorussV3TestDlg 대화 상자
class CTorussV3TestDlg : public CDialogEx
{
	// 생성입니다.
public:
	CTorussV3TestDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TORUSS_V3_TEST_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


	// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

public:
	// 카메라 선택 ComboBox의 글자 크기 및 글꼴 조절용 Font 객체
	CFont m_fontCombo;

	// "장비 선택" Static Text 라벨의 글자 크기 및 글꼴 조절용 Font 객체
	CFont m_fontLabel;

	// 카메라 선택 영역 제목 표시용 Static Text 컨트롤
	// 예: "장비 선택"
	CStatic m_labelCamera;

public:
	// 왼쪽   EO 카메라 출력 영역
	CStatic m_ColorCam;

	// 오른쪽 IR 카메라 출력 영역
	CStatic m_ThermalCam;

	CComboBox m_comboCamera; // 카메라 선택 ComboBox

	// 현재 선택된 카메라 인덱스 (-1: 선택 없음)
	int nSel = -1;

private:
	// EO 화면 출력 담당 클래스
	CVideoView m_ColorCamView;

	// IR 화면 출력 담당 클래스
	CVideoView m_ThermalCamView;

	// EO RTSP 디코더
	CRTSPDecoder m_eoDecoder;

	// IR RTSP 디코더
	CRTSPDecoder m_irDecoder;

private:
	std::thread m_videoThread; // 영상 재생을 위한 별도 스레드
	bool m_isPlaying = false; // 영상 재생 상태 플래그

	CMSSQLManager m_sqlManager;

	void PlayTestVideo(); // 테스트 영상 출력 함수
	void PlayRTSPVideo(); // [RTSP] 카메라 영상 출력 함수

	void PlayEOIRVideo(); // [EOIR] 영상 출력 함수
	void ClearEOIRView(); // [EOIR] 및 [RTSP] 영상 초기화 및 검은 화면 출력 함수
	void StopEOIRVideo(); // [EOIR] 영상 종료 함수

public:
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();

};
