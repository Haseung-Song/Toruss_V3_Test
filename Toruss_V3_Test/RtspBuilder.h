#pragma once

#include "pch.h"
#include "DeviceProfile.h"

// 카메라 제조사별 RTSP 주소 생성 담당 클래스
class CRtspBuilder
{
public:
	// 공통 RTSP 주소 생성
	static CString MakeRtsp(const DeviceCamInfo& cam, const CString& path);

	// EO 주간 카메라 RTSP 주소 생성
	static CString BuildColorRtsp(const DeviceProfile& profile);

	// IR 열화상 카메라 RTSP 주소 생성
	static CString BuildThermalRtsp(const DeviceProfile& profile);
};