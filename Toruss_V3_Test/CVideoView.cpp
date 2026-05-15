
// CVideoView.cpp: 구현 파일
//

#include "pch.h"
#include "CVideoView.h"


// 생성자
CVideoView::CVideoView()
{
	m_pStatic = nullptr; // 초기에는 연결된 Static Control 없음
}


// Static Control 연결
void CVideoView::Attach(CStatic* pStatic)
{
	m_pStatic = pStatic;
}


// 영상 프레임 출력
void CVideoView::DrawFrame(const cv::Mat& frame)
{
	// 연결된 화면이 없으면 종료
	if (m_pStatic == nullptr)
		return;

	// 비어있는 프레임 => 종료
	if (frame.empty())
		return;

	// Static Control 크기 얻기
	CRect rect;
	m_pStatic->GetClientRect(&rect);

	if (rect.Width() <= 0 || rect.Height() <= 0)
		return;

	// 화면 크기에 맞게 리사이즈
	cv::Mat resizedFrame;
	cv::resize(
		frame,
		resizedFrame,
		cv::Size(rect.Width(), rect.Height()),
		0,
		0,
		cv::INTER_LINEAR);

	// 24bit는 한 줄 바이트 정렬 문제로 대각선 찢김 현상 발생 가능
	// 그래서, 32bit BGRA로 변환해서 출력
	cv::Mat bgraFrame;

	if (resizedFrame.channels() == 3)
	{
		// OpenCV 기본 BGR -> BGRA
		cv::cvtColor(resizedFrame, bgraFrame, cv::COLOR_BGR2BGRA);
	}
	else if (resizedFrame.channels() == 4)
	{
		// 이미 4채널이면 그대로 사용
		bgraFrame = resizedFrame.clone();
	}
	else if (resizedFrame.channels() == 1)
	{
		// 흑백 영상이면 Gray -> BGRA
		cv::cvtColor(resizedFrame, bgraFrame, cv::COLOR_GRAY2BGRA);
	}
	else
	{
		return;
	}

	// 메모리 연속성 보장
	if (!bgraFrame.isContinuous())
	{
		bgraFrame = bgraFrame.clone();
	}

	// Static Control의 Device Context 얻기
	CClientDC dc(m_pStatic);

	// 출력 품질 설정
	SetStretchBltMode(dc.GetSafeHdc(), HALFTONE);

	// Windows Bitmap 정보 구조체
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = bgraFrame.cols;

	// 음수면 Top-Down 방식 출력
	bmi.bmiHeader.biHeight = -bgraFrame.rows;

	bmi.bmiHeader.biPlanes = 1;

	// 32bit BGRA 출력
	bmi.bmiHeader.biBitCount = 32;

	// 압축 없음
	bmi.bmiHeader.biCompression = BI_RGB;

	// 실제 영상 출력
	StretchDIBits(
		dc.GetSafeHdc(),
		0,
		0,
		rect.Width(),
		rect.Height(),
		0,
		0,
		bgraFrame.cols,
		bgraFrame.rows,
		bgraFrame.data,
		&bmi,
		DIB_RGB_COLORS,
		SRCCOPY);
}