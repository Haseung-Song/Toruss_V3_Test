#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

// TCP Client 통신 처리 클래스

class CTcpClient
{
public:
	CTcpClient(); // 생성자
	~CTcpClient(); // 소멸자

	// TCP 서버 연결
	bool Connect(const std::string& ip, int port);

	// TCP 서버 연결 종료
	void Disconnect();

	// TCP 데이터 송신
	bool Send(const std::vector<unsigned char>& data);

private:
	// TCP 수신 스레드 함수
	void RecvLoop();

private:
	SOCKET m_socket; // TCP Socket 핸들

	std::thread m_recvThread; // 수신 스레드

	std::atomic_bool m_isConnected; // TCP 연결 상태
};