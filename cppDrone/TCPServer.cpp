#include "TCPServer.h"


TCPServer::TCPServer(void)
{
	int err;
	err = WSAStartup(MAKEWORD(2, 0), &wsaData);
	if (err != 0) {
		printf("tcp server");
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

	mySock = INVALID_SOCKET;
	clientSock = INVALID_SOCKET;
}


TCPServer::~TCPServer(void)
{
	if (mySock != INVALID_SOCKET){
		closesocket(mySock);

	}
	if(clientSock != INVALID_SOCKET){
		closesocket(clientSock);
	}

	WSACleanup();
}

int TCPServer::createServer(int port){
	int err;

	mySock =socket(AF_INET, SOCK_STREAM, 0);

	if (mySock == INVALID_SOCKET){
		std::cout << "create socket failed" << std::endl;
		return -1;
	}

	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(port);
	myAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	err = bind(mySock, (struct sockaddr *)&myAddr, sizeof(myAddr));
	if (err == SOCKET_ERROR){
		std::cout << "bind failed" << std::endl;
		return -1;
	}
	return 1;
}

int TCPServer::listen(){
	int err;
	int len;

	if (mySock == INVALID_SOCKET){
		std::cout << "socket unavailable" << std::endl;
		return -1;
	}
	
	err = ::listen(mySock, 1);
	if (err == SOCKET_ERROR){
		std::cout << "listen failed" << std::endl;
		return -1;
	}

	len = sizeof(clientAddr);
	clientSock = accept(mySock, (struct sockaddr *)&clientAddr, &len);
	if (clientSock == INVALID_SOCKET){
		std::cout << "accept failed" << std::endl;
		return -1;
	}

	return 1;
}

int TCPServer::send(char *buf, int size){
	int len;

	if (clientSock == INVALID_SOCKET){
		std::cout << "socket unavailable" << std::endl;
		return -1;
	}

	len = ::send(clientSock, buf, size, 0);
	if (len == SOCKET_ERROR){
		std::cout << "send failed(tcp server)" << std::endl;
		std::cout << "err no:" << WSAGetLastError() << std::endl;
		return -1;
	}
	
	return len;
}

