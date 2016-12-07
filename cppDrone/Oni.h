#pragma once
#include "bebop2_device.h"
#include "bebop_video_decoder.h"
#include "StateController.h"
#include "OniTracker.h"

#define MONITOR_WINDOW_NAME "Drone Monitor"

#define CONTROLLER_WINDOW_NAME "Oni Controller"
#define CONTROLLER_IMAGE_FILE "controller.png"

class Oni
{
// Models
public:
	enum OniCommand
	{
		None = 0,
		Emergency,
		TakeOff,
		Search,
		Land,
		Disconnect,
	};

// Members
private:
	ARCONTROLLER_Device_t* mDeviceController;
	bebop_driver::VideoDecoder* mVideoDecoder;
	StateController* mStateController;
	OniCommand mReceivedCommand = None;
	OniTracker* mTracker;

	ARCONTROLLER_DICTIONARY_CALLBACK_t cEvent;
	ARCONTROLLER_Stream_DidReceiveFrameCallback_t cFrame;

	HANDLE hThread[2];
	DWORD hThreadId[2];

// Methods
public:
	static Oni* createOni()
	{
		auto oni = new Oni();

		auto startError = start_bebop2(&oni->mDeviceController, oni->cEvent, oni->mVideoDecoder, oni->cFrame);
		if(startError != ARCONTROLLER_OK)
		{
			return nullptr;
		}
		oni->mStateController = new StateController(oni->mDeviceController);

		return oni;
	}

	void startOni()
	{
		hThread[0] = CreateThread(nullptr, 0, user_command_loop, this, 0, &hThreadId[0]);
		hThread[1] = CreateThread(nullptr, 0, oni_state_loop, this, 0, &hThreadId[1]);

		WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

		CloseHandle(hThread[0]);
		CloseHandle(hThread[1]);
	}

	cv::Mat getCameraImage(double ratioX = 0.3, double ratioY = 0.3) const;

private:
	
	static DWORD WINAPI user_command_loop(LPVOID lpParam);

	static DWORD WINAPI oni_state_loop(LPVOID lpParam);

	static void oni_event_loop(eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData);

	static eARCONTROLLER_ERROR oni_image_loop(ARCONTROLLER_Frame_t *frame, void *customData);

// Constructors
private:
	Oni(): mDeviceController(nullptr), cEvent(oni_event_loop), cFrame(oni_image_loop)
	{
		mVideoDecoder = new bebop_driver::VideoDecoder();
		mStateController = new StateController(mDeviceController);
		mTracker = new OniTracker();
	}
};
