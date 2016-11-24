#ifndef _BEBOP_CONTROLLER_H_
#define _BEBOP_CONTROLLER_H_

extern "C" {
#include "libARController/ARCONTROLLER.h"
}
#include "bebop_video_decoder.h"

eARCONTROLLER_ERROR start_control_bebop2(ARCONTROLLER_Device_t** aDeviceController, bebop_driver::VideoDecoder* aVideoDecoder, ARCONTROLLER_Stream_DidReceiveFrameCallback_t aDidReceiveFrameCallback);

eARCONTROLLER_ERROR finish_control_bebop2(ARCONTROLLER_Device_t* deviceController);

// called when the state of the device controller has changed
void stateChanged(eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData);

// called when a command has been received from the drone
void commandReceived(eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData);

// IHM updates from commands
void batteryStateChanged(uint8_t percent);

eARCONTROLLER_ERROR decoderConfigCallback(ARCONTROLLER_Stream_Codec_t codec, void *customData);

#endif
