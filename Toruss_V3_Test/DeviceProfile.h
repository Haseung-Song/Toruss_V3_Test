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
	CString camera_Id;		// 장비 ID
	CString site_Name;	    // 사이트명
	CString device_Model;	// 장비 모델

	CString ccb_Ip;		    // CCB IP
	CString ccb_Port;		// CCB Port

	DeviceCamInfo color;	// EO 주간 카메라
	DeviceCamInfo thermal;	// IR 열화상 카메라

	// 초기화 코드
	void Clear()
	{
		camera_Id.Empty();
		site_Name.Empty();
		device_Model.Empty();

		ccb_Ip.Empty();
		ccb_Port.Empty();

		color = DeviceCamInfo();
		thermal = DeviceCamInfo();
	}

};