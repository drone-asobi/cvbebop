#ifndef _BEBOP2_DEVICE_H_
#define _BEBOP2_DEVICE_H_

extern "C" {
#include "libARController/ARCONTROLLER.h"
}
#include "bebop_video_decoder.h"

#define BEBOP_IP_ADDRESS "192.168.42.1"
#define BEBOP_DISCOVERY_PORT 44444

eARCONTROLLER_ERROR start_bebop2(ARCONTROLLER_Device_t** aDeviceController, ARCONTROLLER_DICTIONARY_CALLBACK_t aCommandCallback, bebop_driver::VideoDecoder* aVideoDecoder, ARCONTROLLER_Stream_DidReceiveFrameCallback_t aDidReceiveFrameCallback);
eARCONTROLLER_ERROR finish_bebop2(ARCONTROLLER_Device_t* deviceController);
void keyboard_controller_loop(ARCONTROLLER_Device_t *deviceController, const char *cvWindowName);

static void state_changed_callback(eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData);

#endif
