#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>


class TCPServer
{
public:
	TCPServer(void);
	~TCPServer(void);
	int createServer(int port);
	int listen();
	int send(char *buf, int size);
	

private:
	WSADATA wsaData;
	SOCKET mySock;
	SOCKET clientSock;
	struct sockaddr_in myAddr;
	struct sockaddr_in clientAddr;
};

