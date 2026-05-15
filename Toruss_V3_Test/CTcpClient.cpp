
// CTcpClient.cpp: 구현 파일
//

#include "pch.h"
#include "CTcpClient.h"


// 생성자
CTcpClient::CTcpClient()
{
	m_socket = INVALID_SOCKET;
	m_isConnected = false;
}


// 소멸자
CTcpClient::~CTcpClient()
{
	Disconnect(); // 객체 소멸 시 TCP 연결 종료
}


/* [TCP CONNECT] 구조 흐름
[TCP CONNECT BUTTON CLICK]
↓
IP / Port 읽기
↓
CTcpClient::Connect()
↓
WSAStartup()
↓
socket 생성
↓
connect 시도
↓
성공 / 실패 Console 출력 */

// [TCP 서버] 연결 시작
bool CTcpClient::Connect(const std::string& ip, int port)
{
	// 이미 연결 중이면 중복 연결 방지
	if (m_isConnected)
	{
		std::cout << "[TCP] Already Connected." << std::endl;
		return true;
	}

	WSADATA wsaData; // Winsock 초기화용 구조체

	// Winsock 2.2 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "[TCP] WSAStartup Failed." << std::endl;
		return false;
	}

	// TCP Socket 생성
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Socket 생성 실패 확인
	if (m_socket == INVALID_SOCKET)
	{
		std::cout << "[TCP] Socket Create Failed." << std::endl;

		WSACleanup(); // Winsock 사용 종료
		return false;
	}

	// 서버 주소 구조체
	sockaddr_in serverAddr;

	// 구조체 초기화
	memset(&serverAddr, 0, sizeof(serverAddr));

	// IPv4 사용
	serverAddr.sin_family = AF_INET;

	// Port 설정
	serverAddr.sin_port = htons(port);

	// 문자열 IP 주소를 네트워크 주소로 변환
	if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0)
	{
		std::cout << "[TCP] Invalid IP Address." << std::endl;

		closesocket(m_socket);
		m_socket = INVALID_SOCKET;

		WSACleanup(); // Winsock 사용 종료
		return false;
	}

	std::cout << "[TCP] Connect Try..." << std::endl;

	// TCP 서버 연결 시도
	if (connect(
		m_socket,
		(sockaddr*)&serverAddr,
		sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cout << "[TCP] Connect Failed." << std::endl;

		closesocket(m_socket);
		m_socket = INVALID_SOCKET;

		WSACleanup(); // Winsock 사용 종료
		return false;
	}

	// TCP 연결 상태 ON
	m_isConnected = true;

	std::cout << "[TCP] Connect Success." << std::endl;

	if (m_recvThread.joinable())
	{
		std::cout << "[TCP] Previous Recv Thread Still Joinable." << std::endl;

		std::cout << " " << std::endl;
		return false;
	}

	// TCP 수신 스레드 시작
	m_recvThread = std::thread(&CTcpClient::RecvLoop, this);

	return true;
}


/* [TCP DISCONNECT] 구조 흐름
Disconnect 버튼 클릭
↓
closesocket()

(RecvLoop Thread)
recv() 대기 중
↓
소켓 닫힘 감지
↓
recv() 종료
↓
while 탈출
↓
RecvLoop 끝
↓
thread 종료

(Main Thread)
↓
join()
↓
"스레드 종료 완료 확인" */

// [TCP] 서버 연결 종료 함수
void CTcpClient::Disconnect()
{
	// 이미 연결 종료 상태면 함수 종료
	if (!m_isConnected)
	{
		std::cout << "[TCP] Already Disconnected." << std::endl;

		std::cout << " " << std::endl;

		return;
	}

	// TCP 연결 상태 OFF
	m_isConnected = false;

	// 유효한 Socket이면 종료 처리
	if (m_socket != INVALID_SOCKET)
	{
		// Socket 종료
		closesocket(m_socket);

		// Socket 값 초기화
		m_socket = INVALID_SOCKET;
	}

	// 수신 스레드가 실행 중이면 종료 대기
	if (m_recvThread.joinable())
	{
		std::cout << "[TCP] Recv Thread Join..." << std::endl;

		m_recvThread.join();

		std::cout << "[TCP] Recv Thread Join Complete." << std::endl;
	}

	WSACleanup(); // Winsock 사용 종료

	std::cout << "[TCP] Disconnect Complete." << std::endl; // Console Log 출력
}


// [TCP] 데이터 송신 함수
bool CTcpClient::Send(const std::vector<unsigned char>& data)
{
	// TCP 연결 상태가 아니면, 송신 불가
	if (!m_isConnected || m_socket == INVALID_SOCKET)
	{
		std::cout << "[TCP] Send Failed. Not Connected." << std::endl;
		return false;
	}

	// 송신할 데이터가 비어있으면, 함수 종료
	if (data.empty())
	{
		std::cout << "[TCP] Send Failed. Empty Data." << std::endl;
		return false;
	}

	// TCP 데이터 송신
	int sendSize = send(
		m_socket,
		reinterpret_cast<const char*>(data.data()),
		static_cast<int>(data.size()),
		0);

	// 송신 실패 확인
	if (sendSize == SOCKET_ERROR)
	{
		std::cout << "[TCP] Send Failed." << std::endl;
		return false;
	}

	std::cout << "[TCP SEND]: "; // 송신 성공 로그 출력

	// 송신할 데이터(data)에서 1바이트씩 순차적으로...
	for (unsigned char byte : data)
	{
		// 현재 바이트 값을 16진수(HEX) 2자리 형식으로 출력
		// 예: 0x01 -> 01
		//     0x0A -> 0A
		//     0xFF -> FF
		printf("%02X ", byte);
	}
	std::cout << " " << std::endl;

	return true;
}


// [TCP] 수신 스레드 함수
void CTcpClient::RecvLoop()
{
	// 수신 버퍼
	char buffer[1024] = { 0 };

	// TCP 연결 상태일 때 반복 수신
	while (m_isConnected)
	{
		// 버퍼 초기화
		memset(buffer, 0, sizeof(buffer));

		// TCP 데이터 수신
		int recvSize = recv(
			m_socket,
			buffer,
			sizeof(buffer),
			0);

		// TCP 연결 종료 또는 수신 실패 확인
		//   0 = 상대 연결 종료
		// - 1 = 에러
		if (recvSize <= 0)
		{
			// 상대방 정상 연결 종료
			if (recvSize == 0)
			{
				std::cout << "[TCP] Server Disconnected." << std::endl;

				std::cout << " " << std::endl;
			}
			else
			{
				std::cout << "[TCP] Receive Failed." << std::endl;

				std::cout << " " << std::endl;
			}
			break;
		}

		std::cout << "[TCP RECV] "; // 수신 데이터 [HEX]로 출력

		for (int i = 0; i < recvSize; i++)
		{
			printf("%02X ", static_cast<unsigned char>(buffer[i])); // 수신 데이터 [1Byte]씩 [HEX] 출력
		}
		printf("\n"); // 출력 줄 -> 정리
	}
	m_isConnected = false; // TCP 연결 상태 OFF
}
