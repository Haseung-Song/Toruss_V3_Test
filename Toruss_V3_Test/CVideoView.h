#pragma once

// MFC 기본 클래스(CStatic 등) 사용
#include "pch.h"


// OpenCV 사용
#include <opencv2/opencv.hpp>


// 영상 출력 담당 클래스
class CVideoView
{
public:

	// 생성자
	CVideoView();

	// Static Control 연결
	// ex) IDC_STATIC_COLOR_CAM
	void Attach(CStatic* pStatic);

	// OpenCV 프레임을 Static Control에 출력
	void DrawFrame(const cv::Mat& frame);

private:

	// 출력 대상 Static Control
	CStatic* m_pStatic;
};