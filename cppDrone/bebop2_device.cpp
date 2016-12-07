#define TAG "bebop2_device"

#include <stdlib.h>
#include <string.h>
#include <opencv2/highgui.hpp>

extern "C" {
#include <libARSAL/ARSAL.h>
#include <libARController/ARController.h>
#include <libARDiscovery/ARDiscovery.h>
}

#include "bebop2_device.h"

static bool isBebopRunning;

ARSAL_Sem_t stateSem;

eARCONTROLLER_ERROR start_bebop2(ARCONTROLLER_Device_t** aDeviceController, ARCONTROLLER_DICTIONARY_CALLBACK_t aCommandReceivedCallback, bebop_driver::VideoDecoder* aVideoDecoder, ARCONTROLLER_Stream_DidReceiveFrameCallback_t aDidReceiveFrameCallback)
{
	// local declarations
	auto error = ARCONTROLLER_OK;
	int failed = 0;
	ARDISCOVERY_Device_t* device = nullptr;
	ARCONTROLLER_Device_t* deviceController = nullptr;

	ARSAL_Sem_Init(&(stateSem), 0, 0);

	ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- Bebop 2 Piloting --");

	// create a discovery device
	if (!failed)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- init discovey device ... ");
		eARDISCOVERY_ERROR errorDiscovery = ARDISCOVERY_OK;

		device = ARDISCOVERY_Device_New(&errorDiscovery);

		if (errorDiscovery == ARDISCOVERY_OK)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - ARDISCOVERY_Device_InitWifi ...");
			// create a Bebop drone discovery device (ARDISCOVERY_PRODUCT_ARDRONE)

			errorDiscovery = ARDISCOVERY_Device_InitWifi(device, ARDISCOVERY_PRODUCT_BEBOP_2, "bebop2", BEBOP_IP_ADDRESS, BEBOP_DISCOVERY_PORT);
			if (errorDiscovery != ARDISCOVERY_OK)
			{
				failed = 1;
				ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Discovery error :%s", ARDISCOVERY_Error_ToString(errorDiscovery));
			}
		}
		else
		{
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Discovery error :%s", ARDISCOVERY_Error_ToString(errorDiscovery));
			failed = 1;
		}
	}

	// create a device controller
	if (!failed)
	{
		deviceController = ARCONTROLLER_Device_New(device, &error);

		if (error != ARCONTROLLER_OK)
		{
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Creation of deviceController failed.");
			failed = 1;
		}
		else
		{
			*aDeviceController = deviceController;
		}
	}

	if (!failed)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- delete discovey device ... ");
		ARDISCOVERY_Device_Delete(&device);
	}

	// add the state change callback to be informed when the device controller starts, stops...
	if (!failed)
	{	
		error = ARCONTROLLER_Device_AddStateChangedCallback(deviceController, state_changed_callback, deviceController);

		if (error != ARCONTROLLER_OK)
		{
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "add State callback failed.");
			failed = 1;
		}
	}

	// add the command received callback to be informed when a command has been received from the device
	if (!failed)
	{
		error = ARCONTROLLER_Device_AddCommandReceivedCallback(deviceController, aCommandReceivedCallback, deviceController);

		if (error != ARCONTROLLER_OK)
		{
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "add callback failed.");
			failed = 1;
		}
	}

	// add the frame received callback to be informed when a streaming frame has been received from the device
	if (!failed)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- set Video callback ... ");
		error = ARCONTROLLER_Device_SetVideoStreamCallbacks(
			deviceController,
			[](ARCONTROLLER_Stream_Codec_t codec, void *customData)
			{
				auto *decoder = static_cast<bebop_driver::VideoDecoder*>(customData);

				if (decoder == nullptr)
				{
					ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "VideoDecoder is NULL.");
					return ARCONTROLLER_OK;
				}

				if (codec.type == ARCONTROLLER_STREAM_CODEC_TYPE_H264)
				{
					if (!decoder->SetH264Params(codec.parameters.h264parameters.spsBuffer, codec.parameters.h264parameters.spsSize, codec.parameters.h264parameters.ppsBuffer, codec.parameters.h264parameters.ppsSize))
					{
						ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "SetH264Params has failed.");
					}
				}
				return ARCONTROLLER_OK;
			},
			aDidReceiveFrameCallback,
			nullptr,
			aVideoDecoder);

		if (error != ARCONTROLLER_OK)
		{
			failed = 1;
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%", ARCONTROLLER_Error_ToString(error));
		}
	}

	if (!failed)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Connecting ...");
		error = ARCONTROLLER_Device_Start(deviceController);

		if (error != ARCONTROLLER_OK)
		{
			failed = 1;
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
		}
	}

	if (!failed)
	{
		// wait state update update
		ARSAL_Sem_Wait(&(stateSem));

		auto deviceState = ARCONTROLLER_Device_GetState(deviceController, &error);

		if ((error != ARCONTROLLER_OK) || (deviceState != ARCONTROLLER_DEVICE_STATE_RUNNING))
		{
			failed = 1;
			error = ARCONTROLLER_ERROR_STATE;
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- deviceState :%d", deviceState);
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
		}
	}

	// send the command that tells to the Bebop to begin its streaming
	if (!failed)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- send StreamingVideoEnable ... ");
		error = deviceController->aRDrone3->sendPictureSettingsVideoStabilizationMode(deviceController->aRDrone3, ARCOMMANDS_ARDRONE3_PICTURESETTINGS_VIDEOSTABILIZATIONMODE_MODE_NONE);
		error = deviceController->aRDrone3->sendPictureSettingsVideoFramerate(deviceController->aRDrone3, ARCOMMANDS_ARDRONE3_PICTURESETTINGS_VIDEOFRAMERATE_FRAMERATE_24_FPS);
		error = deviceController->aRDrone3->sendPictureSettingsVideoResolutions(deviceController->aRDrone3, ARCOMMANDS_ARDRONE3_PICTURESETTINGS_VIDEORESOLUTIONS_TYPE_REC720_STREAM720);
		error = deviceController->aRDrone3->sendMediaStreamingVideoStreamMode(deviceController->aRDrone3, ARCOMMANDS_ARDRONE3_MEDIASTREAMING_VIDEOSTREAMMODE_MODE_HIGH_RELIABILITY);
		for(int cur_tilt = 0; cur_tilt >= -40; cur_tilt -= 5)
		{
			deviceController->aRDrone3->setCameraOrientationTilt(deviceController->aRDrone3, cur_tilt);
			Sleep(200);
		}
		error = deviceController->aRDrone3->sendMediaRecordVideoV2(deviceController->aRDrone3, ARCOMMANDS_ARDRONE3_MEDIARECORD_VIDEOV2_RECORD_START);
		error = deviceController->aRDrone3->sendMediaStreamingVideoEnable(deviceController->aRDrone3, 1);
		if (error != ARCONTROLLER_OK)
		{
			failed = 1;
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
		}
	}

	// send the command that for safety
	if (!failed)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- send SettingsMax ... ");
		error = deviceController->aRDrone3->sendPilotingSettingsNoFlyOverMaxDistance(deviceController->aRDrone3, 1);
		error = deviceController->aRDrone3->sendPilotingSettingsMaxDistance(deviceController->aRDrone3, 5.0);
		error = deviceController->aRDrone3->sendPilotingSettingsMaxAltitude(deviceController->aRDrone3, 2.0);
		if (error != ARCONTROLLER_OK)
		{
			failed = 1;
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
		}
	}

	return error;
}

eARCONTROLLER_ERROR finish_bebop2(ARCONTROLLER_Device_t* deviceController)
{
	auto error = ARCONTROLLER_OK;
	eARCONTROLLER_DEVICE_STATE deviceState;

	// we are here because of a disconnection or user has quit IHM, so safely delete everything
	if (deviceController != nullptr)
	{
		deviceState = ARCONTROLLER_Device_GetState(deviceController, &error);
		if ((error == ARCONTROLLER_OK) && (deviceState != ARCONTROLLER_DEVICE_STATE_STOPPED))
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Disconnecting ...");

			error = ARCONTROLLER_Device_Stop(deviceController);

			if (error == ARCONTROLLER_OK)
			{
				// wait state update update
				ARSAL_Sem_Wait(&(stateSem));
			}
		}

		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "ARCONTROLLER_Device_Delete ...");
		ARCONTROLLER_Device_Delete(&deviceController);
	}

	ARSAL_Sem_Destroy(&(stateSem));

	ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- END --");

	return error;
}

void keyboard_controller_loop(ARCONTROLLER_Device_t *deviceController, const char *cvWindowName)
{
	static int cur_pan = 0;
	static int cur_tilt = 0;

	if (deviceController == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "DeviceController is NULL.");
		return;
	}

	cvNamedWindow(cvWindowName);

	auto control = true;
	while (control) {
		char key = cv::waitKey(1);
		// Manage IHM input events
		auto error = ARCONTROLLER_OK;

		switch (key)
		{
		case 'q':
		case 'Q':
			// send a landing command to the drone
			error = deviceController->aRDrone3->sendPilotingLanding(deviceController->aRDrone3);
			control = false;
			break;
		case 'e':
		case 'E':
			// send a Emergency command to the drone
			error = deviceController->aRDrone3->sendPilotingEmergency(deviceController->aRDrone3);
			break;
		case 'l':
		case 'L':
			// send a landing command to the drone
			error = deviceController->aRDrone3->sendPilotingLanding(deviceController->aRDrone3);
			break;
		case 't':
		case 'T':
			// send a takeoff command to the drone
			error = deviceController->aRDrone3->sendPilotingTakeOff(deviceController->aRDrone3);
			break;
		case 'u': // UP
		case 'U':
			// set the flag and speed value of the piloting command
			error = deviceController->aRDrone3->setPilotingPCMDGaz(deviceController->aRDrone3, 100);
			break;
		case 'j': // DOWN
		case 'J':
			error = deviceController->aRDrone3->setPilotingPCMDGaz(deviceController->aRDrone3, -100);
			break;
		case 'k': // RIGHT
		case 'K':
			error = deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, 100);
			break;
		case 'h':
		case 'H':
			error = deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, -100);
			break;
		case 'r': //IHM_INPUT_EVENT_FORWARD
		case 'R':
			error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDPitch(deviceController->aRDrone3, 100);
			break;
		case 'f': //IHM_INPUT_EVENT_BACK:
		case 'F':
			error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDPitch(deviceController->aRDrone3, -100);
			break;
		case 'd': //IHM_INPUT_EVENT_ROLL_LEFT:
		case 'D':
			error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDRoll(deviceController->aRDrone3, -100);
			break;
		case 'g': //IHM_INPUT_EVENT_ROLL_RIGHT:
		case 'G':
			error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDRoll(deviceController->aRDrone3, 100);
			break;
		case '1':
			error = deviceController->aRDrone3->sendMediaStreamingVideoEnable(deviceController->aRDrone3, 1);
			break;
		case '2':
			error = deviceController->aRDrone3->sendMediaStreamingVideoEnable(deviceController->aRDrone3, 0);
			break;
		case 'n':
		case 'N':
			if(--cur_tilt < -83)
			{
				cur_tilt = -83;
			}
			deviceController->aRDrone3->setCameraOrientationTilt(deviceController->aRDrone3, cur_tilt);
			break;
		case 'm':
		case 'M':
			if(++cur_tilt > 17)
			{
				cur_tilt = 17;
			}
			deviceController->aRDrone3->setCameraOrientationTilt(deviceController->aRDrone3, cur_tilt);
			break;
		case 'v':
		case 'V':
			if(--cur_pan < -35)
			{
				cur_pan = -35;
			}
			deviceController->aRDrone3->setCameraOrientationPan(deviceController->aRDrone3, cur_pan);
			break;
		case 'b':
		case 'B':
			if(++cur_pan > 35)
			{
				cur_pan = 35;
			}
			deviceController->aRDrone3->setCameraOrientationPan(deviceController->aRDrone3, cur_pan);
			break;
		default:
			error = deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			break;
		}

		Sleep(10);

		// This should be improved, here it just displays that one error occured
		if (error != ARCONTROLLER_OK)
		{
			printf("Error sending an event: %s\n", ARCONTROLLER_Error_ToString(error));
		}
	}
}

static void state_changed_callback(eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData)
{
	ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - stateChanged newState: %d .....", newState);

	switch (newState)
	{
	case ARCONTROLLER_DEVICE_STATE_STOPPED:
		isBebopRunning = false;
		ARSAL_Sem_Post(&(stateSem));
		break;

	case ARCONTROLLER_DEVICE_STATE_RUNNING:
		isBebopRunning = true;
		ARSAL_Sem_Post(&(stateSem));
		break;

	default:
		break;
	}
}
