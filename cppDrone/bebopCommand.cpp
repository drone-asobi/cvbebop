#include "bebopCommand.h"
#include <chrono>
#include <Windows.h>

#pragma comment (lib, "winmm.lib")

bebopCommand::bebopCommand()
{
	if (handshake() == -1){
		std::cout << "handshake error" << std::endl;
	}
	
	//open soket
	d2cSocket.open(IP, D2C_PORT);
	c2dSocket.open(IP, C2D_PORT);
	vodeoStream.createServer(VIDEO_PORT);
	
	
	//ofs.open("video.bin", std::ios_base::binary);	

	newImageFlag = false;
	memset(arStreamFrame.frameACK, 0, 16);
	
	seq = new int[256];
	memset(seq, 0, 256);
	
	generateAllStates();
	
	//スレッド開始
	threadGroup.push_back(std::thread([this]{recivingThread();}));
	//ビデオの初期化
	initVideo();
	threadGroup.push_back(std::thread([this]{PCMDThread();}));
	threadGroup.push_back(std::thread([this]{decodingThread();}));
	
}

bebopCommand::~bebopCommand(void)
{

	for(std::thread &th: threadGroup) th.join();	

	// FFmpeg終了
	/*
    sws_freeContext(pConvertCtx);
    av_free(buffer);
    av_free(pFrameBGR);
    av_free(pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
	*/
}

int bebopCommand::handshake(){
	
	TCPClient tcpClient;
	std::string message = "{\"controller_type\":\"computer\", \"controller_name\":\"katarina\", \"d2c_port\":\"43210\"}";
	
	char buf[1024];
	
	//socket open
	if(tcpClient.open(IP, DISCOVERY_PORT) == -1){
		return -1;
	}	

	//send message
	if (tcpClient.send(message.c_str(), message.length()) == -1){
		return -1;
	}

	//receive
	if (tcpClient.receive(buf, 1024) == -1){
		return -1;
	}
	
	std::string json(buf);
	picojson::value v;
	std::string jsonErr = picojson::parse(v, json);
	if (!jsonErr.empty()){
		std::cout << jsonErr << std::endl;
	}
	std::cout << buf << std::endl;
	
	picojson::object& o = v.get<picojson::object>();
	
	maxFragmentSize = 65000;
	//::cout << "size:" << o.size() << std::endl;
	//std::string s = o["c2d_port"].get<std::string>();
	

	//maxFragmentSize = std::stoi(o["arstream_fragment_size"].get<std::string>());
	//maxFragmentNumber = std::stoi(o["arstream_fragment_maximum_number"].get<std::string>());
		
	return 1;
}

void bebopCommand::initVideo(){
	//create ffmpeg decoder
	
	avcodec_register_all();

	av_log_set_level(AV_LOG_QUIET);

	ffmpegDecoder.codec = avcodec_find_decoder(AV_CODEC_ID_H264);

	if (ffmpegDecoder.codec == NULL){
		std::cout << "faild find decoder" << std::endl;
		getchar();
	}

	ffmpegDecoder.codecCtx = avcodec_alloc_context3(ffmpegDecoder.codec);
	if (ffmpegDecoder.codecCtx == NULL){
		std::cout << "faild allocate ffmpeg context" << std::endl;
		getchar();
	}
	
	ffmpegDecoder.codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	ffmpegDecoder.codecCtx->skip_frame = AVDISCARD_DEFAULT;
	ffmpegDecoder.codecCtx->error_concealment = FF_EC_GUESS_MVS | FF_EC_DEBLOCK;
	ffmpegDecoder.codecCtx->skip_loop_filter = AVDISCARD_DEFAULT;
	ffmpegDecoder.codecCtx->workaround_bugs = FF_BUG_AUTODETECT;
	ffmpegDecoder.codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	ffmpegDecoder.codecCtx->codec_id = AV_CODEC_ID_H264;
	ffmpegDecoder.codecCtx->skip_idct = AVDISCARD_DEFAULT;

	if (avcodec_open2(ffmpegDecoder.codecCtx, ffmpegDecoder.codec, NULL) < 0){
		std::cout << "faild open avcodec" << std::endl;
		exit(1);
	}
	
	//ffmpegDecoder.decodedFrame = avcodec_alloc_frame();
	ffmpegDecoder.decodedFrame = av_frame_alloc();
	if (ffmpegDecoder.decodedFrame == NULL){
		std::cout << "faild allocate decoded frame" << std::endl;
		exit(1);
	}
	
	
	//ffmpegDecoder.decodedFrameBGR = avcodec_alloc_frame();
	ffmpegDecoder.decodedFrameBGR = av_frame_alloc();
	if (ffmpegDecoder.decodedFrameBGR == NULL){
		std::cout << "faild allocate decoded frame RGB" << std::endl;
		exit(1);
	}

	ffmpegDecoder.buffer = (UINT8*)av_malloc(avpicture_get_size(AV_PIX_FMT_BGR24, 640, 368));
	avpicture_fill((AVPicture*)ffmpegDecoder.decodedFrameBGR, ffmpegDecoder.buffer, AV_PIX_FMT_BGR24, 640, 368);
	//pConvertCtx = sws_getContext(640, 368, ffmpegDecoder.codecCtx->pix_fmt,  640, 368, PIX_FMT_BGR24, SWS_SPLINE, NULL, NULL, NULL);
	ffmpegDecoder.convertCtx = sws_getContext(640, 368, ffmpegDecoder.codecCtx->pix_fmt,  640, 368, AV_PIX_FMT_BGR24, SWS_SPLINE, NULL, NULL, NULL);
	av_init_packet(&ffmpegDecoder.avPacket);
	std::cout << "initialize video finished" << std::endl;


}


void bebopCommand::decodingThread(){
	//TODO: デコードの処理速度の計測
	while (true){
		int finished = 0;
		int len = 0;

		videoPoolMtx.lock();
		VideoData *videoData = videoDataPool.takeData();
		videoPoolMtx.unlock();

		if (videoData == NULL){
			
			continue;
		}

		ffmpegDecoder.avPacket.data = videoData->frame;
		ffmpegDecoder.avPacket.size = videoData->size;
		
		while (ffmpegDecoder.avPacket.size > 0){
			//decoding

			len = avcodec_decode_video2(ffmpegDecoder.codecCtx, ffmpegDecoder.decodedFrame, &finished, &(ffmpegDecoder.avPacket));
			if (len > 0){
				if (finished){
					
					imageMtx.lock();
					//sws_scale(pConvertCtx, (const uchar* const*)ffmpegDecoder.decodedFrame->data, ffmpegDecoder.decodedFrame->linesize, 0, ffmpegDecoder.codecCtx->height, pFrameBGR->data, pFrameBGR->linesize);
					sws_scale(ffmpegDecoder.convertCtx, (const uchar* const*)ffmpegDecoder.decodedFrame->data, ffmpegDecoder.decodedFrame->linesize, 0, ffmpegDecoder.codecCtx->height, ffmpegDecoder.decodedFrameBGR->data, ffmpegDecoder.decodedFrameBGR->linesize);
					newImageFlag = true;
					imageMtx.unlock();
				}

				if (ffmpegDecoder.avPacket.data){
					ffmpegDecoder.avPacket.size -= len;
					ffmpegDecoder.avPacket.data += len;
				}
			} else {
				
				ffmpegDecoder.avPacket.size = 0;
				//delete[]ffmpegDecoder.avPacket.data;
			}
		}

	}
}


void bebopCommand::PCMDThread(){
	Command cmd;
	while (true){
		std::this_thread::sleep_for(std::chrono::microseconds(25));
		
		pcmdMtx.lock();
		initPCMD();
		generatePCMD(&cmd);
		sendCommand(&cmd);
		pcmdMtx.unlock();
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

void bebopCommand::move(int x, int y, int z, int r){
	Command cmd;

	pcmdMtx.lock();

	_pcmd.flag = 1;
    _pcmd.roll = x;
    _pcmd.pitch = y;
    _pcmd.yaw = r;
	_pcmd.gaz = z;
	_pcmd.psi = 0.0F;
	generatePCMD(&cmd);
	sendCommand(&cmd);

	pcmdMtx.unlock();
}

void bebopCommand::generatePCMD(bebopCommand::Command *command){
	//Command buf;
	UINT8 *temp;
	float val = _pcmd.psi;
	temp = (UINT8 *)&val;

	command->cmd = new UINT8[13];
	command->size = 13;
	command->typeSize = sizeof(UINT8);

	command->cmd[0] = ARCOMMANDS_ID_PROJECT_ARDRONE3;
	command->cmd[1] = ARCOMMANDS_ID_ARDRONE3_CLASS_PILOTING;
	command->cmd[2] = (UINT8)(ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_PCMD & 0xff);
	command->cmd[3] = (UINT8)((ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_PCMD & 0xff00) >> 8);
	command->cmd[4] = _pcmd.flag;
	command->cmd[5] = _pcmd.roll;
	command->cmd[6] = _pcmd.pitch;
	command->cmd[7] = _pcmd.yaw;
	command->cmd[8] = _pcmd.gaz;
	
	command->cmd[9] = temp[0];
	command->cmd[10] = temp[1];
	command->cmd[11] = temp[2];
	command->cmd[12] = temp[3];

	//return beforeSendCommandFilter(buf);	
}

void bebopCommand::moveCamera(short tilt, short pan){
	Command command;

	command.size = 6;
	command.cmd = new UINT8[command.size];
	command.typeSize = sizeof(UINT8);

	command.cmd[0] = ARCOMMANDS_ID_PROJECT_ARDRONE3;
	command.cmd[1] = ARCOMMANDS_ID_ARDRONE3_CLASS_CAMERA;

	command.cmd[2] = (UINT8)(ARCOMMANDS_ID_ARDRONE3_CAMERA_CMD_ORIENTATION & 0xff);
	command.cmd[3] = (UINT8)((ARCOMMANDS_ID_ARDRONE3_CAMERA_CMD_ORIENTATION & 0xff00) >> 8);
	command.cmd[4] = (UINT8)tilt;
	command.cmd[5] = (UINT8)pan;

	beforeSendCommandFilter(&command);
	sendCommand(&command);
	
}

//データ受信スレッド
void bebopCommand::recivingThread(){
	int len;
	
	//listen
	/*
	if (vodeoStream.listen() == -1){
		return;
	}
	*/
	
	//受信用バッファの初期化
	//int bufSize = 2048;
	//int bufSize = 70000;
	char *buf = new char[maxFragmentSize];
	auto timeSpan = endTime - startTime;
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(timeSpan).count();
	while (true){
		
		memset(buf, 0, maxFragmentSize);
		//receive data
		len = d2cSocket.receive(buf, maxFragmentSize);
		if (len == -1){
			return;
		}		
		
		NetworkFrame networkFrame;
		Command ack;
		//受信データの解析
		parseRawFrame(buf, &networkFrame);
	
		if (networkFrame.type == ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK){
		//	std::cout << "ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK" << std::endl;
			createAck(&networkFrame, &ack);
			sendCommand(&ack);
		}

		if (networkFrame.type == ARNETWORKAL_FRAME_TYPE_DATA_LOW_LATENCY && networkFrame.id == BD_NET_DC_VIDEO_DATA_ID){
			//std::cout << "on video" << std::endl;
			StreamFrame streamFrame;
			parseNetworkFrame(&networkFrame, &streamFrame);
			appendNewFrame(&streamFrame);
			createARStreamACK(&streamFrame, &ack);
			sendCommand(&ack);
		}
		
		if (networkFrame.id == BD_NET_DC_EVENT_ID){
			//std::cout << "event" << std::endl;
			parseEvent(&networkFrame);
		}
		

		if (networkFrame.id == BD_NET_DC_NAVDATA_ID){
			//std::cout << "get navdata" << std::endl;
			parseNavdata(&networkFrame);
		}

		if (networkFrame.id == ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PING){
			//std::cout << "ping" << std::endl; //debug
			createPong(&networkFrame, &ack);
			sendCommand(&ack);
			//takeOff();
		}
		delete[] networkFrame.data;

	}
	delete[] buf;
}

//受信フレームのパーサー
void bebopCommand::parseRawFrame(char* buf, bebopCommand::NetworkFrame *networkFrame){	
	networkFrame->type = (UINT8)buf[0];
	networkFrame->id = (UINT8)buf[1];
	networkFrame->seq = (UINT8)buf[2];
	networkFrame->size = readUInt32(buf, 3);
	networkFrame->dataSize = networkFrame->size - 7;
	
	if (networkFrame->size > 7){
		networkFrame->data = new UINT8[networkFrame->dataSize];		
		memcpy(networkFrame->data, &buf[7], networkFrame->dataSize);
	}	
}

void bebopCommand::createAck(bebopCommand::NetworkFrame *networkFrame, bebopCommand::Command *command){

	command->cmd = new UINT8[1];
	command->size = 1;
	command->typeSize = sizeof(UINT8);
	UINT8 id;	
	UINT8 t = ARNETWORKAL_MANAGER_DEFAULT_ID_MAX / 2 + networkFrame->id;
	
	command->cmd[0] = networkFrame->seq;
	id = (networkFrame->id + (int)(ARNETWORKAL_MANAGER_DEFAULT_ID_MAX / 2));
	
	beforeSendCommandFilter(command, ARNETWORKAL_FRAME_TYPE_ACK, id);	
}

cv::Mat bebopCommand::getImage(){

	cv::Mat matImage;

	imageMtx.lock();
	if (newImageFlag){
		IplImage *img = cvCreateImage(cvSize(640, 368), IPL_DEPTH_8U, 3);
		memcpy(img->imageData, ffmpegDecoder.decodedFrameBGR->data[0], 640 * 368 * sizeof(uchar) * 3);

		matImage = cv::cvarrToMat(img, true);
		cvReleaseImage(&img);
		newImageFlag = false;
	}
	imageMtx.unlock();
	return matImage;

}

void bebopCommand::beforeSendCommandFilter(bebopCommand::Command *cmd, int type, int id){
	
	UINT8 temp[4] = {0, 0, 0, 0};

	//cmd->size = cmd->size + 7;
	int bufSize = cmd->size + 7;
	UINT8 *buf = new UINT8[bufSize];
	

	//memcpy(buf, cmd->cmd, bufSize);	

	seq[id]++;
	if (seq[id] > 255) seq[id] = 0;

	buf[0] = (UINT8) type;
	buf[1] = (UINT8)id;
	buf[2] = seq[id];
	buf[3] = (UINT8)(bufSize & 0xff);
	buf[4] = (UINT8)((bufSize & 0xff00) >> 8);
	buf[5] = (UINT8)((bufSize & 0xff0000) >> 16);
	buf[6] = (UINT8)((bufSize & 0xff000000) >> 24);
	
	memcpy(&buf[7], cmd->cmd, cmd->size);
	delete[] cmd->cmd;
	cmd->cmd = new UINT8[bufSize];
	cmd->size = bufSize;
	memcpy(cmd->cmd, buf, cmd->size);

	delete[] buf;
}

void bebopCommand::generateAllStates(){
	
	Command buf;
	buf.cmd = new UINT8[4];
	buf.size = 4;
	buf.typeSize = sizeof(UINT8);

	std::cout << "generateAllStates" << std::endl;

	buf.cmd[0] = ARCOMMANDS_ID_PROJECT_COMMON;
	buf.cmd[1] = ARCOMMANDS_ID_COMMON_CLASS_COMMON;
	buf.cmd[2] = (ARCOMMANDS_ID_COMMON_COMMON_CMD_ALLSTATES & 0xff);
	buf.cmd[3] = (ARCOMMANDS_ID_COMMON_COMMON_CMD_ALLSTATES & 0xff00 >> 8);
	
	beforeSendCommandFilter(&buf);
	sendCommand(&buf);
}

void bebopCommand::createPong(NetworkFrame *networkFrame, bebopCommand::Command *pong){
	//UINT8 *temp = new UINT8(size);
	//Command buf;
	pong->size = networkFrame->size - 7;
	pong->cmd = new UINT8[pong->size];
	
	pong->typeSize = sizeof(UINT8);
	
	//for (int i = 0; i < pong->size; i++) pong->cmd[i] = networkFrame->data[i];
	memcpy(pong->cmd, networkFrame->data, networkFrame->size);
	beforeSendCommandFilter(pong, ARNETWORKAL_FRAME_TYPE_DATA, ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PONG);
	//return beforeSendCommandFilter(buf, ARNETWORKAL_FRAME_TYPE_DATA, ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PONG);
}

void bebopCommand::takeOff(){
	//UINT8 buf[4] = {ARCOMMANDS_ID_PROJECT_ARDRONE3, ARCOMMANDS_ID_ARDRONE3_CLASS_PILOTING, ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_TAKEOFF};
	Command buf;
	buf.cmd = new UINT8[4];
	buf.size = 4;
	buf.typeSize = sizeof(UINT8);

	buf.cmd[0] = ARCOMMANDS_ID_PROJECT_ARDRONE3;
	buf.cmd[1] = ARCOMMANDS_ID_ARDRONE3_CLASS_PILOTING;
	buf.cmd[2] = ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_TAKEOFF;
	buf.cmd[3] = 0;
	beforeSendCommandFilter(&buf);
	sendCommand(&buf);
	std::cout << "takeoff" << std::endl; //debug
}

void bebopCommand::landing(){

	Command buf;
	buf.cmd = new UINT8[4];
	buf.size = 4;
	buf.typeSize = sizeof(UINT8);

	buf.cmd[0] = ARCOMMANDS_ID_PROJECT_ARDRONE3;
	buf.cmd[1] = ARCOMMANDS_ID_ARDRONE3_CLASS_PILOTING;
	buf.cmd[2] = ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_LANDING;
	buf.cmd[3] = 0;
	beforeSendCommandFilter(&buf);
	sendCommand(&buf);
	std::cout << "landing" << std::endl;	//debug

}

void bebopCommand::sendCommand(Command *packet, std::string ip){
	udpMtx.lock();
	c2dSocket.send((char *)packet->cmd, packet->size);
	udpMtx.unlock();
	delete[] packet->cmd;
}

void bebopCommand::parseNetworkFrame(bebopCommand::NetworkFrame *networkFrame, bebopCommand::StreamFrame *streamFrame){

	streamFrame->frameNumber = readUInt16(networkFrame->data, 0);
	streamFrame->frameFlags = networkFrame->data[2];
	streamFrame->fragmentNumber = networkFrame->data[3];
	streamFrame->fragmentPerFrame = networkFrame->data[4];
	streamFrame->frameSize = networkFrame->dataSize - 5;
	streamFrame->frame = new UINT8[streamFrame->frameSize];

	memcpy(streamFrame->frame, &networkFrame->data[5], streamFrame->frameSize);

	//return frame;
}

void bebopCommand::appendNewFrame(bebopCommand::StreamFrame *streamFrame){
	if (streamFrame->frameNumber != arStreamFrame.frameNumber){
		
		if (arStreamFrame.fragmentCounter > 0){
			//びでお
			bool sendFlag = false;
			
			if (arStreamFrame.frameFlags == 1 || streamFrame->frameNumber > arStreamFrame.frameNumber){  //if (arStreamFrame.frameFlags == 1 || streamFrame->frameNumber > arStreamFrame.frameNumber){
				arStreamFrame.waitFlag = false;
				sendFlag = true;
				//std::cout << "frame number: " << streamFrame->frameNumber << std::endl;
			} else {
				arStreamFrame.waitFlag = true;
				//std::cout << "test aaaaaaaaaaaaaaaa" << std::endl;
				//std::cout << "number1: " << streamFrame->frameNumber << " number2: " << arStreamFrame.frameNumber << std::endl;
			}

			if (sendFlag){
				bool skip = false;
				//TODO: arStreamFrame.fram を一括確保したい今日このごろ
				for (int i = 0; i < arStreamFrame.fragmentPerFrame; i++){
					if (arStreamFrame.fragments[i].size < 0){
						skip = true;
						//std::cout << "break" << std::endl;
						break;
					}
					int tempSize = arStreamFrame.frameSize;
					UINT8 *temp = new UINT8[tempSize];
					memcpy(temp, arStreamFrame.frame, tempSize);

					delete[] arStreamFrame.frame;
					arStreamFrame.frameSize = tempSize + arStreamFrame.fragments[i].size;
					arStreamFrame.frame = new UINT8[arStreamFrame.frameSize];

					memcpy(arStreamFrame.frame, temp, tempSize);
					memcpy(&(arStreamFrame.frame[tempSize]), arStreamFrame.fragments[i].data, arStreamFrame.fragments[i].size);

					delete[] temp;
				}

				if (!skip){
					//ofs.write((char *)arStreamFrame.frame, arStreamFrame.frameSize);
					//int err = send(videoSocket.sock, (char *)arStreamFrame.frame, arStreamFrame.frameSize, 0);
					
					videoPoolMtx.lock();
					videoDataPool.addData(arStreamFrame.frame, arStreamFrame.frameSize);
					videoPoolMtx.unlock();
					
					//vodeoStream.send((char *)arStreamFrame.frame, arStreamFrame.frameSize);
					
				}
			}
			
		}
		//データの初期化
		
		for (int i = 0; i < arStreamFrame.fragmentPerFrame; i++) delete[] arStreamFrame.fragments[i].data;
		delete[] arStreamFrame.fragments;
		arStreamFrame.frameNumber = streamFrame->frameNumber;
		arStreamFrame.frameSize = 0;
		arStreamFrame.frameFlags = streamFrame->frameFlags;
		arStreamFrame.fragmentCounter = 0;
		arStreamFrame.fragments = new Fragment[streamFrame->fragmentPerFrame];
		arStreamFrame.fragmentPerFrame = streamFrame->fragmentPerFrame;
		delete[] arStreamFrame.frameACK;
		
		arStreamFrame.frameACK = new UINT8[16];
		

		memset(arStreamFrame.frameACK, 0, 16);
		
		
		
	}
}


void bebopCommand::createARStreamACK(StreamFrame *streamFrame, bebopCommand::Command *command){	
	
	
	delete[] arStreamFrame.fragments[streamFrame->fragmentNumber].data;
	arStreamFrame.fragments[streamFrame->fragmentNumber].data = new UINT8[streamFrame->frameSize];
	memcpy(arStreamFrame.fragments[streamFrame->fragmentNumber].data, streamFrame->frame, streamFrame->frameSize);
	arStreamFrame.fragments[streamFrame->fragmentNumber].size = streamFrame->frameSize;
	arStreamFrame.fragmentCounter++;

	int bufferPosition = (int)floor(streamFrame->fragmentNumber / 8 | 0);
	arStreamFrame.frameACK[bufferPosition] |= (UINT8)(1 << streamFrame->fragmentNumber % 8);
	
	delete[] streamFrame->frame;
//	uint32_t retVal;

	
	
	AckPacket ack;
	ack.frameNumber = htons(arStreamFrame.frameNumber);
	ack.packetsACK = new UINT8[16];

	memcpy(ack.packetsACK, &arStreamFrame.frameACK[8], 8);
	memcpy(&ack.packetsACK[8], arStreamFrame.frameACK, 8);
	
	//Command ret;
	command->cmd = new UINT8[18];
	command->size = 18;
	command->typeSize = sizeof(UINT8);

	memset(command->cmd, 0, 18);

	command->cmd[0] = (UINT8)(ack.frameNumber & 0xff);
	command->cmd[1] = (UINT8)((ack.frameNumber & 0xff00) >> 8);
	memcpy(&(command->cmd[2]), ack.packetsACK, 16);
	beforeSendCommandFilter(command,  ARNETWORKAL_FRAME_TYPE_DATA, BD_NET_CD_VIDEO_ACK_ID);
	
}

void bebopCommand::flatTrim(){
	Command command;

	command.cmd = new UINT8[4];
	command.size = 4;
	
	command.cmd[0] = ARCOMMANDS_ID_PROJECT_ARDRONE3;
	command.cmd[1] = ARCOMMANDS_ID_ARDRONE3_CLASS_PILOTING;
	command.cmd[2] = (UINT8)(ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_FLATTRIM & 0xff);
	command.cmd[3] = (UINT8)(ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_FLATTRIM & 0xff00 >> 8);
	beforeSendCommandFilter(&command);
	sendCommand(&command);
}

int bebopCommand::getBatteryCharge(){
	navdataMtx.lock();
	return navData.battery;
	navdataMtx.unlock();
}

void bebopCommand::emergency(){
	Command command;

	command.cmd = new UINT8[4];
	command.size = 4;

	command.cmd[0] = ARCOMMANDS_ID_PROJECT_ARDRONE3;
	command.cmd[1] = ARCOMMANDS_ID_ARDRONE3_CLASS_PILOTING;
	command.cmd[2] = (UINT8)(ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_EMERGENCY & 0xff);
	command.cmd[3] = (UINT8)(ARCOMMANDS_ID_ARDRONE3_PILOTING_CMD_EMERGENCY & 0xff00 >> 8);

	beforeSendCommandFilter(&command);
	sendCommand(&command);

}

void bebopCommand::parseEvent(bebopCommand::NetworkFrame *networkFrame){
	if (networkFrame->data[0] == ARCOMMANDS_ID_PROJECT_ARDRONE3){
		if (networkFrame->data[1] == ARCOMMANDS_ID_ARDRONE3_CLASS_PILOTINGSTATE){
			if (readUInt16(networkFrame->data, 2) == ARCOMMANDS_ID_ARDRONE3_PILOTINGSTATE_CMD_SPEEDCHANGED){
				std::cout << "x:" << (float)readUInt32(networkFrame->data, 4)<< std::endl;
				std::cout << "y:" << (float)readUInt32(networkFrame->data, 8) << std::endl;
				std::cout << "z:" << (float)readUInt32(networkFrame->data, 12) << std::endl;
			}
			if (readUInt16(networkFrame->data, 2) == ARCOMMANDS_ID_ARDRONE3_PILOTINGSTATE_CMD_FLYINGSTATECHANGED){
				navdataMtx.lock();
				switch (readUInt32(networkFrame->data, 4))
				{
			
				case ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_LANDED:
					navData.flyingState = ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_LANDED;
					break;

				case ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_TAKINGOFF:
					navData.flyingState = ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_TAKINGOFF;
					break;

				case ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_HOVERING:
					navData.flyingState = ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_HOVERING;
					break;

				case ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_FLYING:
					navData.flyingState = ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_FLYING;
					break;

				case ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_LANDING:
					navData.flyingState = ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_LANDING;
					break;

				case ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_EMERGENCY:
					navData.flyingState = ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_EMERGENCY;
					emergency();
					break;

				default:
					break;
				}
				navdataMtx.unlock();
			}
		}

		if (networkFrame->data[1] ==  ARCOMMANDS_ID_ARDRONE3_CLASS_SETTINGSSTATE){
			if (readUInt16(networkFrame->data, 2) == ARCOMMANDS_ID_ARDRONE3_CLASS_SETTINGSSTATE_CMD_MOTORERRORSTATECHANGED){
				if (networkFrame->data[4] & 1){
					std::cout << "motor 1 is affected by this error" << std::endl;
				}
				if (networkFrame->data[4] & 2){
					std::cout << "motor 2 is affected by this error" << std::endl;
				}
				if (networkFrame->data[4] & 4){
					std::cout << "motor 3 is affected by this error" << std::endl;
				}
				if (networkFrame->data[4] & 8){
					std::cout << "motor 4 is affected by this error" << std::endl;
				}

				switch (readUInt32(networkFrame->data, 5))
				{
				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_NOERROR:

					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERROREEPROM:
					std::cout << "EEPROM access failure" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORMOTORSTALLED:
					std::cout << "Motor stalled" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORPROPELLERSECURITY:
					std::cout << "Propeller cutout security triggered" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORCOMMLOST:
					std::cout << "Communication with motor failed by timeout" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORRCEMERGECYSTOP:
					std::cout << "RC emergency stop" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORREALTIME:
					std::cout << "Motor controler scheduler real-time out of bounds" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORMOTORSETTING:
					std::cout << "One or several incorrect values in motor settings" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORTEMPERATURE:
					std::cout << "Too hot or too cold Cypress temperature" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORBATTERYVOLTAGE:
					std::cout << "Battery voltage out of bounds" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORLIPOCELLS:
					std::cout << "Incorrect number of LIPO cells" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORMOSFET:
					std::cout << "Defectuous MOSFET or broken motor phases" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORBOOTLOADER:
					std::cout << "Not use for BLDC but useful for HAL" << std::endl;
					break;

				case ARCOMMANDS_ID_ARDRONE3_SETTINGSSTATE_MOTORERRORSTATECHANGED_MOTORERROR_ERRORASSERT:
					std::cout << "Error Made by BLDC_ASSERT()" << std::endl;
					break;

				default:
					break;
				}
			}

		}
	}
	else if (networkFrame->data[0] == ARCOMMANDS_ID_PROJECT_COMMON){
		if (networkFrame->data[1] == ARCOMMANDS_ID_COMMON_CLASS_COMMONSTATE){
			if (readUInt16(networkFrame->data, 2) == ARCOMMANDS_ID_COMMON_COMMONSTATE_CMD_BATTERYSTATECHANGED){
				//std::cout << "battery:" << (int)networkFrame.data[4] << std::endl;
				navdataMtx.lock();
				navData.battery = (int)networkFrame->data[4];
				navdataMtx.unlock();

			}
		}
	}
}

void bebopCommand::parseNavdata(bebopCommand::NetworkFrame *networkFrame){
	if ( networkFrame->data[0] == ARCOMMANDS_ID_PROJECT_ARDRONE3){
		navdataMtx.lock();
		if ( networkFrame->data[1] == ARCOMMANDS_ID_ARDRONE3_CLASS_PILOTINGSTATE){
			if ( networkFrame->data[2] == ARCOMMANDS_ID_ARDRONE3_PILOTINGSTATE_CMD_SPEEDCHANGED){
				navData.vx = (float)readUInt32(networkFrame->data , 4 );
				navData.vy = (float)readUInt32(networkFrame->data , 8 );
				navData.vz = (float)readUInt32(networkFrame->data , 12 );
			}
			else if ( networkFrame->data[2] == 6 ){
				navData.phi = (float)readUInt32(networkFrame->data , 4 );
				navData.theta = (float)readUInt32(networkFrame->data , 8 );
				navData.psi = (float)readUInt32(networkFrame->data , 12 );
			}
		}
		navdataMtx.unlock();
	}
}

void bebopCommand::getVlocity(double *vx, double *vy, double *vz){
	navdataMtx.lock();
	*vx = navData.vx;
	*vy = navData.vy;
	*vz = navData.vz;
	navdataMtx.unlock();
}

bool bebopCommand::onGround(){
	navdataMtx.lock();
	bool flag = navData.flyingState == ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_LANDED ? true : false;
	navdataMtx.unlock();

	return flag;
}

UINT32 bebopCommand::readUInt32(UINT8 *byte, int index){
	return (UINT32)((byte[index] << 0) | (byte[index + 1] << 8) | (byte[index + 2] << 16) | (byte[index + 3] << 24));
}

UINT32 bebopCommand::readUInt32(char *byte, int index){
	return (UINT32)((UINT8)(byte[index] << 0) | ((UINT8)byte[index + 1] << 8) | ((UINT8)byte[index + 2] << 16) | ((UINT8)byte[index + 3] << 24));
}

UINT16 bebopCommand::readUInt16(UINT8 *byte, int index){
	return (UINT16)((byte[index] << 0) | (byte[index + 1] << 8));
}

UINT16 bebopCommand::readUInt16(char *byte, int index){
	return (UINT16)((UINT8)(byte[index] << 0) | (UINT8)(byte[index + 1] << 8));
}