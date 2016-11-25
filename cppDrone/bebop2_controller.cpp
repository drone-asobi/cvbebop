#define TAG "bebop2_controller"

extern "C" {
#include <libARSAL/ARSAL.h>
#include <libARController/ARController.h>
}

#include "bebop2_controller.h"

// called when a command has been received from the drone
void command_received_callback(eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData)
{
	auto *deviceController = static_cast<ARCONTROLLER_Device_t*>(customData);

	if (deviceController == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, " DeviceController is NULL!");
		return;
	}

	// if the command received is a battery state changed
	if (commandKey == ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED)
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *singleElement = nullptr;

		if (elementDictionary != nullptr)
		{
			// get the command received in the device controller
			HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, singleElement);

			if (singleElement != nullptr)
			{
				// get the value
				HASH_FIND_STR(singleElement->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED_PERCENT, arg);

				if (arg != nullptr)
				{
					// update UI
					printf("Battery: %d\n", arg->value.U8);
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
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;

		if (elementDictionary != nullptr)
		{
			ARCONTROLLER_DICTIONARY_ELEMENT_t *dictElement = nullptr;
			ARCONTROLLER_DICTIONARY_ELEMENT_t *dictTmp = nullptr;

			eARCOMMANDS_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORNAME sensorName = ARCOMMANDS_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORNAME_MAX;
			int sensorState = 0;

			HASH_ITER(hh, elementDictionary, dictElement, dictTmp)
			{
				// get the Name
				HASH_FIND_STR(dictElement->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORNAME, arg);
				if (arg != nullptr)
				{
					sensorName = static_cast<eARCOMMANDS_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORNAME>(arg->value.I32);
				}
				else
				{
					ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "arg sensorName is NULL");
				}

				// get the state
				HASH_FIND_STR(dictElement->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORSTATE, arg);
				if (arg != nullptr)
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

	if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED) && (elementDictionary != nullptr))
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *element = nullptr;
		HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
		if (element != nullptr)
		{
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED_FRAMERATE, arg);
			if (arg != nullptr)
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

	if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED) && (elementDictionary != nullptr))
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *element = nullptr;
		HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
		if (element != nullptr)
		{
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE, arg);
			if (arg != nullptr)
			{
				eARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE type = static_cast<eARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE>(arg->value.I32);

				int res = -1;
				switch (type)
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

	if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED) && (elementDictionary != nullptr))
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *element = nullptr;
		HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
		if (element != nullptr)
		{
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED_MODE, arg);
			if (arg != nullptr)
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