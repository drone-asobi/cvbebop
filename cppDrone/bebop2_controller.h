#ifndef _BEBOP2_CONTROLLER_H_
#define _BEBOP2_CONTROLLER_H_

extern "C" {
#include "libARController/ARCONTROLLER.h"
}

void command_received_callback(eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData);

#endif
