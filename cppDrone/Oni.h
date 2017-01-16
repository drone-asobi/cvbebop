#pragma once
#include "bebop2_device.h"
#include "bebop_video_decoder.h"
#include "StateController.h"
#include "OniTracker.h"

#define MONITOR_WINDOW_NAME "Drone Monitor"

#define COOL_SCREEN_WINDOW_NAME "DRONE_TAGGER"

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

	struct DroneStatus
	{
		int battery;
		int framerate;
		int resolution;
		int tilt;
		int pan;
		float speedX;
		float speedY;
		float speedZ;
		float roll;
		float pitch;
		float yaw;
		double altitude;
		int dX;
		int dY;
		int dZ;
		int dPsi;
		cv::Rect currentTarget;
	};

// Members
private:
	ARCONTROLLER_Device_t* mDeviceController;
	bebop_driver::VideoDecoder* mVideoDecoder;
	StateController* mStateController;
	OniCommand mReceivedCommand = None;
	OniTracker* mTracker;

	DroneStatus* mDroneStatus;

	ARCONTROLLER_DICTIONARY_CALLBACK_t cEvent;
	ARCONTROLLER_Stream_DidReceiveFrameCallback_t cFrame;

	HANDLE hThread[2];
	DWORD hThreadId[2];

// Methods
public:
	static Oni* createOni()
	{
		auto oni = new Oni();

		auto startError = start_bebop2(&oni->mDeviceController, oni->cEvent, oni->mVideoDecoder, oni->cFrame, (void *)oni->mDroneStatus);
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

	cv::Mat getCameraImage(double ratioX, double ratioY) const;

private:
	static DWORD WINAPI user_command_loop(LPVOID lpParam);

	static DWORD WINAPI oni_state_loop(LPVOID lpParam);

	static void oni_event_loop(eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData);

	static eARCONTROLLER_ERROR oni_image_loop(ARCONTROLLER_Frame_t *frame, void *customData);

private:
	static void processCoolScreen(Oni * oni, const cv::Mat& wanted_base);

	static void processStateStart(StateController::STATE_PARAMETER*& currentParameter);
	static void processStateReady(StateController::STATE_PARAMETER*& currentParameter, Oni::OniCommand command);
	static void processStateTakingOff(StateController::STATE_PARAMETER*& currentParameter);
	static void processStateHovering(StateController::STATE_PARAMETER*& currentParameter, Oni::OniCommand command);
	static void processStateSearching(Oni* oni, StateController::STATE_PARAMETER*& currentParameter);
	static void processStateTracking(Oni* oni, StateController::STATE_PARAMETER*& currentParameter);
	static void processStateMissing(Oni* oni, StateController::STATE_PARAMETER*& currentParameter);
	static void processStateCaptured(Oni* oni, StateController::STATE_PARAMETER*& currentParameter);
	static void processStateLanding(Oni* oni, StateController::STATE_PARAMETER*& currentParameter);
	static void processStateFinished(Oni* oni, StateController::STATE_PARAMETER*& currentParameter);

// Constructors
private:
	Oni(): mDeviceController(nullptr), cEvent(oni_event_loop), cFrame(oni_image_loop)
	{
		mVideoDecoder = new bebop_driver::VideoDecoder();
		mStateController = new StateController(mDeviceController);
		mTracker = new OniTracker();
		mDroneStatus = new DroneStatus;
		memset(mDroneStatus, 0, sizeof(mDroneStatus));
	}
};
