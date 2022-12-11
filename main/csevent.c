#include "csevent.h"
#include "esp_log.h"


EventGroupHandle_t cs_event_group = NULL;

void initGroupCSevent()
{
    cs_event_group = xEventGroupCreate();
    if(cs_event_group == NULL)
        ESP_LOGE("eventCS", "Couldn't init event group");
    else
        ESP_LOGI("eventCS", "OK init event group");
}

bool waiEventyCS(EventBits_t eventBit, TickType_t xTicksToWait)
{
    if(cs_event_group == NULL)  return false;
    return xEventGroupWaitBits(cs_event_group, eventBit, false, true, xTicksToWait) != 0;
}

void sendEvent(EventBits_t eventBit)
{
    if(cs_event_group != NULL)
        xEventGroupSetBits(cs_event_group, eventBit);
}
