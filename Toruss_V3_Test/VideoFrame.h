#pragma once

#include "pch.h"
#include <opencv2/opencv.hpp>

// RTSP Decoder에서 수신한 영상 프레임 정보
struct VideoFrame
{
	// 실제 영상 프레임
	// OpenCV 기본 색상 포맷은 BGR
	cv::Mat bgr;

	// 프레임 번호
	// 디버깅 또는 최신 프레임 갱신 확인용
	int frameIndex = 0;

	// 프레임 유효 여부 확인
	bool IsValid() const
	{
		return !bgr.empty();
	}

	// 프레임 데이터 초기화
	void Clear()
	{
		bgr.release();
		frameIndex = 0;
	}
};