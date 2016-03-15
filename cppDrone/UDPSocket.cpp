#include "UDPSocket.h"


UDPSocket::UDPSocket(void)
{
	int err;
	err = WSAStartup(MAKEWORD(2, 0), &wsaData);
	if (err != 0) {
		printf("udp socket");
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


UDPSocket::~UDPSocket(void)
{
	if (sock != INVALID_SOCKET){
		closesocket(sock);
	}

	WSACleanup();
}

//open client socket
int UDPSocket::open(char *addr, int port){
	int err;

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock == INVALID_SOCKET){
		std::cout << "create socket failed" << std::endl;
		return -1;
	}

	//setting client port
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(port);
	client_addr.sin_addr.S_un.S_addr = inet_addr(addr);

	//setting server port
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	err = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err == SOCKET_ERROR){
		std::cout << "bind failed" << std::endl;
		return -1;
	}

	return 1;
}

int UDPSocket::send(const char *buf, int size){
	int len;
	if (sock == INVALID_SOCKET){
		std::cout << "socket unavailable" << std::endl;
		return -1;
	}

	len = sendto(sock, buf, size, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
	if (len == SOCKET_ERROR){
		std::cout << "send failed(udp)" << std::endl;
		return -1;
	}
	return len;
}

int UDPSocket::receive(char *buf, int size){
	int len;
	int addrLen;

	if (sock == INVALID_SOCKET){
		std::cout << "socket unavailable" << std::endl;
		return -1;
	}
	
	addrLen = sizeof(server_addr);

	len = recvfrom(sock, buf, size, 0, (struct sockaddr *)&server_addr, &addrLen);

	if (len == SOCKET_ERROR){
		std::cout << "receive failed" << std::endl;
		return -1;
	}

	return len;

}