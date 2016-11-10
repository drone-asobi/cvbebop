#ifndef _BEBOP_CONTROLLER_H_
#define _BEBOP_CONTROLLER_H_

#include "libARController/ARCONTROLLER.h"

ARCONTROLLER_Device_t* start_control_bebop2(eARCONTROLLER_ERROR &error);

eARCONTROLLER_ERROR finish_control_bebop2(ARCONTROLLER_Device_t* deviceController);

// called when the state of the device controller has changed
void stateChanged(eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData);

// called when a command has been received from the drone
void commandReceived(eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData);

// IHM updates from commands
void batteryStateChanged(uint8_t percent);

// called when a streaming frame has been received
eARCONTROLLER_ERROR didReceiveFrameCallback(ARCONTROLLER_Frame_t *frame, void *customData);

eARCONTROLLER_ERROR decoderConfigCallback(ARCONTROLLER_Stream_Codec_t codec, void *customData);

int customPrintCallback(eARSAL_PRINT_LEVEL level, const char *tag, const char *format, va_list va);

#endif
