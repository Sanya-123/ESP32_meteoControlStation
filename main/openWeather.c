#include "openWeather.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "cJSON/cJSON.h"

#include "esp_request/include/esp_request.h"
#include "net.h"

//#define SIZE_BUFFER_REQUEST     2048

request_t *req;
char requeSendData[300] = {0};
char requeSendDataForcastDayly[300] = {0};
//char requeSendDataForcastTime[300] = {0};
cJSON *root = NULL;
SemaphoreHandle_t xSemaphoreDataIsGet;
//char buff[SIZE_BUFFER_REQUEST] = {0};//NOTE возможно потребуеться больше для предсказания
//uint16_t sizeBuff = 0;

void getWetharData(cJSON * root, OpenWeather *weather)
{//TODO while
    cJSON *_weather = cJSON_GetObjectItem(root, "weather");
    cJSON *_main = cJSON_GetObjectItem(root, "main");
    cJSON *visibility = cJSON_GetObjectItem(root, "visibility");
    cJSON *wind = cJSON_GetObjectItem(root, "wind");
    cJSON *clouds = cJSON_GetObjectItem(root, "clouds");
    cJSON *sys = cJSON_GetObjectItem(root, "sys");

    //weather
//    cJSON *_weather_icon = cJSON_GetObjectItem(_weather, "icon");
    char *weatherIcon = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "icon"));
    if(weatherIcon != NULL)
        memcpy(weather->weatherIcon, weatherIcon, 3);

    //main
    weather->temp = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "temp"));
    weather->feels_like = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "feels_like"));
    weather->temp_min = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "temp_min"));
    weather->temp_max = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "temp_max"));
    weather->pressure = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "pressure"));
    weather->humidity = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "humidity"));
    weather->sea_level = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "sea_level"));
    weather->grnd_level = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "grnd_level"));
    //visibility
    weather->visibility = cJSON_GetNumberValue(visibility);
    //wind
//    cJSON *speed = cJSON_GetObjectItem(wind, "speed");
//    cJSON *deg = cJSON_GetObjectItem(wind, "deg");
    weather->wind_speed = cJSON_GetNumberValue(/*speed*/cJSON_GetObjectItem(wind, "speed"));
    weather->wind_deg = cJSON_GetNumberValue(/*deg*/cJSON_GetObjectItem(wind, "deg"));
    //clouds
    weather->clouds = cJSON_GetNumberValue(cJSON_GetObjectItem(clouds, "all"));
    //sys
    weather->sunrise = cJSON_GetNumberValue(cJSON_GetObjectItem(sys, "sunrise"));
    weather->sunset = cJSON_GetNumberValue(cJSON_GetObjectItem(sys, "sunset"));
}

extern EventGroupHandle_t s_wifi_event_group;

int download_callback(request_t *req, char *data, int len)
{
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
    //len ~=500
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

void initOpenWeather()
{
    xSemaphoreDataIsGet = xSemaphoreCreateBinary();
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
//    ESP_LOGI(TAG, "Connected to AP, freemem=%d",esp_get_free_heap_size());
    // vTaskDelay(1000/portTICK_RATE_MS);
//    req = req_new("http://httpbin.org/post");
    //or

    char City[] = "Tomsk";
    char apikey[] = "b5699c83d361c7f2d4b0d192a2534554";
    sprintf(requeSendData, "http://api.openweathermap.org/data/2.5/%s?q=%s,%s&APPID=%s&mode=json&units=%s&lang=%s", "weather", City, "RU", apikey, "M", "EN");
    sprintf(requeSendDataForcastDayly, "http://api.openweathermap.org/data/2.5/%s?q=%s,%s&APPID=%s&mode=json&units=%s&lang=%s&cnt=%d", "forecast", City, "RU", apikey, "M", "EN", 10);
//    sprintf(requeSendDataForcastTime, "http://pro.openweathermap.org/data/2.5/%s/hourly?q=%s,%s&APPID=%s&mode=json&units=%s&lang=%s&cnt=%d", "forecast", City, "RU", apikey, "M", "EN", 24);//мне он не доступен требуеться другой аккаунт

    req = req_new("https://api.openweathermap.org"); //for SSL
    req_setopt(req, REQ_SET_METHOD, "GET");

    ESP_LOGI("HTTP", "reque:%s", requeSendData);
    req_setopt(req, REQ_SET_URI, requeSendData);
    req_setopt(req, REQ_FUNC_DOWNLOAD_CB, download_callback);
}

void JSON_Parse(cJSON * root, int level) {
    //ESP_LOGI(TAG, "root->type=%s", JSON_Types(root->type));
    cJSON *current_element = NULL;
    //ESP_LOGI(TAG, "roo->child=%p", root->child);
    //ESP_LOGI(TAG, "roo->next =%p", root->next);

    char level_offst[128] = {};
    for(int i = 0; i < level; i++)
        level_offst[i] = '\t';
    cJSON_ArrayForEach(current_element, root) {
        //ESP_LOGI(TAG, "type=%s", JSON_Types(current_element->type));
        //ESP_LOGI(TAG, "current_element->string=%p", current_element->string);
        if (current_element->string) {
            const char* string = current_element->string;
            ESP_LOGI("cJSON:", "%s[%s]", level_offst, string);
        }
        if (cJSON_IsInvalid(current_element)) {
            ESP_LOGI("cJSON:", "%s\tInvalid", level_offst);
        } else if (cJSON_IsFalse(current_element)) {
            ESP_LOGI("cJSON:", "%s\tFalse", level_offst);
        } else if (cJSON_IsTrue(current_element)) {
            ESP_LOGI("cJSON:", "%s\tTrue", level_offst);
        } else if (cJSON_IsNull(current_element)) {
            ESP_LOGI("cJSON:", "%s\tNull", level_offst);
        } else if (cJSON_IsNumber(current_element)) {
            int valueint = current_element->valueint;
            double valuedouble = current_element->valuedouble;
            ESP_LOGI("cJSON:", "%s\tint=%d double=%f", level_offst, valueint, valuedouble);
        } else if (cJSON_IsString(current_element)) {
            const char* valuestring = current_element->valuestring;
            ESP_LOGI("cJSON:", "%s\t%s", level_offst, valuestring);
        } else if (cJSON_IsArray(current_element)) {
            ESP_LOGI("cJSON:", "%sArray", level_offst);
            JSON_Parse(current_element, level + 1);
            ESP_LOGI("cJSON:", "%sEND Array", level_offst);
        } else if (cJSON_IsObject(current_element)) {
            ESP_LOGI("cJSON:", "%sObject", level_offst);
            JSON_Parse(current_element, level + 1);
            ESP_LOGI("cJSON:", "%sEND Object", level_offst);
        } else if (cJSON_IsRaw(current_element)) {
            ESP_LOGI("cJSON:", "%s\tRaw(Not support)", level_offst);
        }
    }
}

int askWeather(OpenWeather *veather)
{
    int status = req_perform(req);
    req_setopt(req, REQ_SET_URI, requeSendData);
    ESP_LOGI("HTTP:", "ststus %d", status);
    if( xSemaphoreTake( xSemaphoreDataIsGet, portMAX_DELAY ) == pdTRUE )
    {
        getWetharData(root, veather);
//        JSON_Parse(root, 0);
        ESP_LOGI("weathe:", "ok res weather");
        cJSON_Delete(root);
        return 0;
    }
    return -1;
}

int askWeatherDayly(OpenWeather *veather)
{
    int status = req_perform(req);
    req_setopt(req, REQ_SET_URI, requeSendDataForcastDayly);
    ESP_LOGI("HTTP:", "ststus %d", status);
    if( xSemaphoreTake( xSemaphoreDataIsGet, portMAX_DELAY ) == pdTRUE )
    {
        getWetharData(root, veather);
//        JSON_Parse(root, 0);
        ESP_LOGI("weathe:", "ok res weather");
        cJSON_Delete(root);
        return 0;
    }
    return -1;
}

void printOpenWeather(OpenWeather weather)
{
    //weather
    ESP_LOGI("weathe:", "weatherIcon %s", weather.weatherIcon);
    //main
    ESP_LOGI("weathe:", "temp %f", weather.temp);
    ESP_LOGI("weathe:", "feels_like %f", weather.feels_like);
    ESP_LOGI("weathe:", "temp_min %f", weather.temp_min);
    ESP_LOGI("weathe:", "temp_max %f", weather.temp_max);
    ESP_LOGI("weathe:", "pressure %f", weather.pressure);
    ESP_LOGI("weathe:", "humidity %f", weather.humidity);
    ESP_LOGI("weathe:", "sea_level %f", weather.sea_level);
    ESP_LOGI("weathe:", "grnd_level %f", weather.grnd_level);
    //visibility
    ESP_LOGI("weathe:", "visibility %d", weather.visibility);
    //wind
    ESP_LOGI("weathe:", "wind_speed %d", weather.wind_speed);
    ESP_LOGI("weathe:", "wind_deg %d", weather.wind_deg);
    //clouds
    ESP_LOGI("weathe:", "clouds %d", weather.clouds);
    //sys
    ESP_LOGI("weathe:", "sunrise %d", weather.sunrise);
    ESP_LOGI("weathe:", "sunset %d", weather.sunset);
}
