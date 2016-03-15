#include "TCPClient.h"


TCPClient::TCPClient(void)
{
	int err;
	err = WSAStartup(MAKEWORD(2, 0), &wsaData);
	if (err != 0) {
		printf("tcp client");
		switch (err) {
			case WSASYSNOTREADY:
				printf("WSASYSNOTREADY\n");
				break;
			case WSAVERNOTSUPPORTED:
				printf("WSAVERNOTSUPPORTED\n");
				break;
			case WSAEINPROGRESS:
				printf("WSAEINPROGRESS\n");
				break;
			case WSAEPROCLIM:
				printf("WSAEPROCLIM\n");
				break;
			case WSAEFAULT:
				printf("WSAEFAULT\n");
				break;
		}
	}
	sock = INVALID_SOCKET;
}

TCPClient::~TCPClient(void)
{
	if (sock != INVALID_SOCKET){
		closesocket(sock);
	}
	WSACleanup();
}

int TCPClient::open(char *addr, int port){
	int err;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock == INVALID_SOCKET){
		std::cout << "create socket failed" << std::endl;
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.S_un.S_addr = inet_addr(addr);

	err = connect(sock, (const sockaddr *)&server, sizeof(server));
	if (err == SOCKET_ERROR){
		std::cout << "connect failed" << std::endl;
		return -1;
	}
	return 1;
}

int TCPClient::receive(char *dest, int size){
	int len;
	
	if (sock == INVALID_SOCKET){
		std::cout << "socket unavailable" << std::endl;
		return -1;
	}

	len = recv(sock,dest, size, 0);

	if(len == SOCKET_ERROR){
		std::cout << "receive failed" << std::endl;
		return -1;
	}

	return len;
	
}

int TCPClient::send(const char *buf, int size){
	int len;

	if (sock == INVALID_SOCKET){
		std::cout << "socket unavailable" << std::endl;
		return -1;
	}

	std::cout << "length:" << size << std::endl;
	

	len = ::send(sock, buf, size, 0);
	if (len == SOCKET_ERROR){
		std::cout << "send failed(tcp)" << std::endl;
		std::cout << "err no:" << WSAGetLastError() << std::endl;
		return -1;
	}
	
	return len;

}