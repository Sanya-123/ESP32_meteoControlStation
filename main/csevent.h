#ifndef CSEVENT_H
#define CSEVENT_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define EVENT_DISPLAY_INIT_DONE         BIT0
#define EVENT_LOCATION_GET              BIT1
#define EVENT_WIFI_CONNECT_STA          BIT2

/* FreeRTOS event group to signal of some event */
void initGroupCSevent();
bool waiEventyCS(EventBits_t eventBit, TickType_t xTicksToWait);
void sendEvent(EventBits_t eventBit);

#endif // CSEVENT_H
