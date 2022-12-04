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
char requeSendDataOneCall[300] = {0};
//char requeSendDataForcastTime[300] = {0};
static cJSON *root = NULL;
static SemaphoreHandle_t xSemaphoreDataIsGet;
//char buff[SIZE_BUFFER_REQUEST] = {0};//NOTE возможно потребуеться больше для предсказания
//uint16_t sizeBuff = 0;
char city[128] = {0};
char countryCode[3] = {0};
static float lat = 0, lon = 0;

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
        day_hour = cJSON_GetObjectItem(_daily, iter);
        _temp = cJSON_GetObjectItem(day_hour, "temp");
        _weather = cJSON_GetObjectItem(day_hour, "weather");
        dayly[i].temp_min = cJSON_GetNumberValue(cJSON_GetObjectItem(_temp, "min")) - 273;
        dayly[i].temp_max = cJSON_GetNumberValue(cJSON_GetObjectItem(_temp, "max")) - 273;

        char *weatherIcon = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "icon"));
        if(weatherIcon != NULL)
            memcpy(dayly[i].weatherIcon, weatherIcon, 3);

        char *weatherMain = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "description"));
        if(weatherMain != NULL)
            memcpy(dayly[i].weatherName, weatherMain, strlen(weatherMain));

        //NOTE параметров много возможно чтот еще нужно будет
    }

    for(int i = 0; i < houres; i++)
    {
        sprintf(iter, "%d", i);
        day_hour = cJSON_GetObjectItem(_hourly, iter);
//        _temp = cJSON_GetObjectItem(day_hour, "temp");
        _weather = cJSON_GetObjectItem(day_hour, "weather");
        dayly[i].temp = cJSON_GetNumberValue(cJSON_GetObjectItem(day_hour, "temp"));

        char *weatherIcon = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "icon"));
        if(weatherIcon != NULL)
            memcpy(dayly[i].weatherIcon, weatherIcon, 3);

        char *weatherMain = cJSON_GetStringValue(/*_weather_icon*/cJSON_GetObjectItem(cJSON_GetArrayItem(_weather, 0), "description"));
        if(weatherMain != NULL)
            memcpy(dayly[i].weatherName, weatherMain, strlen(weatherMain));

        //NOTE параметров много возможно чтот еще нужно будет
    }
}

//extern EventGroupHandle_t s_wifi_event_group;

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
//    while(xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY) != pdTRUE) {};
//    ESP_LOGI(TAG, "Connected to AP, freemem=%d",esp_get_free_heap_size());
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
    sprintf(requeSendDataForcastDayly, "http://api.openweathermap.org/data/2.5/%s?q=%s,%s&APPID=%s&mode=json&units=%s&lang=%s&cnt=%d", "forecast", city, countryCode, apikey, "M", "EN", 10);
    sprintf(requeSendDataOneCall, "http://api.openweathermap.org/data/2.5/%s?lat=%f&lon=%f&APPID=%s&units=%s&lang=%s&exclude=minutely,alerts", "onecall", /*City, "RU", */lat, lon, apikey, "M", "EN");
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

