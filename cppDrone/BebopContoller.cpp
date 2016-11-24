#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <libavutil/frame.h>

extern "C" {
#include <libARSAL/ARSAL.h>
#include <libARController/ARController.h>
#include <libARDiscovery/ARDiscovery.h>
}

#include "BebopController.h"

using namespace bebop_driver;

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

int frameNb = 0;
ARSAL_Sem_t stateSem;
int isBebop2 = 1;

eARCONTROLLER_ERROR start_control_bebop2(ARCONTROLLER_Device_t** aDeviceController, VideoDecoder* aVideoDecoder, ARCONTROLLER_Stream_DidReceiveFrameCallback_t aDidReceiveFrameCallback)
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
		error = ARCONTROLLER_Device_SetVideoStreamCallbacks(deviceController, decoderConfigCallback, aDidReceiveFrameCallback, NULL, aVideoDecoder);

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

	return error;
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

		if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED) && (elementDictionary != NULL))
		{
			ARCONTROLLER_DICTIONARY_ARG_t *arg = NULL;
			ARCONTROLLER_DICTIONARY_ELEMENT_t *element = NULL;
			HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
			if (element != NULL)
			{
				HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED_FRAMERATE, arg);
				if (arg != NULL)
				{
					auto framerate = eARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED_FRAMERATE(arg->value.I32);

					int res = -1;
					switch (framerate)
					{
					case ARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED_FRAMERATE_24_FPS:
						res = 24;
						break;
					case ARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED_FRAMERATE_25_FPS:
						res = 25;
						break;
					case ARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED_FRAMERATE_30_FPS:
						res = 30;
						break;
					default:
						break;
					}

					ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "video frame rate %d", res);
				}
			}
		}

		if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED) && (elementDictionary != NULL))
		{
			ARCONTROLLER_DICTIONARY_ARG_t *arg = NULL;
			ARCONTROLLER_DICTIONARY_ELEMENT_t *element = NULL;
			HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
			if (element != NULL)
			{
				HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE, arg);
				if (arg != NULL)
				{
					eARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE type = static_cast<eARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE>(arg->value.I32);
					
					int res = -1;
					switch(type)
					{
					case ARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE_REC1080_STREAM480:
						res = 1080;
						break;
					case ARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE_REC720_STREAM720:
						res = 720;
						break;
					default:
						break;
					}

					ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "video resolution %d", res);
				}
			}
		}

		if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED) && (elementDictionary != NULL))
		{
			ARCONTROLLER_DICTIONARY_ARG_t *arg = NULL;
			ARCONTROLLER_DICTIONARY_ELEMENT_t *element = NULL;
			HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
			if (element != NULL)
			{
				HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED_MODE, arg);
				if (arg != NULL)
				{
					auto mode = eARCOMMANDS_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED_MODE(arg->value.I32);

					switch (mode)
					{
					case ARCOMMANDS_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED_MODE_LOW_LATENCY:
						ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "video stream mode LOW_LATENCY");
						break;
					case ARCOMMANDS_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED_MODE_HIGH_RELIABILITY:
						ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "video stream mode HIGH_RELIABILITY");
						break;
					case ARCOMMANDS_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED_MODE_HIGH_RELIABILITY_LOW_FRAMERATE:
						ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "video stream mode HIGH_RELIABILITY_LOW_FRAMERATE");
						break;
					default:
						ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "video stream mode unknown");
						break;
					}
				}
			}
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
	VideoDecoder *decoder = static_cast<VideoDecoder*>(customData);

	if (decoder == NULL)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "VideoDecoder is NULL.");
	}
	else {
		if (codec.type == ARCONTROLLER_STREAM_CODEC_TYPE_H264)
		{
			bool res = decoder->SetH264Params(codec.parameters.h264parameters.spsBuffer, codec.parameters.h264parameters.spsSize, codec.parameters.h264parameters.ppsBuffer, codec.parameters.h264parameters.ppsSize);
			if (!res)
			{
				ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "SetH264Params has failed.");
			}
		}
	}

	return ARCONTROLLER_OK;
}
