
// CMSSQLManager.cpp: 구현 파일
//

#include "pch.h"
#include "CMSSQLManager.h"

#include <atlconv.h>

#include <iostream>


// 생성자
CMSSQLManager::CMSSQLManager()
{
	m_isConnected = false;
	m_connection = nullptr;
}


// 소멸자
CMSSQLManager::~CMSSQLManager()
{
	Close();
}


// DB 연결
bool CMSSQLManager::Connect()
{
	try
	{
		std::cout << "[DB] Connect Start" << std::endl;

		if (m_isConnected && m_connection != nullptr)
		{
			std::cout << "[DB] Already Connected" << std::endl;
			return true;
		}

		std::cout << "[DB] Connection CreateInstance Start" << std::endl;

		HRESULT hr =
			m_connection.CreateInstance(__uuidof(Connection));

		if (FAILED(hr))
		{
			std::cout << "[DB ERROR] CreateInstance Failed : 0x"
				<< std::hex
				<< hr
				<< std::dec
				<< std::endl;

			return false;
		}

		std::cout << "[DB] CreateInstance Success" << std::endl;

		_bstr_t connStr =
			L"Provider=MSOLEDBSQL;"
			L"Data Source=DESKTOP-BTEMCPL;"
			L"Initial Catalog=TorussTestDB;"
			L"Integrated Security=SSPI;"
			L"TrustServerCertificate=True;";

		std::cout << "[DB] Open Start" << std::endl;

		m_connection->Open(connStr, L"", L"", adConnectUnspecified);

		std::cout << "[DB] Open Success" << std::endl;

		m_isConnected = true;

		return true;
	}
	catch (_com_error& e)
	{
		std::cout << "[DB ERROR] Connect Exception" << std::endl;

		std::cout << "[DB ERROR] Description : "
			<< CT2A((LPCTSTR)e.Description(), CP_ACP)
			<< std::endl;

		std::cout << "[DB ERROR] Message     : "
			<< CT2A(e.ErrorMessage(), CP_ACP)
			<< std::endl;

		m_isConnected = false;
		m_connection = nullptr;

		return false;
	}
	catch (...)
	{
		std::cout << "[DB ERROR] Unknown Connect Exception" << std::endl;

		m_isConnected = false;
		m_connection = nullptr;

		return false;
	}

}


// DB 연결 종료
void CMSSQLManager::Close()
{
	try
	{
		if (m_connection != nullptr)
		{
			if (m_connection->State == adStateOpen)
			{
				std::cout << "[DB] Close Start" << std::endl;

				m_connection->Close();

				std::cout << "[DB] Close Complete" << std::endl;
			}
			m_connection = nullptr;
		}

	}
	catch (_com_error& e)
	{
		std::cout << "[DB ERROR] Close Exception" << std::endl;

		std::cout << "[DB ERROR] Description : "
			<< CT2A((LPCTSTR)e.Description(), CP_ACP)
			<< std::endl;

		std::cout << "[DB ERROR] Message     : "
			<< CT2A(e.ErrorMessage(), CP_ACP)
			<< std::endl;
	}
	catch (...)
	{
		std::cout << "[DB ERROR] Unknown Close Exception" << std::endl;
	}
	m_isConnected = false;
}


// 연결 상태 확인
bool CMSSQLManager::IsConnected() const
{
	return m_isConnected;
}


// DB 필드 값을 CString으로 변환
CString CMSSQLManager::GetFieldString(_RecordsetPtr recordset, const wchar_t* fieldName)
{
	try
	{
		_variant_t value = recordset->Fields->GetItem(fieldName)->Value;

		if (value.vt == VT_NULL || value.vt == VT_EMPTY)
			return _T("");

		return CString((LPCTSTR)(_bstr_t)value);
	}
	catch (_com_error& e)
	{
		std::cout << "[DB ERROR] GetFieldString Exception : "
			<< CT2A(fieldName, CP_ACP)
			<< std::endl;

		std::cout << "[DB ERROR] Description : "
			<< CT2A((LPCTSTR)e.Description(), CP_ACP)
			<< std::endl;

		return _T("");
	}
	catch (...)
	{
		std::cout << "[DB ERROR] Unknown GetFieldString Exception : "
			<< CT2A(fieldName, CP_ACP)
			<< std::endl;

		return _T("");
	}

}


// Camera_Id 기준 장비 프로파일 조회
bool CMSSQLManager::LoadDeviceProfile(const CString& camera_Id, DeviceProfile& outProfile)
{
	try
	{
		std::cout << "[DB] LoadDeviceProfile Start" << std::endl;

		CT2A cameraIdA(camera_Id, CP_ACP);

		std::cout << "[DB] Camera_Id : "
			<< cameraIdA
			<< std::endl;

		// DB 미연결 상태면 연결 시도
		if (!IsConnected())
		{
			std::cout << "[DB] Not Connected. Try Connect..." << std::endl;

			if (!Connect())
			{
				std::cout << "[DB ERROR] Connect Failed" << std::endl;
				return false;
			}

		}

		// SQL Injection 방지를 위해 최소한 작은따옴표 치환
		CString safeCamera_Id = camera_Id;
		safeCamera_Id.Replace(_T("'"), _T("''"));

		CString query;
		query.Format(
			_T("SELECT * FROM DeviceProfile WHERE Camera_Id = N'%s'"),
			safeCamera_Id.GetString());

		CT2A queryA(query, CP_ACP);

		std::cout << "[DB] Query : "
			<< queryA
			<< std::endl;

		_RecordsetPtr recordset;
		HRESULT hr = recordset.CreateInstance(__uuidof(Recordset));

		if (FAILED(hr))
		{
			std::cout << "[DB ERROR] Recordset CreateInstance Failed : 0x"
				<< std::hex
				<< hr
				<< std::dec
				<< std::endl;

			return false;
		}

		// 쿼리 실행
		recordset->Open(
			_bstr_t(query),
			_variant_t((IDispatch*)m_connection, true),
			adOpenForwardOnly,
			adLockReadOnly,
			adCmdText);

		// 조회 결과 없음
		if (recordset->ADOEOF)
		{
			std::cout << "[DB ERROR] DeviceProfile Not Found" << std::endl;

			recordset->Close();

			return false;
		}

		// 기본 장비 정보
		outProfile.camera_Id = GetFieldString(recordset, L"Camera_Id");
		outProfile.site_Name = GetFieldString(recordset, L"Site_Name");
		outProfile.device_Model = GetFieldString(recordset, L"Device_Model");

		outProfile.ccb_Ip = GetFieldString(recordset, L"Ccb_Ip");
		outProfile.ccb_Port = GetFieldString(recordset, L"Ccb_Port");

		// EO / Color 카메라 정보
		outProfile.color.maker = GetFieldString(recordset, L"Color_Maker");
		outProfile.color.ip = GetFieldString(recordset, L"Color_Ip");
		outProfile.color.id = GetFieldString(recordset, L"Color_Id");
		outProfile.color.pw = GetFieldString(recordset, L"Color_Pw");

		// IR / Thermal 카메라 정보
		outProfile.thermal.maker = GetFieldString(recordset, L"Thermal_Maker");
		outProfile.thermal.ip = GetFieldString(recordset, L"Thermal_Ip");
		outProfile.thermal.id = GetFieldString(recordset, L"Thermal_Id");
		outProfile.thermal.pw = GetFieldString(recordset, L"Thermal_Pw");

		recordset->Close();

		std::cout << "[DB] LoadDeviceProfile Success" << std::endl;

		std::cout << "========================================" << std::endl;

		return true;
	}
	catch (_com_error& e)
	{
		std::cout << "[DB ERROR] LoadDeviceProfile Exception" << std::endl;

		std::cout << "[DB ERROR] Description : "
			<< CT2A((LPCTSTR)e.Description(), CP_ACP)
			<< std::endl;

		std::cout << "[DB ERROR] Message     : "
			<< CT2A(e.ErrorMessage(), CP_ACP)
			<< std::endl;

		return false;
	}
	catch (...)
	{
		std::cout << "[DB ERROR] Unknown LoadDeviceProfile Exception" << std::endl;

		return false;
	}

}