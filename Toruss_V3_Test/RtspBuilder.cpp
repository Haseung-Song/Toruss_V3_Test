#include "pch.h"
#include "RtspBuilder.h"

// 공통 RTSP 주소 생성
CString CRtspBuilder::MakeRtsp(const DeviceCamInfo& cam, const CString& path)
{
	CString url;

	// 최종 RTSP 주소 형식:
	// rtsp://ID:PW@IP:554/PATH
	url.Format(
		_T("rtsp://%s:%s@%s:%d/%s"),
		cam.id.GetString(),
		cam.pw.GetString(),
		cam.ip.GetString(),
		554,
		path.GetString());

	return url;
}

// EO 주간 카메라 RTSP 주소 생성
CString CRtspBuilder::BuildColorRtsp(const DeviceProfile& profile)
{
	CString maker(profile.color.maker);

	// Bosch EO 카메라
	if (maker.CompareNoCase(_T("Bosch")) == 0)
		return MakeRtsp(profile.color, _T("rtsp_tunnel"));

	// 한화비전 EO 카메라
	if (maker.CompareNoCase(_T("한화비전")) == 0)
		return MakeRtsp(profile.color, _T("profile2/media.smp"));

	// SONY 또는 기타 EO 카메라 기본값
	return MakeRtsp(profile.color, _T("profile2/media.smp"));
}

// IR 열화상 카메라 RTSP 주소 생성
CString CRtspBuilder::BuildThermalRtsp(const DeviceProfile& profile)
{
	CString maker(profile.thermal.maker);

	// FLIR 열화상 카메라
	if (maker.CompareNoCase(_T("FLIR")) == 0)
		return MakeRtsp(profile.thermal, _T("stream2"));

	// I3 System 열화상 카메라
	if (maker.CompareNoCase(_T("I3 System")) == 0)
		return MakeRtsp(profile.thermal, _T("cam0_0"));

	// 기타 열화상 기본값
	return MakeRtsp(profile.thermal, _T("rtsp_tunnel"));
}