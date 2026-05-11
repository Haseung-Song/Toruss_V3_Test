#pragma once


#include "pch.h"
#include <afxstr.h>

// 카메라 접속 정보
struct DeviceCamInfo
{
	CString maker; // 제조사
	CString ip; // 카메라 IP
	CString id; // 접속 ID
	CString pw; // 접속 PW
};

// 장비 프로파일 정보
struct DeviceProfile
{
	CString unitId;		// 장비 ID
	CString siteName;	// 사이트명
	CString deviceModel;	// 장비 모델

	CString ccbip;		// CCB IP
	CString ccbport;		// CCB Port

	DeviceCamInfo color;	// EO 주간 카메라
	DeviceCamInfo thermal;	// IR 열화상 카메라

	// 초기화 코드
	void Clear()
	{
		unitId.Empty();
		siteName.Empty();
		deviceModel.Empty();
		ccbip.Empty();
		ccbport.Empty();

		color = DeviceCamInfo();
		thermal = DeviceCamInfo();
	}
};