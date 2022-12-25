#include "flash.h"
#include "nvs.h"
#include "nvs_sync.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include <stdio.h> //sprintf


const char controlStation_nvs_namespace[] = "controlstation";

bool initFlash()
{
    nvs_flash_init();
    return nvs_sync_create() == ESP_OK;
}

void saveGuiSittings(SettignDisplay settings)
{
    if(nvs_sync_lock( portMAX_DELAY ))
    {
        nvs_handle handle;
        esp_err_t esp_err;
        size_t sz;

        esp_err = nvs_open(controlStation_nvs_namespace, NVS_READWRITE, &handle);
        if (esp_err != ESP_OK){
            nvs_sync_unlock();
            ESP_LOGE("flash", "Couldn't open flash");
            return;
        }

        /*******************************GET autochange display*******************************/
        sz = sizeof(settings.autoChangeDisplay);
        esp_err = nvs_set_blob(handle, "autochange", &settings.autoChangeDisplay, sz);
        if(esp_err != ESP_OK)
        {
            ESP_LOGW("flash", "Couldn't save autochange");
        }

        /*******************************GET use mmHg*******************************/
        sz = sizeof(settings.usemmHg);
        esp_err = nvs_set_blob(handle, "usemmhg", &settings.usemmHg, sz);
        if(esp_err != ESP_OK)
        {
            ESP_LOGW("flash", "Couldn't save usemmhg");
        }

        /*******************************GET dark theme*******************************/
        sz = sizeof(settings.darkTheme);
        esp_err = nvs_set_blob(handle, "darktheme", &settings.darkTheme, sz);
        if(esp_err != ESP_OK)
        {
            ESP_LOGW("flash", "Couldn't save darktheme");
        }

        /*******************************GET dark theme*******************************/
        sz = sizeof(settings.style);
        esp_err = nvs_set_blob(handle, "style", &settings.style, sz);
        if(esp_err != ESP_OK)
        {
            ESP_LOGW("flash", "Couldn't save style");
        }

        /*******************************GET time on display*******************************/
        for(int i = 0; i < DISPLAY_SIZE; i++)
        {
            char settingTimeOnDisplayFlashName[32] = {0};
            sprintf(settingTimeOnDisplayFlashName, "timedispay%d", i);
            sz = sizeof(settings.timeOnDisplay[i]);
            esp_err = nvs_set_blob(handle, settingTimeOnDisplayFlashName, &settings.timeOnDisplay[i], sz);
            if(esp_err != ESP_OK)
            {
                ESP_LOGW("flash", "Couldn't save %s", settingTimeOnDisplayFlashName);
            }
        }

        ESP_LOGI("flash", "End save paramter");
        nvs_sync_unlock();
        return;
    }

    ESP_LOGE("flash", "Couldn't looh nvs sync");

    return;
}

bool getGuiSittings(SettignDisplay *settings)
{
    if(nvs_sync_lock( portMAX_DELAY ))
    {
        nvs_handle handle;
        esp_err_t esp_err;
        size_t sz;

        esp_err = nvs_open(controlStation_nvs_namespace, NVS_READWRITE, &handle);
        if (esp_err != ESP_OK){
            nvs_sync_unlock();
            ESP_LOGE("flash", "Couldn't open flash");
            return false;
        }

        /*******************************GET autochange display*******************************/
        sz = sizeof(settings->autoChangeDisplay);
        esp_err = nvs_get_blob(handle, "autochange", &settings->autoChangeDisplay, &sz);
        if((esp_err != ESP_OK) || (sz == 0))
        {
            ESP_LOGW("flash", "Couldn't get autochange");
            settings->autoChangeDisplay = DEFOULT_AUTO_CHANGE_DISPAY;
        }
        else
            ESP_LOGI("flash", "autochange is %d", settings->autoChangeDisplay);

        /*******************************GET use mmHg*******************************/
        sz = sizeof(settings->usemmHg);
        esp_err = nvs_get_blob(handle, "usemmhg", &settings->usemmHg, &sz);
        if((esp_err != ESP_OK) || (sz == 0))
        {
            ESP_LOGW("flash", "Couldn't get usemmhg");
            settings->usemmHg = DEFOULT_USE_MMHG;
        }
        else
            ESP_LOGI("flash", "usemmhg is %d", settings->usemmHg);

        /*******************************GET dark theme*******************************/
        sz = sizeof(settings->darkTheme);
        esp_err = nvs_get_blob(handle, "darktheme", &settings->darkTheme, &sz);
        if((esp_err != ESP_OK) || (sz == 0))
        {
            ESP_LOGW("flash", "Couldn't get darktheme");
            settings->darkTheme = DEFOULT_DARK_THEME;
        }
        else
            ESP_LOGI("flash", "darktheme is %d", settings->darkTheme);

        /*******************************GET dark theme*******************************/
        sz = sizeof(settings->style);
        esp_err = nvs_get_blob(handle, "style", &settings->style, &sz);
        if((esp_err != ESP_OK) || (sz == 0))
        {
            ESP_LOGW("flash", "Couldn't get style");
            settings->style = DEFOULT_STYLE;
        }
        else
            ESP_LOGI("flash", "style is %d", settings->style);

        /*******************************GET time on display*******************************/
        for(int i = 0; i < DISPLAY_SIZE; i++)
        {
            char settingTimeOnDisplayFlashName[32] = {0};
            sprintf(settingTimeOnDisplayFlashName, "timedispay%d", i);
            sz = sizeof(settings->timeOnDisplay[i]);
            esp_err = nvs_get_blob(handle, settingTimeOnDisplayFlashName, &settings->timeOnDisplay[i], &sz);
            if((esp_err != ESP_OK) || (sz == 0))
            {
                ESP_LOGW("flash", "Couldn't get %s", settingTimeOnDisplayFlashName);
                settings->timeOnDisplay[i] = DEFOULT_TIME_ON_DISPLAY;
            }
            else
                ESP_LOGI("flash", "%s is %d", settingTimeOnDisplayFlashName, settings->timeOnDisplay[i]);
        }

        ESP_LOGI("flash", "End get paramter");
        nvs_sync_unlock();
        return true;
    }

    ESP_LOGE("flash", "Couldn't looh nvs sync");

    return false;
}

bool saveExtDeviceSize(uint8_t connections)
{
    if(nvs_sync_lock( portMAX_DELAY ))
    {
        nvs_handle handle;
        esp_err_t esp_err;

        esp_err = nvs_open(controlStation_nvs_namespace, NVS_READWRITE, &handle);
        if (esp_err != ESP_OK){
            nvs_sync_unlock();
            ESP_LOGE("flash", "Couldn't open flash");
            return false;
        }

        esp_err = nvs_set_u8(handle, "extconnections", connections);
        if(esp_err != ESP_OK)
        {
            ESP_LOGW("flash", "Couldn't set extconnections");
        }

        nvs_sync_unlock();
        return false;
    }

    return false;
}

uint8_t getExtDeviceSize()
{
    if(nvs_sync_lock( portMAX_DELAY ))
    {
        nvs_handle handle;
        esp_err_t esp_err;

        esp_err = nvs_open(controlStation_nvs_namespace, NVS_READWRITE, &handle);
        if (esp_err != ESP_OK){
            nvs_sync_unlock();
            ESP_LOGE("flash", "Couldn't open flash");
            return 0;
        }

        uint8_t data = 0;
        esp_err = nvs_get_u8(handle, "extconnections", &data);
        if(esp_err != ESP_OK)
        {
            ESP_LOGW("flash", "Couldn't get extconnections");
            data = 0;
            nvs_set_u8(handle, "extconnections", data);
        }
        else
            ESP_LOGI("flash", "extconnections is %d", data);

        nvs_sync_unlock();
        return data;
    }
    return 0;
}
