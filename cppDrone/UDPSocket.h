#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

class UDPSocket
{
public:
	UDPSocket(void);
	~UDPSocket(void);
	int open(char *addr, int port);
	int send(const char *buf, int size);
	int receive(char *buf, int size);

private:
	WSADATA wsaData;
	SOCKET sock;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;


};

