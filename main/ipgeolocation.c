#include "ipgeolocation.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "cJSON.h"

#include "esp_request/include/esp_request.h"
#include "net.h"

#define URL_GET_LOCATION            "https://ipwho.is/"
//use service https://ipwhois.io/


static request_t *req;
//static char requeSendData[300] = {0};
static cJSON *root = NULL;
static SemaphoreHandle_t xSemaphoreDataIsGet = NULL;

static int download_callback(request_t *req, char *data, int len)
{
    (void)len;
    req_list_t *found = req->response->header;
    while(found->next != NULL) {
        found = found->next;
        ESP_LOGI("HTTP","Response header %s:%s", (char*)found->key, (char*)found->value);
    }
    //or
    found = req_list_get_key(req->response->header, "Content-Length");
    if(found) {
        ESP_LOGI("HTTP","Get header %s:%s", (char*)found->key, (char*)found->value);
    }

    //BUG data maybe some header
    for(int i = 0; i < len-1; i++)//tru to found begin JSON
    {
        if(data[0] == '{')
            break;
        else
            data++;
    }
    ESP_LOGI("HTTP","*DATA:%s", data);
    root = cJSON_Parse(data);
    ESP_LOGI("HTTP","OK cJSON_Parse");
    xSemaphoreGive( xSemaphoreDataIsGet );
    //TODO еселать это и семафоры если за раз не будет приходиться вся посылка а это маловероятно
//    if((sizeBuff + len) < SIZE_BUFFER_REQUEST)
//    {
//        memcpy(&buff[sizeBuff], data, len);
//        sizeBuff += len;
//    }
//    else
//    {
//        memcpy(&buff[sizeBuff], data, SIZE_BUFFER_REQUEST - 1 - sizeBuff);
//        sizeBuff = SIZE_BUFFER_REQUEST - 1;
//    }

//    buff[sizeBuff] = '\0';
    return 0;
}

void parsetLocation(cJSON * root, IpLocation *location)
{
    cJSON *_country_code = cJSON_GetObjectItem(root, "country_code");
    cJSON *_city = cJSON_GetObjectItem(root, "city");
    cJSON *_lat = cJSON_GetObjectItem(root, "latitude");
    cJSON *_lon = cJSON_GetObjectItem(root, "longitude");
    cJSON *_time = cJSON_GetObjectItem(cJSON_GetObjectItem(root, "timezone"), "current_time");

    char *valString;

    valString = cJSON_GetStringValue(cJSON_GetObjectItem(root, "ip"));
    if(valString)
        ESP_LOGI("location", "IP:%s", valString);

    valString = cJSON_GetStringValue(_country_code);
    if(valString)
        memcpy(location->country_code, valString, 2);

    valString = cJSON_GetStringValue(_city);
    if(valString)
        memcpy(location->city, valString, strlen(valString));

    location->lat = cJSON_GetNumberValue(_lat);
    location->lon = cJSON_GetNumberValue(_lon);

    //time
    valString = cJSON_GetStringValue(_time);
    if(valString)
    {
        //try to get data
//        valString
        location->year = atoi(&valString[0]);
        location->month = atoi(&valString[5]);
        location->day = atoi(&valString[8]);

        location->hour = atoi(&valString[11]);
        location->minutes = atoi(&valString[14]);
    }
}

int getLocation(IpLocation *location)
{
    if(xSemaphoreDataIsGet == NULL)
        xSemaphoreDataIsGet = xSemaphoreCreateBinary();

    //init
    req = req_new(URL_GET_LOCATION); //for SSL
    req_setopt(req, REQ_SET_METHOD, "GET");

    req_setopt(req, REQ_SET_URI, URL_GET_LOCATION);
    req_setopt(req, REQ_FUNC_DOWNLOAD_CB, download_callback);

    int status = req_perform(req);
    ESP_LOGI("HTTP", "ststus %d", status);

    if( xSemaphoreTake( xSemaphoreDataIsGet, portMAX_DELAY ) == pdTRUE )
    {
        memset(location, 0, sizeof(IpLocation));//clean data
        parsetLocation(root, location);
        ESP_LOGI("location", "ok res location");
        ESP_LOGI("location", "%c%c:%s(%f,%f)", location->country_code[0], location->country_code[1],\
                    location->city, location->lat, location->lon);

        ESP_LOGI("location", "date:%02d.%02d.%04d", location->day, location->month, location->year);
        ESP_LOGI("location", "time:%02d:%02d", location->hour, location->minutes);

        cJSON_Delete(root);
        req_clean(req);
        return 0;
    }



    req_clean(req);
    return -1;
}
