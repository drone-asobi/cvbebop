#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

extern "C" {
#include <libARSAL/ARSAL.h>
#include <libARController/ARController.h>
#include <libARDiscovery/ARDiscovery.h>
}

#include "BebopController.h"

/*****************************************
*
*             define :
*
*****************************************/
#define TAG "BebopController"

#define ERROR_STR_LENGTH 2048

#define BEBOP_IP_ADDRESS "192.168.42.1"
#define BEBOP_DISCOVERY_PORT 44444

#define FIFO_DIR_PATTERN "./"
#define FIFO_NAME "arsdk_fifo"

/*****************************************
*
*             implementation :
*
*****************************************/

static char fifo_dir[] = FIFO_DIR_PATTERN;
static char fifo_name[128] = "bebopvideo.bin";

int gIHMRun = 1;
char gErrorStr[ERROR_STR_LENGTH];

FILE *videoOut = NULL;
int frameNb = 0;
ARSAL_Sem_t stateSem;
int isBebop2 = 1;

ARCONTROLLER_Device_t* start_control_bebop2(eARCONTROLLER_ERROR &error)
{
	// local declarations
	error = ARCONTROLLER_OK;
	int failed = 0;
	ARDISCOVERY_Device_t *device = nullptr;
	ARCONTROLLER_Device_t *deviceController = nullptr;

	ARSAL_Sem_Init(&(stateSem), 0, 0);

	ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- Bebop 2 Piloting --");

	videoOut = fopen(fifo_name, "w");

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
	}

	if (!failed)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- delete discovey device ... ");
		ARDISCOVERY_Device_Delete(&device);
	}

	// add the state change callback to be informed when the device controller starts, stops...
	if (!failed)
	{
		error = ARCONTROLLER_Device_AddStateChangedCallback(deviceController, stateChanged, deviceController);

		if (error != ARCONTROLLER_OK)
		{
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "add State callback failed.");
			failed = 1;
		}
	}

	// add the command received callback to be informed when a command has been received from the device
	if (!failed)
	{
		error = ARCONTROLLER_Device_AddCommandReceivedCallback(deviceController, commandReceived, deviceController);

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
		error = ARCONTROLLER_Device_SetVideoStreamCallbacks(deviceController, decoderConfigCallback, didReceiveFrameCallback, NULL, NULL);

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
		error = deviceController->aRDrone3->sendMediaStreamingVideoEnable(deviceController->aRDrone3, 1);
		if (error != ARCONTROLLER_OK)
		{
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
			failed = 1;
		}
	}

//	if (!failed)
//	{
//		IHM_PrintInfo(ihm, "Running ... ('t' to takeoff ; Spacebar to land ; 'e' for emergency ; Arrow keys and ('r','f','d','g') to move ; 'q' to quit)");
//
//#ifdef IHM
//		while (gIHMRun)
//		{
//			_sleep(50);
//		}
//#else
//		int i = 20;
//		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- sleep 20 ... ");
//		while (gIHMRun && i--)
//			sleep(1);
//#endif
//	}
//
//#ifdef IHM
//	IHM_Delete(&ihm);
//#endif

	return deviceController;
}

eARCONTROLLER_ERROR finish_control_bebop2(ARCONTROLLER_Device_t* deviceController)
{
	eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
	eARCONTROLLER_DEVICE_STATE deviceState = ARCONTROLLER_DEVICE_STATE_MAX;

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

		fflush(videoOut);
		fclose(videoOut);
	}

	ARSAL_Sem_Destroy(&(stateSem));

	ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- END --");

	return error;
}

/*****************************************
*
*             private implementation:
*
****************************************/

// called when the state of the device controller has changed
void stateChanged(eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData)
{
	ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - stateChanged newState: %d .....", newState);

	switch (newState)
	{
	case ARCONTROLLER_DEVICE_STATE_STOPPED:
		ARSAL_Sem_Post(&(stateSem));
		//stop
		gIHMRun = 0;

		break;

	case ARCONTROLLER_DEVICE_STATE_RUNNING:
		ARSAL_Sem_Post(&(stateSem));
		break;

	default:
		break;
	}
}

// called when a command has been received from the drone
void commandReceived(eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData)
{
	ARCONTROLLER_Device_t *deviceController = static_cast<ARCONTROLLER_Device_t*>(customData);

	if (deviceController != NULL)
	{
		// if the command received is a battery state changed
		if (commandKey == ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED)
		{
			ARCONTROLLER_DICTIONARY_ARG_t *arg = NULL;
			ARCONTROLLER_DICTIONARY_ELEMENT_t *singleElement = NULL;

			if (elementDictionary != NULL)
			{
				// get the command received in the device controller
				HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, singleElement);

				if (singleElement != NULL)
				{
					// get the value
					HASH_FIND_STR(singleElement->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED_PERCENT, arg);

					if (arg != NULL)
					{
						// update UI
						batteryStateChanged(arg->value.U8);
					}
					else
					{
						ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "arg is NULL");
					}
				}
				else
				{
					ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "singleElement is NULL");
				}
			}
			else
			{
				ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "elements is NULL");
			}
		}
	}

	if (commandKey == ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED)
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = NULL;

		if (elementDictionary != NULL)
		{
			ARCONTROLLER_DICTIONARY_ELEMENT_t *dictElement = NULL;
			ARCONTROLLER_DICTIONARY_ELEMENT_t *dictTmp = NULL;

			eARCOMMANDS_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORNAME sensorName = ARCOMMANDS_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORNAME_MAX;
			int sensorState = 0;

			HASH_ITER(hh, elementDictionary, dictElement, dictTmp)
			{
				// get the Name
				HASH_FIND_STR(dictElement->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORNAME, arg);
				if (arg != NULL)
				{
					sensorName = static_cast<eARCOMMANDS_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORNAME>(arg->value.I32);
				}
				else
				{
					ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "arg sensorName is NULL");
				}

				// get the state
				HASH_FIND_STR(dictElement->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORSTATE, arg);
				if (arg != NULL)
				{
					sensorState = arg->value.U8;

					ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "sensorName %d ; sensorState: %d", sensorName, sensorState);
				}
				else
				{
					ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "arg sensorState is NULL");
				}
			}
		}
		else
		{
			ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "elements is NULL");
		}
	}
}

void batteryStateChanged(uint8_t percent)
{
	// callback of changing of battery level
	printf("Battery: %d\n", percent);
}

eARCONTROLLER_ERROR decoderConfigCallback(ARCONTROLLER_Stream_Codec_t codec, void *customData)
{
	if (videoOut != NULL)
	{
		if (codec.type == ARCONTROLLER_STREAM_CODEC_TYPE_H264)
		{
			fwrite(codec.parameters.h264parameters.spsBuffer, codec.parameters.h264parameters.spsSize, 1, videoOut);
			fwrite(codec.parameters.h264parameters.ppsBuffer, codec.parameters.h264parameters.ppsSize, 1, videoOut);
			fflush(videoOut);
		}

	}
	else
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "videoOut is NULL.");
	}

	return ARCONTROLLER_OK;
}


eARCONTROLLER_ERROR didReceiveFrameCallback(ARCONTROLLER_Frame_t *frame, void *customData)
{
	if (videoOut != NULL)
	{
		if (frame != NULL)
		{
			fwrite(frame->data, frame->used, 1, videoOut);

			fflush(videoOut);
		}
		else
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "frame is NULL.");
		}
	}
	else
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "videoOut is NULL.");
	}

	return ARCONTROLLER_OK;
}

int customPrintCallback(eARSAL_PRINT_LEVEL level, const char *tag, const char *format, va_list va)
{
	// Custom callback used when ncurses is runing for not disturb the IHM

	if ((level == ARSAL_PRINT_ERROR) && (strcmp(TAG, tag) == 0))
	{
		// Save the last Error
		vsnprintf(gErrorStr, (ERROR_STR_LENGTH - 1), format, va);
		gErrorStr[ERROR_STR_LENGTH - 1] = '\0';
	}

	return 1;
}
