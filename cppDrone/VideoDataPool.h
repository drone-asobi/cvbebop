#pragma once
#include <iostream>
#include <basetsd.h>

#define POOL_SIZE 500

typedef struct {
	UINT8 *frame;
	int size;
}VideoData;

class VideoDataPool
{
private:
	VideoData pool[POOL_SIZE];
	int readIndex;
	int writeIndex;


public:
	VideoDataPool(void);
	~VideoDataPool(void);
	void addData(UINT8 *data, int size);
	VideoData* takeData();

};

