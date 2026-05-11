#pragma once

#include "pch.h"
#include "DeviceProfile.h"

// MSSQL 연결 및 장비 프로파일 조회 담당 클래스
class CMSSQLManager
{
public:
	CMSSQLManager();
	~CMSSQLManager();

	// DB 연결
	bool Connect();

	// DB 연결 종료
	void Close();

	// 연결 상태 확인
	bool IsConnected() const;

	// UnitId 기준 장비 프로파일 조회
	bool LoadDeviceProfile(const CString& unitId, DeviceProfile& outProfile);

private:
	// DB에서 읽은 값을 CString으로 변환
	CString GetFieldString(_RecordsetPtr recordset, const wchar_t* fieldName);

private:
	// ADO Connection 객체
	_ConnectionPtr m_connection;

	// DB 연결 여부
	bool m_isConnected;
};