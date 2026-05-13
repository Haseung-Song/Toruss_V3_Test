#pragma once

#include "pch.h"
#include <opencv2/opencv.hpp>

// RTSP 디코더에서 넘겨주는 최신 프레임 정보
struct VideoFrame
{
	// BGR 영상 프레임
	cv::Mat bgr;

	// 영상 너비
	int width = 0;

	// 영상 높이
	int height = 0;

	// FFmpeg PTS
	int64_t pts = 0;

	// 프레임 번호
	int64_t frameNumber = 0;

	// 유효 프레임 여부
	bool IsValid() const
	{
		return !bgr.empty();
	}

	// 프레임 초기화
	void Clear()
	{
		bgr.release();
		width = 0;
		height = 0;
		pts = 0;
		frameNumber = 0;
	}

};