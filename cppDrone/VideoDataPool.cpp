#include "VideoDataPool.h"


VideoDataPool::VideoDataPool(void)
{
	readIndex = 0;
	writeIndex = 0;
}


VideoDataPool::~VideoDataPool(void)
{
}

void VideoDataPool::addData(UINT8 *data, int size){
	pool[writeIndex].frame = new UINT8[size];
	pool[writeIndex].size = size;

	memcpy(pool[writeIndex].frame, data, size);
	
	writeIndex = (writeIndex + 1) >= POOL_SIZE ? 0 : writeIndex + 1;
}

VideoData* VideoDataPool::takeData(){
	VideoData *dataPtr = NULL;

	if (readIndex == writeIndex) return dataPtr;
	
	dataPtr = &pool[readIndex];
	readIndex = (readIndex + 1) >= POOL_SIZE ? 0 : readIndex + 1;
	return dataPtr;
}
