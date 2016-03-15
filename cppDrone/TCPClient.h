#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

class TCPClient
{
public:
	TCPClient(void);
	~TCPClient(void);
	int open(char *addr, int port);
	int receive(char *dest, int size);
	int send(const char *buf, int size);

private:
	WSADATA wsaData;
	SOCKET sock;
	struct sockaddr_in server;
};

