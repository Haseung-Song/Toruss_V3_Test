#include "pch.h"
#include "CMSSQLManager.h"

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
		OutputDebugString(_T("[DB] Connect 시작\r\n"));

		if (m_isConnected && m_connection != nullptr)
		{
			OutputDebugString(_T("[DB] 이미 연결됨\r\n"));
			return true;
		}

		OutputDebugString(_T("[DB] Connection CreateInstance 시작\r\n"));

		HRESULT hr =
			m_connection.CreateInstance(__uuidof(Connection));

		if (FAILED(hr))
		{
			CString msg;
			msg.Format(_T("[DB] CreateInstance 실패: 0x%08X\r\n"), hr);

			OutputDebugString(msg);
			AfxMessageBox(msg);

			return false;
		}

		OutputDebugString(_T("[DB] CreateInstance 성공\r\n"));

		_bstr_t connStr =
			L"Provider=MSOLEDBSQL;"
			L"Data Source=DESKTOP-BTEMCPL;"
			L"Initial Catalog=TorussTestDB;"
			L"Integrated Security=SSPI;"
			L"TrustServerCertificate=True;";

		OutputDebugString(_T("[DB] Open 시작\r\n"));

		m_connection->Open(connStr, L"", L"", adConnectUnspecified);

		OutputDebugString(_T("[DB] Open 성공\r\n"));

		m_isConnected = true;

		return true;
	}
	catch (_com_error& e)
	{
		CString msg;

		msg.Format(
			_T("[DB] Connect 예외 발생\n%s"),
			(LPCTSTR)e.Description());

		OutputDebugString(msg);
		AfxMessageBox(msg);

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
				m_connection->Close();
			}

			m_connection = nullptr;
		}
	}
	catch (...)
	{
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
	catch (...)
	{
		return _T("");
	}
}

// CameraId 기준 장비 프로파일 조회
bool CMSSQLManager::LoadDeviceProfile(const CString& cameraId, DeviceProfile& outProfile)
{
	try
	{
		// DB 미연결 상태면 연결 시도
		if (!IsConnected())
		{
			if (!Connect())
				return false;
		}

		// SQL Injection 방지를 위해 최소한 작은따옴표 치환
		CString safeCameraId = cameraId;
		safeCameraId.Replace(_T("'"), _T("''"));

		CString query;
		query.Format(
			_T("SELECT * FROM DeviceProfile WHERE Camera_Id = N'%s'"),
			safeCameraId.GetString());

		_RecordsetPtr recordset;
		recordset.CreateInstance(__uuidof(Recordset));

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

		return true;
	}
	catch (_com_error& e)
	{
		CString msg;
		msg.Format(_T("[CMSSQLManager] LoadDeviceProfile 실패: %s\r\n"), e.ErrorMessage());
		OutputDebugString(msg);

		return false;
	}
}