#include "bebopCommand.h"


bebopCommand::bebopCommand(string ip, int c2dPort, int d2cPort)
{
	this->c2dPort = c2dPort;
	this->d2cPort = d2cPort;
	this->ip = ip;
	int err =  WSAStartup(MAKEWORD(2, 0), &wsaData);
	if (err){
		printf("test");
	}

	if (!handshake()){
		//err
	}

	this->c2dSocket = createSocket(this->c2dPort);
	this->d2cSocket = createSocket(this->d2cPort);


	//th = thread(receiveingData);
	thread th(&bebopCommand::receiveingData, this);

	
	
}


bebopCommand::~bebopCommand(void)
{
	WSACleanup();
	closesocket(this->c2dSocket.sock);
	closesocket(this->d2cSocket.sock);
}

int bebopCommand::handshake(){
	SOCKET tcpSock;
	struct sockaddr_in server;
	char buf[1024];
	int result = 1;

	//バッファの初期化
	memset(buf, 0, sizeof(buf));

	//create tcp socket
	tcpSock = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_port = htons(44444);
	server.sin_addr.S_un.S_addr = inet_addr("192.168.42.1");

	//connect
	int e = connect(tcpSock, (struct sockaddr *)&server, sizeof(server));
	if (e == SOCKET_ERROR){
		printf("err");

	}
	string message = "{\"controller_type\":\"computer\", \"controller_name\":\"katarina\", \"d2c_port\":\"43210\"}";

	//send message
	if (send(tcpSock, message.c_str(), message.length(), 0) == SOCKET_ERROR){
		cout << "send err" << endl;
	}

	//recv message	
	if (recv(tcpSock, buf, sizeof(buf), 0) == SOCKET_ERROR){		
		result = 0;
		cout << "recv err" << endl;
	 } else {
		printf("message:::%s\n", buf);
	}
	closesocket(tcpSock);
	
	return result;
}

udpSocket bebopCommand::createSocket(int port){
	udpSocket sock;

	sock.sock = socket(AF_INET, SOCK_DGRAM, 0);

	sock.addr.sin_family = AF_INET;
	sock.addr.sin_port = htons(port);
	sock.addr.sin_addr.S_un.S_addr = inet_addr(this->ip.c_str());

	return sock;

}

void bebopCommand::receiveingData(){
	bind(this->d2cSocket.sock, (struct sockaddr *)&this->d2cSocket.addr, sizeof(this->d2cSocket.addr));
	//generateAllStates();
	//start receiveing data 
	while (true)
	{
		char recvBytes[1024];
		recv(this->c2dSocket.sock, recvBytes, sizeof(recvBytes), 0);
		printf("size::::%d", strlen(recvBytes));
		_networkFrame = networkFrameParser(recvBytes);
		printf("type:%d id:%d seq:%d size:%d\n", _networkFrame.type, _networkFrame.id, _networkFrame.seq, _networkFrame.size);

	}

}

networkFrame bebopCommand::networkFrameParser(char *recvBytes){
	networkFrame frame;
	
	//memcpy(&frame, recvBytes, 7);
	//vector<UINT8> temp(frame.size - 7);
	//frame.data.reserve(frame.size - 7);

	//frame.data = new UINT8[frame.size - 7];
	//frame.data[0] = 1;
	//printf("%d %d", sizeof(frame.data), frame.size-7);

	///////test////////
	frame.type = recvBytes[0];
	frame.id = recvBytes[1];
	frame.seq = recvBytes[2];
	memcpy(&frame.size, &recvBytes[3], sizeof(UINT32));
	
	printf("type:%d id:%d seq:%d size:%d\n", frame.type, frame.id, frame.seq, frame.size);
	////////end test//////////

	//memcpy(&frame.data[0], &recvBytes[7], sizeof(frame.data));

	return frame;	
}

void bebopCommand::generateAllStates(){
	char buf[4];
	UINT16 temp = ARCOMMANDS_ID_COMMON_COMMON_CMD_ALLSTATES;

	buf[0] = ARCOMMANDS_ID_PROJECT_COMMON;
	buf[1] = ARCOMMANDS_ID_COMMON_CLASS_COMMON;
	buf[2] = temp & 0xff;
	buf[3] = (temp >> 8);
	int a = strlen(buf);
	writePacket(buf);
}

void bebopCommand::writePacket(char *packet){
	char *t = "aaaa";
	int a = strlen(t);
	sendto(this->c2dSocket.sock, packet, 4, 0, (struct sockaddr *)&this->c2dSocket.addr, sizeof(this->c2dSocket.addr));

}