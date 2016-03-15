#include "bebopCommand.h"


bebopCommand::bebopCommand(string ip, int c2dPort, int d2cPort, asio::io_service& io_service)
	: d2cSocket(io_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), d2cPort)),
	c2dSocket(io_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), c2dPort)),
	droneIp(ip)
{

	if (!handshake()){
		std::cout << "handshake error" << std::endl;
	}
	
	ofs.open("video.bin", std::ios_base::binary);

	

	_arStreamFrame.frameSize = 0;
	_arStreamFrame.frame = new UINT8[0];
	
	_arStreamFrame.frameACK = new UINT8[16];
	//ゼロクリア
	for (int i = 0; i < 16; i++) _arStreamFrame.frameACK[i] = 0;

	//decoder = createFfmpegDecoder();
	//img = cvCreateImage(cvSize(decoder->codecCtx->width, decoder->codecCtx->height), IPL_DEPTH_8U, 3);
	initPCMD();
	startReceive();
	io_service.post([this](){
		sendPCMD();
	});
	//setSendPCMDTask();
	for (int i = 0; i < 256; i++){
		seq.push_back(0);
	}
	generateAllStates();
	//takeOff();
	
	
}


bebopCommand::~bebopCommand(void)
{
	
}

int bebopCommand::handshake(){
	//SOCKET tcpSock;
	//struct sockaddr_in server;
	string message = "{\"controller_type\":\"computer\", \"controller_name\":\"katarina\", \"d2c_port\":\"43210\"}";
	//asioでtcpソケット作成
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::socket sock(io_service);

	//connect
	sock.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(this->droneIp), DISCOVERY_PORT));
	boost::asio::write(sock, boost::asio::buffer(message));

	boost::asio::streambuf receiveBuffer;
	boost::system::error_code error;

	
	//receive
	boost::asio::read(sock, receiveBuffer, boost::asio::transfer_at_least(1), error);

	if (error && error != boost::asio::error::eof){
		cout << "failed " << error.message() << endl;
		return 0;
	}
	cout << "receive message::" << &receiveBuffer <<endl;

	return 1;
}

void bebopCommand::startReceive(){
	this->d2cSocket.async_receive_from(asio::buffer(this->recvBuffer), this->remoteEndpoint, boost::bind(&bebopCommand::reciveData, this, asio::placeholders::error, _2));

}

void bebopCommand::sendPCMD(){
	while (true){
		Sleep(25);
		//std::cout << "send pcmd" << std::endl;
		writePacket(generatePCMD());
		initPCMD();
	}
}

void bebopCommand::initPCMD(){
	_pcmd.flag = 1;
    _pcmd.roll = 0;
    _pcmd.pitch = 0;
    _pcmd.yaw = 0;
	_pcmd.gaz = 0;
	_pcmd.psi = 0.0F;
}

bebopCommand::command bebopCommand::generatePCMD(){
	command buf;
	UINT8 *temp;
	float val = _pcmd.psi;
	temp = (UINT8 *)&val;

	buf.cmd = new UINT8[13];
	buf.size = 13;
	buf.typeSize = sizeof(UINT8);

	buf.cmd[0] = ARCOMMANDS_ID_PROJECT_ARDRONE3;
	buf.cmd[1] = ARCOMMANDS_ID_ARDRONE3_CLASS_PILOTING;
	buf.cmd[2] = (UINT8)(ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_PCMD & 0xff);
	buf.cmd[3] = (UINT8)((ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_PCMD & 0xff00) >> 8);
	buf.cmd[4] = _pcmd.flag;
	buf.cmd[5] = _pcmd.roll;
	buf.cmd[6] = _pcmd.pitch;
	buf.cmd[7] = _pcmd.yaw;
	buf.cmd[8] = _pcmd.gaz;
	
	buf.cmd[9] = temp[0];
	buf.cmd[10] = temp[1];
	buf.cmd[11] = temp[2];
	buf.cmd[12] = temp[3];



	return networkFrameGenerator(buf);
	
}

//データ受信イベントのコールバック
void bebopCommand::reciveData(const boost::system::error_code& error, std::size_t len){
	if(error){
		std::cout << "err" << std::endl;
		return;
	}
	//std::cout << "recived data" << std::endl;
	netFrame networkFrame;

	networkFrame = netFrameParser(this->recvBuffer.data());
	
	if (networkFrame.type == ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK){
		
		writePacket(createAck(networkFrame));
	}

	if (networkFrame.type == ARNETWORKAL_FRAME_TYPE_DATA_LOW_LATENCY && networkFrame.id == BD_NET_DC_VIDEO_DATA_ID){
		//std::cout << "video" << std::endl; //debug
		//std::cout << "seq" << (int)networkFrame.seq << std::endl;
		streamFrame streamFram = parseStreamFrame(networkFrame);
		writePacket(createARStreamACK(streamFram));
	}
	/////イベント///////
	if (networkFrame.id == BD_NET_DC_EVENT_ID){
		std::cout << "event" << std::endl;
	}
	////////////////////

	if (networkFrame.id == ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PING){
		std::cout << "ping" << std::endl; //debug
		
		writePacket(createPong(networkFrame));
		//takeOff();
	}
	delete[] networkFrame.data;
	startReceive();
}

//受信フレームのパーサー
bebopCommand::netFrame bebopCommand::netFrameParser(char* frame){
	netFrame networkFrame;
	
	networkFrame.type = (UINT8)frame[0];
	networkFrame.id = (UINT8)frame[1];
	networkFrame.seq = (UINT8)frame[2];
	networkFrame.size = toUInt32(frame, 3);
	
	if (networkFrame.size > 7){
		networkFrame.data = new UINT8[networkFrame.size - 7];		
		memcpy(networkFrame.data, &frame[7], networkFrame.size - 7);
	}
	return networkFrame;
}



bebopCommand::command bebopCommand::createAck(bebopCommand::netFrame networkFrame){
	command buf;
	//UINT8 buf[1];
	buf.cmd = new UINT8[1];
	buf.size = 1;
	buf.typeSize = sizeof(UINT8);
	UINT8 id;	
	UINT8 t = ARNETWORKAL_MANAGER_DEFAULT_ID_MAX / 2 + networkFrame.id;
	
	buf.cmd[0] = networkFrame.seq;
	id = (networkFrame.id + (int)(ARNETWORKAL_MANAGER_DEFAULT_ID_MAX / 2));
	
	return networkFrameGenerator(buf, ARNETWORKAL_FRAME_TYPE_ACK, id);	
}

bebopCommand::command bebopCommand::networkFrameGenerator(bebopCommand::command cmd, int type, int id){
	
	UINT8 temp[4] = {0, 0, 0, 0};
	command marge;
	int packetSize = cmd.size + 7;

	marge.cmd = new UINT8[packetSize];
	marge.size = packetSize;
	marge.typeSize = sizeof(UINT8);
	
	seq[id]++;
	if (seq[id] > 255) seq[id] = 0;

	marge.cmd[0] = (UINT8) type;
	marge.cmd[1] = (UINT8)id;
	marge.cmd[2] = seq[id];
	marge.cmd[3] = (UINT8)(packetSize & 0xff);
	marge.cmd[4] = (UINT8)((packetSize & 0xff00) >> 8);
	marge.cmd[5] = (UINT8)((packetSize & 0xff0000) >> 16);
	marge.cmd[6] = (UINT8)((packetSize & 0xff000000) >> 24);
	int index = 7;
	for (int i = 0; i < cmd.size; i++){
		marge.cmd[index] = cmd.cmd[i];
		index++;
	}
	
	//使用済みのメモリを開放
	delete[] cmd.cmd;
	
	return marge;
}

void bebopCommand::generateAllStates(){
	//UINT8 buf[4];
	UINT8 temp[4] = {0, 0, 0, 0};
	command buf;
	buf.cmd = new UINT8[4];
	buf.size = 4;
	buf.typeSize = sizeof(UINT8);

	std::cout << "generateAllStates" << std::endl;

	temp[0] = ARCOMMANDS_ID_COMMON_COMMON_CMD_ALLSTATES;

	buf.cmd[0] = ARCOMMANDS_ID_PROJECT_COMMON;
	buf.cmd[1] = ARCOMMANDS_ID_COMMON_CLASS_COMMON;
	arrayCopy(temp, 0, buf.cmd, 2, 2);


	//writePacket(networkFrameGenerator(buf), "127.0.0.1");
	writePacket(networkFrameGenerator(buf));
}

bebopCommand::command bebopCommand::createPong(netFrame networkFrame){
	int size = networkFrame.size - 7;
	//UINT8 *temp = new UINT8(size);
	command buf;
	buf.cmd = new UINT8[size];
	buf.size = size;
	buf.typeSize = sizeof(UINT8);
	
	for (int i = 0; i < size; i++) buf.cmd[i] = networkFrame.data[i];

	return networkFrameGenerator(buf, ARNETWORKAL_FRAME_TYPE_DATA, ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PONG);
}

void bebopCommand::videoStreming(){
	//UINT8 buf[4] = {ARCOMMANDS_ID_PROJECT_ARDRONE3, ARCOMMANDS_ID_ARDRONE3_CLASS_CAMERA, ARCOMMANDS_ID_ARDRONE3_CAMERA_CMD_ORIENTATION, true};
	command buf;
	buf.cmd = new UINT8[4];
	buf.size = 4;
	buf.typeSize = sizeof(UINT8);

	buf.cmd[0] = ARCOMMANDS_ID_PROJECT_ARDRONE3;
	buf.cmd[1] = ARCOMMANDS_ID_ARDRONE3_CLASS_CAMERA;
	buf.cmd[2] = ARCOMMANDS_ID_ARDRONE3_CAMERA_CMD_ORIENTATION;
	buf.cmd[3] = true;
	std::cout << "video streaming" << std::endl; //debug
	writePacket(buf);
}

void bebopCommand::takeOff(){
	//UINT8 buf[4] = {ARCOMMANDS_ID_PROJECT_ARDRONE3, ARCOMMANDS_ID_ARDRONE3_CLASS_PILOTING, ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_TAKEOFF};
	command buf;
	buf.cmd = new UINT8[4];
	buf.size = 4;
	buf.typeSize = sizeof(UINT8);

	buf.cmd[0] = ARCOMMANDS_ID_PROJECT_ARDRONE3;
	buf.cmd[1] = ARCOMMANDS_ID_ARDRONE3_CLASS_PILOTING;
	buf.cmd[2] = ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_TAKEOFF;
	buf.cmd[3] = 0;
	writePacket(networkFrameGenerator(buf));
	std::cout << "takeoff" << std::endl; //debug
}
void bebopCommand::writePacket(command packet, std::string ip){
	this->c2dSocket.send_to(asio::buffer(packet.cmd, packet.typeSize * packet.size), asio::ip::udp::endpoint(asio::ip::address::from_string(ip), 54321));
	delete[] packet.cmd;
}


bebopCommand::streamFrame bebopCommand::parseStreamFrame(netFrame data){
	streamFrame frame;

	frame.frameNumber = toUInt16(data.data, 0);
	frame.frameFlags = data.data[2];
	frame.fragmentNumber = data.data[3];
	frame.fragmentPerFrame = data.data[4];
	frame.frame = new UINT8[data.size - 5];
	frame.frameSize = data.size - 5;

	memcpy(frame.frame, &data.data[5], data.size - 5);

	return frame;
}

bebopCommand::command bebopCommand::createARStreamACK(streamFrame streamFrame){	
	//std::cout << "frameNumber::" << streamFrame.frameNumber << std::endl;
	//std::cout << "fragmentNumber::" << (int)streamFrame.fragmentNumber << std::endl;
	//std::cout << "fragmentPerFrame::" << (int)streamFrame.fragmentPerFrame << std::endl;
	if (streamFrame.frameNumber != _arStreamFrame.frameNumber){
		
		if (_arStreamFrame.frameSize > 0){
			//びでお
			
			//std::ofstream ofs("video.bin", std::ios::binary);
			std::cout << "frame::" << _arStreamFrame.frameNumber << std::endl;
			std::cout << "size::" << _arStreamFrame.frameSize << std::endl;


			//std::cout << "fragmentNumber::" << (int)streamFrame.fragmentNumber << std::endl;
			//std::cout << "fragmentPerFrame::" << (int)streamFrame.fragmentPerFrame << std::endl;
			//int *temp = (int *)_arStreamFrame.frame.data();
			
			//ofs.write((char *)temp, _arStreamFrame.frame.size() * sizeof(int));
			ofs.write((char *)_arStreamFrame.frame, _arStreamFrame.frameSize);
			
			/*
			
			int gotFrame = 0;

			decoder->avpkt.data = _arStreamFrame.frame;
			decoder->avpkt.size = _arStreamFrame.frameSize;
			
			while (decoder->avpkt.size > 0){
				int len = avcodec_decode_video2(decoder->codecCtx, decoder->decodedFrame, &gotFrame, &(decoder->avpkt));
				//getchar();
				if(gotFrame){
					std::cout << "gotFrame" << std::endl;
					sws_scale(decoder->convertCtx, (const uchar* const*)decoder->decodedFrame->data, decoder->decodedFrame->linesize, 0, decoder->codecCtx->height, decoder->decodedFrameRBG->data, decoder->decodedFrameRBG->linesize);
					memcpy(img->imageData, decoder->decodedFrameRBG->data[0], decoder->codecCtx->width * decoder->codecCtx->height * sizeof(uchar) * 3);

				}

				decoder->avpkt.size -= len;
				decoder->avpkt.data += len;
			}
			cvShowImage("img", img);
			*/
		}
		//データの初期化
		delete[] _arStreamFrame.frameACK;
		_arStreamFrame.frameNumber = streamFrame.frameNumber;
		_arStreamFrame.frameSize = 0;
		_arStreamFrame.frameACK = new UINT8[16];
		//ゼロクリア
		for (int i = 0; i < 16; i++) _arStreamFrame.frameACK[i] = 0;

	}
	int tempSize = _arStreamFrame.frameSize;
	UINT8 *temp = new UINT8[tempSize];
	
	memcpy(temp, _arStreamFrame.frame, _arStreamFrame.frameSize);

	//フレームサイズのこうしん
	_arStreamFrame.frameSize += streamFrame.frameSize;

	//メモリ領域の再確保
	delete[] _arStreamFrame.frame;
	_arStreamFrame.frame = new UINT8[_arStreamFrame.frameSize];

	//ゼロクリア
	for (int i = 0; i < _arStreamFrame.frameSize; i++) _arStreamFrame.frame[i] = 0;

	//データの結合	
	memcpy(_arStreamFrame.frame, temp, tempSize);
	memcpy(&_arStreamFrame.frame[tempSize], streamFrame.frame, streamFrame.frameSize);

	//使用済みの領域を開放
	delete[] temp;
	delete[] streamFrame.frame;
	
	//_arStreamFrame.frame.insert(_arStreamFrame.frame.end(), streamFrame.frame.begin(), streamFrame.frame.end());
	
	int bufferPosition = (int)floor(streamFrame.fragmentNumber / 8);
	_arStreamFrame.frameACK[bufferPosition] |= (UINT8)(1 << streamFrame.fragmentNumber % 8);
	
	ackPacket ack;
	ack.frameNumber = _arStreamFrame.frameNumber;
	ack.packetsACK = new UINT8[16];

	memcpy(ack.packetsACK, &_arStreamFrame.frameACK[8], 8);
	memcpy(&ack.packetsACK[8], _arStreamFrame.frameACK, 8);
	
	command ret;
	ret.cmd = new UINT8[18];
	ret.size = 18;
	ret.typeSize = sizeof(UINT8);

	//ゼロクリア
	for (int i = 0; i < 18; i++) ret.cmd[i] = 0;

	ret.cmd[0] = (UINT8)(ack.frameNumber & 0xff);
	ret.cmd[1] = (UINT8)((ack.frameNumber & 0xff00) >> 8);
	memcpy(&ret.cmd[2], ack.packetsACK, 16);

	return networkFrameGenerator(ret,  ARNETWORKAL_FRAME_TYPE_DATA, BD_NET_CD_VIDEO_ACK_ID);

}

void bebopCommand::initVideoDatas(bebopCommand::videoDatas *datas){
	datas->freeRawFramePool = (rawFrame **)malloc(sizeof(rawFrame*) * BD_RAW_FRAME_POOL_SIZE);
	datas->rawFramePoolCapacity = BD_RAW_FRAME_POOL_SIZE;

	for (int i= 0; i < datas->rawFramePoolCapacity; i++){
		rawFrame *frame = (rawFrame *)malloc(sizeof(rawFrame));
		frame->size = BD_NET_DC_VIDEO_FRAG_SIZE * BD_NET_DC_VIDEO_MAX_NUMBER_OF_FRAG;
		frame->data = (UINT8 *)malloc(frame->size);

		datas->freeRawFramePool[i] = frame;
	}

	datas->lastRawFrameFreeIdx = 0;

	datas->rawFrameFifo = (rawFrame **)calloc(BD_RAW_FRAME_BUFFER_SIZE, sizeof(rawFrame*));

}

bebopCommand::ffmpegDecoder *bebopCommand::createFfmpegDecoder(){
	ffmpegDecoder *decoder = NULL;
	decoder = (ffmpegDecoder *)calloc(1, sizeof(ffmpegDecoder));
	if(decoder == NULL){
		//return;
	}

	avcodec_register_all();

	//av_log_set_level(AV_LOG_QUIET);

	decoder->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if(decoder->codec == NULL){
		//return;
	}


	decoder->codecCtx = avcodec_alloc_context3(decoder->codec);
	if (decoder->codecCtx == NULL){
		//return;
	}

	decoder->codecCtx->pix_fmt = PIX_FMT_YUV420P;
    decoder->codecCtx->skip_frame = AVDISCARD_DEFAULT;
    decoder->codecCtx->error_concealment = FF_EC_GUESS_MVS | FF_EC_DEBLOCK;
    decoder->codecCtx->skip_loop_filter = AVDISCARD_DEFAULT;
    decoder->codecCtx->workaround_bugs = FF_BUG_AUTODETECT;
    decoder->codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    decoder->codecCtx->codec_id = AV_CODEC_ID_H264;
    decoder->codecCtx->skip_idct = AVDISCARD_DEFAULT;

	if(avcodec_open2(decoder->codecCtx, decoder->codec, NULL) < 0){
		//return;
	}

	decoder->decodedFrame = avcodec_alloc_frame();
	decoder->decodedFrameRBG = avcodec_alloc_frame();

	decoder->buffer = (uchar*)av_malloc(avpicture_get_size(PIX_FMT_BGR24, decoder->codecCtx->width, decoder->codecCtx->height));

	avpicture_fill((AVPicture*)decoder->decodedFrameRBG, decoder->buffer, PIX_FMT_BGR24, decoder->codecCtx->width, decoder->codecCtx->height);

	decoder->convertCtx = sws_getContext(decoder->codecCtx->width, decoder->codecCtx->height, decoder->codecCtx->pix_fmt, decoder->codecCtx->width, decoder->codecCtx->height, PIX_FMT_BGR24, SWS_SPLINE, NULL, NULL, NULL);

	av_init_packet(&decoder->avpkt);

	return decoder;



}
void bebopCommand::arrayCopy(UINT8 *source, int sourceIndex, UINT8 *dest, int destIndex, int len){
	

	for (int i = 0; i < len; i++){
		dest[destIndex + i] = source[sourceIndex + i];
	}

}

UINT32 bebopCommand::toUInt32(char* byte, int index){
	return (UINT32)((UINT8)(byte[index] << 0) | ((UINT8)byte[index + 1] << 8) | ((UINT8)byte[index + 2] << 16) | ((UINT8)byte[index + 3] << 24));
}

UINT16 bebopCommand::toUInt16(UINT8 *byte, int index){
	return (UINT16)((byte[index] << 0) | (byte[index + 1] << 8));
}