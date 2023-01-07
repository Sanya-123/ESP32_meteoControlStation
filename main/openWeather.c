#include "openWeather.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "cJSON.h"

#include "esp_request/include/esp_request.h"
#include "net.h"

//#define SIZE_BUFFER_REQUEST     2048

static request_t *req;
char requeSendData[300] = {0};
char requeSendDataForcastDayly[300] = {0};
char requeSendDataForcastHourly[300] = {0};
char requeSendDataOneCall[300] = {0};
//char requeSendDataForcastTime[300] = {0};
static cJSON *root = NULL;
static SemaphoreHandle_t xSemaphoreDataIsGet;
//char buff[SIZE_BUFFER_REQUEST] = {0};//NOTE возможно потребуеться больше для предсказания
//uint16_t sizeBuff = 0;
char city[128] = {0};
char countryCode[3] = {0};
static float lat = 0, lon = 0;
uint8_t oneCall_step = 5, oneCall_size = 4, oneCall_offset = 4 + 5;

int cutDataOpenWeatherH(char *data, int sizeData, uint8_t step, uint8_t size, uint8_t offset)
{
    char *pointCut;
    int curentSize = sizeData;

    char *pointBegin = strstr(data, "\"hourly\":");

    pointCut = strstr(pointBegin, "{\"dt\":");

    for(int i = 0; i < offset; i++)//+
    {//remove ferst
        char *pointNext = strstr(pointCut + 1, "{\"dt\":");
        int ledtSize = curentSize - (pointNext - data);
        memcpy(pointCut, pointNext, ledtSize);
        curentSize -= pointNext - pointCut;
    }

//    qDebug() << "here";

    for(int i = 0; i < size; i++)
    {//left just some
        pointCut = strstr(pointCut + 1, "{\"dt\":");
        char *pointNext = pointCut;
        for(int j = 0; j < (step - 1); j++)
            pointNext = strstr(pointNext + 1, "{\"dt\":");//remove steps data

        int ledtSize = curentSize - (pointNext - data);
        memcpy(pointCut, pointNext, ledtSize);
        curentSize -= pointNext - pointCut;
    }

    //remove last
    pointCut = strstr(pointBegin, "{\"dt\":");
    for(int i = 0; i < size; i++)
    {//left just some
        pointCut = strstr(pointCut + 1, ",{\"dt\":");
    }
    char *pointNext = strstr(pointCut + 1, "],\"daily\":");
    int ledtSize = curentSize - (pointNext - data);
    memcpy(pointCut, pointNext, ledtSize);
    curentSize -= pointNext - pointCut;

    //cut days maybe

    return curentSize;
}

static int download_callback(request_t *req, char *data, int len)
{
    (void)len;
    static char *downloadData = NULL;
    static int totalRecive = 0;
    static int totalWaytData = 0;

    req_list_t *found = req->response->header;
    while(found->next != NULL) {
        found = found->next;
        ESP_LOGI("HTTP","Response header %s:%s", (char*)found->key, (char*)found->value);
    }
    //or
    found = req_list_get_key(req->response->header, "Content-Length");
    if(found)
    {
        ESP_LOGI("HTTP","Get header %s:%s", (char*)found->key, (char*)found->value);
        if(totalWaytData == 0)
        {
            totalWaytData = atoi((char*)found->value);
            if(len < totalWaytData)//if rx data in several transaction
            {
                ESP_LOGI("HTTP", "Wayt anaouther paket %d", totalWaytData);
                if(downloadData != NULL)//check if maloc
                    free(downloadData);

//                downloadData = malloc(totalWaytData + 16);
                downloadData = heap_caps_malloc(totalWaytData + 16, MALLOC_CAP_IRAM_8BIT);
                if(downloadData == NULL)
                {
                    ESP_LOGE("HTTP", "Error maloc");
                }
                totalRecive = 0;
            }
        }
    }
    else
    {
        totalRecive = 0;
        totalWaytData = 0;
        if(downloadData)
            free(downloadData);
        downloadData = NULL;
    }
    //len ~=500
    ESP_LOGI("HTTP","LEN:%d", len);
    ESP_LOGI("HTTP","*DATA:%s", data);
    if(downloadData == NULL)//if wayt onli 1 packet
    {
        root = cJSON_Parse(data);
        totalRecive = 0;
        totalWaytData = 0;
        ESP_LOGI("HTTP","OK cJSON_Parse");

        xSemaphoreGive( xSemaphoreDataIsGet );
    }
    else
    {
        memcpy(downloadData + totalRecive, data, len);

        totalRecive += len;

        if(totalWaytData <= totalRecive)
        {
            ESP_LOGI("HTTP","Resive all");

            int analizeSize = totalWaytData;
            if(totalWaytData >= 15000)//if it is one call remove some data
            {
                analizeSize = cutDataOpenWeatherH(downloadData, totalWaytData, oneCall_step, oneCall_size, oneCall_offset);
            }
            downloadData[analizeSize] = '\0';
            root = cJSON_ParseWithLength(downloadData, analizeSize);
//            ESP_LOGI("JSON","data:%s", downloadData);

            if(root == NULL)
            {
                ESP_LOGE("JSON","error parset data %d;;;", strlen(cJSON_GetErrorPtr()));
            }
//            ESP_LOGI("JSON","parse:%s", cJSON_Print(root));
            totalRecive = 0;
            totalWaytData = 0;
            free(downloadData);
            downloadData = NULL;

//            ESP_LOGI("Memory", "Free heap size %d", esp_get_free_heap_size());
//            ESP_LOGI("Memory", "Free internal heap size %d", esp_get_free_internal_heap_size());
//            ESP_LOGI("Memory", "Free minimum heap size %d", esp_get_minimum_free_heap_size());

            ESP_LOGI("HTTP","OK cJSON_Parse");

            xSemaphoreGive( xSemaphoreDataIsGet );
        }
    }

    return 0;
}

void getWetharData(cJSON * root, OpenWeather *weather)
{
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

    char *weatherMain = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "main"));
    if(weatherMain != NULL)
        memcpy(weather->weatherName, weatherMain, strlen(weatherMain));

    weather->code = cJSON_GetNumberValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "id"));

    //main
    weather->temp = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "temp")) - 273;
    weather->feels_like = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "feels_like")) - 273;
    weather->temp_min = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "temp_min")) - 273;
    weather->temp_max = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "temp_max")) - 273;
    weather->pressure = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "pressure"));
    weather->humidity = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "humidity"));
//    weather->sea_level = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "sea_level"));
//    weather->grnd_level = cJSON_GetNumberValue(cJSON_GetObjectItem(_main, "grnd_level"));
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

void getWetharOneCallData(cJSON * root, OpenWeather *current, OpenWeather *dayly, int days, OpenWeather *hourlu, int houres)
{
    cJSON *_curent = cJSON_GetObjectItem(root, "current");
    cJSON *_hourly = cJSON_GetObjectItem(root, "hourly");
    cJSON *_daily = cJSON_GetObjectItem(root, "daily");
    cJSON *_weather = cJSON_GetObjectItem(_curent, "weather");
//    cJSON *wind = cJSON_GetObjectItem(root, "wind");
//    cJSON *clouds = cJSON_GetObjectItem(root, "clouds");
//    cJSON *sys = cJSON_GetObjectItem(root, "sys");

//    ESP_LOGI("JSON","parse:%s", cJSON_Print(_daily));

    //weather
//    cJSON *_weather_icon = cJSON_GetObjectItem(_weather, "icon");
    char *weatherIcon = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "icon"));
    if(weatherIcon != NULL)
        memcpy(current->weatherIcon, weatherIcon, 3);

    char *weatherMain = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "description"));
    if(weatherMain != NULL)
        memcpy(current->weatherName, weatherMain, strlen(weatherMain));

    current->code = cJSON_GetNumberValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "id"));

    //main
    current->temp = cJSON_GetNumberValue(cJSON_GetObjectItem(_curent, "temp")) - 273;
    current->feels_like = cJSON_GetNumberValue(cJSON_GetObjectItem(_curent, "feels_like")) - 273;
    current->pressure = cJSON_GetNumberValue(cJSON_GetObjectItem(_curent, "pressure"));
    current->humidity = cJSON_GetNumberValue(cJSON_GetObjectItem(_curent, "humidity"));
    current->visibility = cJSON_GetNumberValue(cJSON_GetObjectItem(_curent, "visibility"));
    current->wind_speed = cJSON_GetNumberValue(cJSON_GetObjectItem(_curent, "wind_speed"));
    current->wind_deg = cJSON_GetNumberValue(cJSON_GetObjectItem(_curent, "wind_deg"));
    current->clouds = cJSON_GetNumberValue(cJSON_GetObjectItem(_curent, "clouds"));
    current->sunrise = cJSON_GetNumberValue(cJSON_GetObjectItem(_curent, "sunrise"));
    current->sunset = cJSON_GetNumberValue(cJSON_GetObjectItem(_curent, "sunset"));

    cJSON *day_hour = cJSON_GetObjectItem(_daily, "0");
    current->phaseMoon = cJSON_GetNumberValue(cJSON_GetObjectItem(day_hour, "moon_phase"));

    char iter[10] = {0};
    cJSON *_temp;

    for(int i = 0; i < days; i++)
    {
        sprintf(iter, "%d", i);
//        day_hour = cJSON_GetObjectItem(_daily, iter);
        day_hour = cJSON_GetArrayItem(_daily, i);
        _temp = cJSON_GetObjectItem(day_hour, "temp");
        _weather = cJSON_GetObjectItem(day_hour, "weather");
        dayly[i].temp_min = cJSON_GetNumberValue(cJSON_GetObjectItem(_temp, "min")) - 273;
        dayly[i].temp_max = cJSON_GetNumberValue(cJSON_GetObjectItem(_temp, "max")) - 273;
        dayly[i].temp = cJSON_GetNumberValue(cJSON_GetObjectItem(_temp, "eve")) - 273;

        char *weatherIcon = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "icon"));
        if(weatherIcon != NULL)
            memcpy(dayly[i].weatherIcon, weatherIcon, 3);

        char *weatherMain = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "description"));
        if(weatherMain != NULL)
            memcpy(dayly[i].weatherName, weatherMain, strlen(weatherMain));

        dayly[i].code = cJSON_GetNumberValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "id"));

        //NOTE параметров много возможно чтот еще нужно будет
    }

    for(int i = 0; i < houres; i++)
    {
        sprintf(iter, "%d", i);
//        day_hour = cJSON_GetObjectItem(_hourly, iter);
        day_hour = cJSON_GetArrayItem(_hourly, i);
//        _temp = cJSON_GetObjectItem(day_hour, "temp");
        _weather = cJSON_GetObjectItem(day_hour, "weather");
        hourlu[i].temp = cJSON_GetNumberValue(cJSON_GetObjectItem(day_hour, "temp")) - 273;

        char *weatherIcon = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "icon"));
        if(weatherIcon != NULL)
            memcpy(hourlu[i].weatherIcon, weatherIcon, 3);

        char *weatherMain = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "description"));
        if(weatherMain != NULL)
            memcpy(hourlu[i].weatherName, weatherMain, strlen(weatherMain));

        hourlu[i].code = cJSON_GetNumberValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "id"));

        //NOTE параметров много возможно чтот еще нужно будет
    }
}

void setLocation(char *_city, char *_countryCode, float _lat, float _lon)
{
    lat = _lat;
    lon = _lon;

    strcpy(city, _city);
    countryCode[0] = _countryCode[0];
    countryCode[1] = _countryCode[1];
}

int initOpenWeather()
{
    xSemaphoreDataIsGet = xSemaphoreCreateBinary();
    // vTaskDelay(1000/portTICK_RATE_MS);
//    req = req_new("http://httpbin.org/post");
    //or

    if(strlen(city) == 0)
        return -1;

    if(strlen(countryCode) == 0)
        return -1;

//    char City[] = "Tomsk";
    char apikey[] = CONFIG_OPEN_WEATHER_API_KEY;
//    float lat = 58.5, lon = 82.5;
    sprintf(requeSendData, "http://api.openweathermap.org/data/2.5/%s?q=%s,%s&APPID=%s&mode=json&units=%s&lang=%s", "weather", city, countryCode, apikey, "M", "EN");
//    sprintf(requeSendDataForcastDayly, "http://api.openweathermap.org/data/2.5/%s?lat=%f&lon=%f&APPID=%s&mode=json&units=%s&lang=%s&cnt=%d", "forecast", lat, lon, apikey, "M", "EN", 10);
    sprintf(requeSendDataOneCall, "http://api.openweathermap.org/data/2.5/%s?lat=%f&lon=%f&APPID=%s&units=%s&lang=%s&exclude=minutely,alerts", "onecall", /*City, "RU", */lat, lon, apikey, "M", "EN");
    sprintf(requeSendDataForcastDayly, "http://api.openweathermap.org/data/2.5/%s?lat=%f&lon=%f&APPID=%s&units=%s&lang=%s&exclude=minutely,alerts,daily,current", "onecall", /*City, "RU", */lat, lon, apikey, "M", "EN");
    //one call can get: current minutely, hourly, daily, alerts(usefull info but not only English
    //exclude minutely and alerts

    req = req_new("https://api.openweathermap.org"); //for SSL
    req_setopt(req, REQ_SET_METHOD, "GET");

    ESP_LOGI("HTTP", "reque:%s", requeSendData);
    ESP_LOGI("HTTP", "forcast:%s", requeSendDataForcastDayly);
    ESP_LOGI("HTTP", "onecall:%s", requeSendDataOneCall);
    req_setopt(req, REQ_SET_URI, requeSendData);
    req_setopt(req, REQ_FUNC_DOWNLOAD_CB, download_callback);

    return 0;
}

int askWeather(OpenWeather *veather)
{
    req_setopt(req, REQ_SET_URI, requeSendData);
    int status = req_perform(req);
    ESP_LOGI("HTTP", "ststus %d", status);
    if( xSemaphoreTake( xSemaphoreDataIsGet, portMAX_DELAY ) == pdTRUE )
    {
        if(root == NULL)
            return -1;
        getWetharData(root, veather);
        ESP_LOGI("weathe", "ok res weather");
        cJSON_Delete(root);
        return 0;
    }
    return -1;
}

int askWeatherDayly(OpenWeather *veather)
{
    req_setopt(req, REQ_SET_URI, requeSendDataForcastDayly);
    int status = req_perform(req);
    ESP_LOGI("HTTP", "ststus %d", status);
    if( xSemaphoreTake( xSemaphoreDataIsGet, portMAX_DELAY ) == pdTRUE )
    {
        if(root == NULL)
            return -1;
        getWetharData(root, veather);
        ESP_LOGI("weathe", "ok res weather");
        cJSON_Delete(root);
        return 0;
    }
    return -1;
}


int askWeatherOneCall(OpenWeather *current, OpenWeather *dayly, int days, OpenWeather *hourly, int houres)
{
    req_setopt(req, REQ_SET_URI, requeSendDataOneCall);
    int status = req_perform(req);
    ESP_LOGI("HTTP", "ststus %d", status);
    if( xSemaphoreTake( xSemaphoreDataIsGet, portMAX_DELAY ) == pdTRUE )
    {
        ESP_LOGI("weathe", "begin analize weather");
        if(root == NULL)
            return -1;
        getWetharOneCallData(root, current, dayly, days, hourly, houres);
        ESP_LOGI("weathe", "ok res weather");
        cJSON_Delete(root);
        return 0;
    }
    return -1;
}

void printOpenWeather(OpenWeather weather)
{
    //weather
    ESP_LOGI("weathe", "weatherIcon %s", weather.weatherIcon);
    ESP_LOGI("weathe", "weather code %d", weather.code);
    ESP_LOGI("weathe", "weather main %s", weather.weatherName);
    //main
    ESP_LOGI("weathe", "temp %f", weather.temp);
    ESP_LOGI("weathe", "feels_like %f", weather.feels_like);
    ESP_LOGI("weathe", "temp_min %f", weather.temp_min);
    ESP_LOGI("weathe", "temp_max %f", weather.temp_max);
    ESP_LOGI("weathe", "pressure %f", weather.pressure);
    ESP_LOGI("weathe", "humidity %f", weather.humidity);
//    ESP_LOGI("weathe", "sea_level %f", weather.sea_level);
//    ESP_LOGI("weathe", "grnd_level %f", weather.grnd_level);
    //visibility
    ESP_LOGI("weathe", "visibility %d", weather.visibility);
    //wind
    ESP_LOGI("weathe", "wind_speed %d", weather.wind_speed);
    ESP_LOGI("weathe", "wind_deg %d", weather.wind_deg);
    //clouds
    ESP_LOGI("weathe", "clouds %d", weather.clouds);
    //sys
    ESP_LOGI("weathe", "sunrise %d", weather.sunrise);
    ESP_LOGI("weathe", "sunset %d", weather.sunset);
    ESP_LOGI("weathe", "sunset %d", (int)(weather.phaseMoon*24));
}

int getImageGuiWheather(OpenWeather weather)
{
    int res = 39;

    switch (weather.code) {
        case 200:
        case 201:
        case 202:
        case 210:
        case 211:
        case 212:
        case 221: res = 11; break;
        case 230:
        case 231:
        case 232: res = 12; break;


        case 300:
        case 301:
        case 302:
        case 310:
        case 311:
        case 312:
        case 313:
        case 314:
        case 321: res = 10; break;

        case 500:
        case 501: res = 13; break;

        case 520: res = 14; break;
        case 502:
        case 503: res = 15; break;
        case 504: res = 17; break;
        case 521:
        case 522:
        case 531: res = 16; break;

        case 511: res = 19; break;

        case 600: res = 22; break;
        case 601: res = 23; break;
        case 602: res = 24; break;
        case 620: res = 23; break;
        case 621: res = 24; break;
        case 622: res = 25; break;

        case 611:
        case 612:
        case 613:
        case 615:
        case 616: res = 20; break;

        case 701: res = 30; break;
        case 711: res = 30; break;
        case 721: res = 30; break;
        case 731: res = 28; break;
        case 741: res = 30; break;
        case 751: res = 31; break;
        case 761: res = 26; break;
        case 762: res = 31; break;
        case 771: res = 35; break;
        case 781: res = 36; break;

        case 800:
            if(weather.weatherIcon[2] == 'd')   res = 0;
            else                                res = 1;
            break;
        case 801:
            if(weather.weatherIcon[2] == 'd')   res = 5;
            else                                res = 6;
            break;
        case 802:
        case 803:
        case 804: res = 4; break; /*9*/

    default: res = 39; break;

    }

    return res;
}

Image getImageWheather(OpenWeather weather, bool big)
{
    Image res;
    switch (weather.code) {
        case 200:
        case 201:
        case 202:
        case 210:
        case 211:
        case 212:
        case 221:
        case 230:
        case 231:
        case 232: res = big ? B_Wthunderstorm : Wthunderstorm; break;

        case 300:
        case 301:
        case 302:
        case 310:
        case 311:
        case 312:
        case 313:
        case 314:
        case 321: res = big ? B_Wdrizzle : Wdrizzle; break;

        case 500:
        case 501: res = big ? B_WlightRain : WlightRain; break;

        case 502:
        case 503:
        case 504:
        case 521:
        case 522:
        case 531: res = big ? B_Wrain : Wrain; break;

        case 511: res = big ? B_Whail : Whail; break;

        case 600:
        case 601:
        case 602:
        case 615:
        case 620:
        case 621:
        case 622: res = big ? B_Wsnow : Wsnow; break;

        case 611:
        case 612:
        case 616:
        case 613: res = big ? B_Wsleet : Wsleet; break;

        case 701: res = big ? B_Wfog : Wfog; break;
        case 711: res = big ? B_Wfog : Wfog; break;
        case 721: res = big ? B_Wfog : Wfog; break;
        case 731: res = big ? B_Wwind : Wwind; break;
        case 741: res = big ? B_Wfog : Wfog; break;
        case 751: res = big ? B_Wwind : Wwind; break;
        case 761: res = big ? B_Wwind : Wwind; break;
        case 762: res = big ? B_Wwind : Wwind; break;
        case 771: res = big ? B_Wwind : Wwind; break;
        case 781: res = big ? B_Wwind : Wwind; break;

        case 800:
            if(weather.weatherIcon[2] == 'd')   res = big ? B_Wclear_day : Wclear_day;
            else                                res = big ? B_Wclear_night : Wclear_night;
            break;
        case 801:
            if(weather.weatherIcon[2] == 'd')   res = big ? B_Wparly_cloudy_day : Wparly_cloudy_day;
            else                                res = big ? B_Wparly_cloudy_night : Wparly_cloudy_night;
            break;
        case 802:
        case 803:
        case 804: res = big ? B_Wcloudy : Wcloudy; break;

    default: res = big ? B_Wunknown : Wunknown; break;

    }

    return res;
}

Image getWind(int deg)
{
    Image res;
         if((deg >= 23 ) && (deg <= 67 ))      res = WindSW;
    else if((deg >= 68 ) && (deg <= 112))      res = WindW;
    else if((deg >= 113) && (deg <= 157))      res = WindNW;
    else if((deg >= 158) && (deg <= 202))      res = WindN;
    else if((deg >= 203) && (deg <= 247))      res = WindNE;
    else if((deg >= 248) && (deg <= 292))      res = WindE;
    else if((deg >= 248) && (deg <= 292))      res = WindSE;
    else                                       res = WindS;

    return res;
}

