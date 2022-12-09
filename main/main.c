/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "time.h"
//#include "soc/rtc.h"
//#include "driver/mcpwm.h"
#include "driver/gpio.h"
//#include "soc/mcpwm_periph.h"
#include "driver/i2c.h"
#include "nvs_flash.h"
#include "driver/mcpwm.h"
//#include "soc/mcpwm_periph.h"

#include "gpioDEF.h"

#include "net.h"
#include "bme_i2c.h"
#include "bme280_ok.h"
#include "display.h"
#include "mh-z19.h"

//#include "esp32-rf24.h"
#include "RF24.h"
//#include "esp_nrf24.h"
//#include "esp_nrf24_map.h"
//#include "nrf24l01.h"
#include "openWeather.h"
#include "ipgeolocation.h"

#include "mqtt_client.h"

#include "display_gui.h"

#define WHEATHER_DAYS_READ      SIZE_FORCAST
#define WHEATHER_HOUR_READ      STEP_FORCAST_HOUR*SIZE_FORCAST

#define EXT_NRF_SIZE            SIZE_EXT_DATA


#define GPIO_INPUT_PIN_SEL  ((1ULL << GPIO_INPUT_IO_3) | (1ULL << GPIO_INPUT_IO_4))

#define GPIO_LEFT           GPIO_INPUT_IO_1
#define GPIO_RIGHT          GPIO_INPUT_IO_3
#define GPIO_MENU           GPIO_INPUT_IO_2
#define GPIO_BACK           GPIO_INPUT_IO_4
//#define GPIO_UP             GPIO_INPUT_IO_1
//#define GPIO_DOWN           GPIO_INPUT_IO_4


const char *TAG = "meteoCS";

OpenWeather weatherCurent, weatherDayli[WHEATHER_DAYS_READ], weatherHourly[WHEATHER_HOUR_READ];
//static SemaphoreHandle_t semaphoreDisplayChange;
//static enum stateDisplay stateDis = stateWeather;
//static SemaphoreHandle_t semaphoreDisplayNextState;
uint32_t periodChange = 30000;//NOTE configuraleble state or config individual stete for ever display
double temp = 0.0, pressure = 0.0, humidity = 0.0;
uint16_t co2Val;

uint8_t NRF_ConnectedDevice = 0;//Read from mem
uint64_t localPipe;


int reciveSMD(char *rx, char *tx, int n)//recive from wi-fi
{
    strcpy(tx, "OK");
    int ret = 2;
    if(strcmp(rx, "askData") == 0)
    {
        sprintf(tx, "T=%.2f;P=%.2f;H=%.2f;", temp,pressure, humidity);
        //         strcpy(tx, "OK");
        ret = strlen(tx);
    }
    else
    {

    }

    return ret;
}

void taskBME(void *p)
{
    ESP_LOGI("BME280", "begin init bme");

    i2c_master_init();

    struct bme280_t bme280 = {
        .bus_write = BME280_I2C_bus_write,
                .bus_read = BME280_I2C_bus_read,
                .dev_addr = BME280_I2C_ADDRESS1,
                .delay_msec = BME280_delay_msek
    };

    s32 com_rslt;

    s32 v_uncomp_pressure_s32;
    s32 v_uncomp_temperature_s32;
    s32 v_uncomp_humidity_s32;

    com_rslt = bme280_init(&bme280);

    com_rslt += bme280_set_oversamp_pressure(BME280_OVERSAMP_16X);
    com_rslt += bme280_set_oversamp_temperature(BME280_OVERSAMP_2X);
    com_rslt += bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);

    com_rslt += bme280_set_standby_durn(BME280_STANDBY_TIME_1_MS);
    com_rslt += bme280_set_filter(BME280_FILTER_COEFF_16);

    com_rslt += bme280_set_power_mode(BME280_NORMAL_MODE);

    if (com_rslt == SUCCESS)
    {
        while(true)
        {
            vTaskDelay(10000/portTICK_RATE_MS);

            com_rslt = bme280_read_uncomp_pressure_temperature_humidity(
                        &v_uncomp_pressure_s32, &v_uncomp_temperature_s32, &v_uncomp_humidity_s32);

            if (com_rslt == SUCCESS)
            {
                temp = bme280_compensate_temperature_double(v_uncomp_temperature_s32);
                pressure = bme280_compensate_pressure_double(v_uncomp_pressure_s32)/100.0;// Pa -> hPa
                humidity = bme280_compensate_humidity_double(v_uncomp_humidity_s32);

                ESP_LOGI("BME280", "%.2f degC / %.3f hPa / %.3f %%",
                         temp,
                         pressure,
                         humidity);
            } else {
                ESP_LOGE("BME280", "measure error. code: %d", com_rslt);
            }
        }
    } else {
        ESP_LOGE("BME280", "init or setting error. code: %d", com_rslt);
    }

    vTaskDelete(NULL);
}

void taskDisplay(void *p)
{
    (void)p;
////    drawMainForm();


//    ESP_LOGI(TAG, "BEGIN display");

////    char buff[100];

//    vTaskDelay(100/portTICK_RATE_MS);

//    drawWheather();

////    setCO2(0);
//    vTaskDelay(1000/portTICK_RATE_MS);

    while(1)
    {
//        //wayt 3 seconds + while chnage states
//        if(xSemaphoreTake(semaphoreDisplayChange, 3000/portTICK_RATE_MS) == pdTRUE)
//        {
//            if(stateDis == stateMainForm)
//                drawMainForm();
//            else if(stateDis == stateDipForm)
//                drawDipForm();
//            else if(stateDis == stateDip2Form)
//                drawDip2Form();
//            else if(stateDis == stateWeatherTeat)
//                drawWheatherTest();
//            else if(stateDis == stateWeather)
//                drawWheather();

//            else if(stateDis == stateTest1)
//                print_imTest1();
//            else if(stateDis == stateTest2)
//                print_imTest2();
//            else if(stateDis == stateTest3)
//                print_imTest3();
//            else if(stateDis == stateTest4)
//                print_imTest4();
//            else if(stateDis == stateTestTiger)
//                print_imTestTiger();
//            else if(stateDis == stateTestNu0)
//                print_imTestNy0();
//            else if(stateDis == stateTestNu1)
//                print_imTestNy1();
//            else if(stateDis == stateTestNu2)
//                print_imTestNy2();
//            else if(stateDis == stateTestNu3)
//                print_imTestNy3();
//            else if(stateDis == stateTestG1)
//                print_imTestG1();
//            else if(stateDis == stateTestG2)
//                print_imTestG2();
//        }
        vTaskDelay(1000/portTICK_RATE_MS);
        setHumiditi(humidity);
        setTemperature(temp);
        setPressure(pressure);
        setCO2(co2Val);
//        testMoon();
//        setWheather(&weatherCurent);

////        vTaskDelay(5000/portTICK_RATE_MS);

    }
}

void initOutGPIO()
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = ((1ULL << GPIO_LED_GREEN) | (1ULL << GPIO_LED_YELLOW) | (1ULL << GPIO_BUZZ_ON) |\
                            (1ULL << GPIO_LED_ORANGE) | ( 1ULL << GPIO_LED_RED) | (1ULL << GPIO_EN_CO2));
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

//    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_BUZZ_ON);
//    mcpwm_config_t pwm_config;
//    pwm_config.frequency = 1000;    //frequency = 1000Hz
//    pwm_config.cmpr_a = 0.0;       //duty cycle of PWMxA = 0.0%
//    pwm_config.cmpr_b = 0.0;       //duty cycle of PWMxb = 0.0%
//    pwm_config.counter_mode = MCPWM_UP_COUNTER;
//    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
//    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);   //Configure PWM0A & PWM0B with above settings
}

void taskButton(void *p)
{
    (void)p;

//    gpio_config_t io_conf;
//    //disable interrupt
//    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
//    //set as output mode
//    io_conf.mode = GPIO_MODE_INPUT;
//    //bit mask of the pins that you want to set,e.g.GPIO18/19
//    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
//    //disable pull-down mode
//    io_conf.pull_down_en = 0;
//    //disable pull-up mode
//    io_conf.pull_up_en = 0;
//    //configure GPIO with the given settings
//    gpio_config(&io_conf);

    gpio_set_direction(GPIO_LEFT, GPIO_MODE_INPUT);
    gpio_pulldown_dis(GPIO_LEFT);
    gpio_pullup_dis(GPIO_LEFT);

    gpio_set_direction(GPIO_RIGHT, GPIO_MODE_INPUT);
    gpio_pulldown_dis(GPIO_RIGHT);
    gpio_pullup_dis(GPIO_RIGHT);

    gpio_set_direction(GPIO_MENU, GPIO_MODE_INPUT);
    gpio_pulldown_dis(GPIO_MENU);
    gpio_pullup_dis(GPIO_MENU);

    gpio_set_direction(GPIO_BACK, GPIO_MODE_INPUT);
    gpio_pulldown_dis(GPIO_BACK);
    gpio_pullup_dis(GPIO_BACK);

    bool oldBleft = false, oldBright = false, oldBmenu = false, oldBback = false;
    bool Bleft = false, Bright = false, Bmenu = false, Bback = false;

    while(1)
    {
        vTaskDelay(50/portTICK_RATE_MS);

        Bright = !gpio_get_level(GPIO_RIGHT);
        Bleft = gpio_get_level(GPIO_LEFT);
        Bmenu = gpio_get_level(GPIO_MENU);
        Bback = gpio_get_level(GPIO_BACK);


        if(oldBleft != Bleft)
        {
            oldBleft = Bleft;
//            ESP_LOGI("BUTTON", "left if %s", Bleft ? "DOWN" : "UP");
//            gpio_set_level(GPIO_LED_GREEN, Bleft ? 1 : 0);
            meteoButtonClicked(ButtonPrev, Bleft ? ButtonPresed : ButtonRealesed);
            if(!Bleft)
            {
//                xSemaphoreGive(semaphoreDisplayNextState);
            }
        }

        if(oldBright != Bright)
        {
            oldBright = Bright;
//            ESP_LOGI("BUTTON", "right if %s", Bright ? "DOWN" : "UP");
//            gpio_set_level(GPIO_LED_YELLOW, Bright ? 1 : 0);
            meteoButtonClicked(ButtonNext, Bright ? ButtonPresed : ButtonRealesed);
            if(!Bright)
            {
//                if(stateDis == stateMainForm)
//                    stateDis = stateDip2Form;
//                else if(stateDis == stateDip2Form)
//                    stateDis = stateDipForm;
//                else
//                    stateDis = stateMainForm;
//                xSemaphoreGive(semaphoreDisplayChange);
            }
        }

        if(oldBmenu != Bmenu)
        {
            oldBmenu = Bmenu;
//            ESP_LOGI("BUTTON", "menu if %s", Bmenu ? "DOWN" : "UP");
//            gpio_set_level(GPIO_LED_ORANGE, Bmenu ? 1 : 0);
            meteoButtonClicked(ButtonSettings, Bmenu ? ButtonPresed : ButtonRealesed);
//            gpio_set_level(GPIO_BUZZ_ON, Bmenu ? 1 : 0);
//            mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM0A, Bmenu ? 10.0 : 0.0);
        }

        if(oldBback != Bback)
        {
            oldBback = Bback;
//            ESP_LOGI("BUTTON", "back if %s", Bback ? "DOWN" : "UP");
//            gpio_set_level(GPIO_LED_RED, Bback ? 1 : 0);
            meteoButtonClicked(ButtonAction, Bback ? ButtonPresed : ButtonRealesed);
//            gpio_set_level(GPIO_BUZZ_ON, Bback ? 1 : 0);
//            mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM0A, Bback ? 10.0 : 0.0);

            if(!Bback)
            {
                ESP_LOGI("Memory", "Free heap size %d", esp_get_free_heap_size());
                ESP_LOGI("Memory", "Free internal heap size %d", esp_get_free_internal_heap_size());
                ESP_LOGI("Memory", "Free minimum heap size %d", esp_get_minimum_free_heap_size());
            }

        }
    }
}

void task_co2(void *p)
{
    (void)p;
    
//    gpio_set_level(GPIO_EN_CO2, 1);

//    co2_init();
//    vTaskDelay(10000/portTICK_RATE_MS);
    
    

    while(1)
    {
        gpio_set_level(GPIO_EN_CO2, 1);
        
        vTaskDelay(10000/portTICK_RATE_MS);

        co2_init();
        vTaskDelay(240000/portTICK_RATE_MS);
        co2_read();
        //vTaskDelay(240000/portTICK_RATE_MS);
//        for(int i = 0; i < 60; i++)
        gpio_set_level(GPIO_LED_GREEN, 1);
        while(1)
        {
            co2Val = co2_read();
            ESP_LOGI("CO2", "CO2 %d", co2Val);
            gpio_set_level(GPIO_LED_YELLOW, co2Val >= CO2_BEGIN_YELLOW);
            gpio_set_level(GPIO_LED_ORANGE, co2Val >= CO2_BEGIN_ORANG);
            gpio_set_level(GPIO_LED_RED, co2Val >= CO2_BEGIN_RED);
            vTaskDelay(60000/portTICK_RATE_MS);
        }
        gpio_set_level(GPIO_EN_CO2, 0);
        vTaskDelay(60000/portTICK_RATE_MS);
    }

}

//extern spi_bus_config_t buscfg;
void addNewNRF(void)
{
    ESP_LOGI("NRF", "Add new dev");
    uint8_t buffTx[8] = {0};
    uint64_t newPipe = localPipe + NRF_ConnectedDevice + 1;
    memcpy(buffTx, &newPipe, 5);
    writeData(buffTx, 8);
}

void nRF24_new_task(void *pvParameters)
{
    vTaskDelay(1000/portTICK_RATE_MS);
    (void)pvParameters;

    const uint64_t pipeWrite = 0xE8E8E8E8E8LL;  //идентификатор трубы от ControlStation до внешнего модуля
    const uint64_t pipeGlovabal = 0xE8E8F0F0E2LL;  //обший идентификатор трубы
    //read id
    uint8_t chipid[8];
    esp_efuse_mac_get_default(chipid);
    uint64_t chipid64 = 0;
    memcpy(&chipid64, chipid, 6);
    ESP_LOGI("chipId", "%X\n", (uint32_t)chipid64);
    ESP_LOGI("chipId", "%02X:%02X:%02X:%02X:%02X:%02X", chipid[0], \
            chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);
    localPipe = chipid64 << 8;
    //localPipe_n = {localPipe[40:8], n} == localPipe + n == localPipe | n;
//    uint64_t NRF
    NRF_ConnectedDevice = 0;//Read from mem



    rf24_bus_cfg_t rf24_bus_cfg = {
        .spi_host = VSPI_HOST,
        .init_host = false,
        .mosi_io_num = GPIO_SPI_MOSI,
        .miso_io_num = GPIO_SPI_MISO,
        .sclk_io_num = GPIO_SPI_CLK,
        .cs_io_num = GPIO_NRF_CS,
        .ce_io_num = GPIO_NRF_CE
    };

    NRF_Init(rf24_bus_cfg);
    ////////////// SET ////////////////
    //enableAckPayload(); //отключаем полезную нагрузку в автоответе
    setAutoAck(false); //отключаем автоответе
    disableDynamicPayloads(); //отключаем динамический размер нагрузки
    //disableCRC();
    setPayloadSize(8); //размер нагрузки 8 байт
    setChannel(100); //канал 100
    maskIRQ(true, true, false);//enable inly rx interupt
    openWritingPipe(pipeWrite);

    openReadingPipe(0, pipeGlovabal); //открываем трубу с номером 0

    for(uint8_t i = 1; i <= EXT_NRF_SIZE; i++)
        openReadingPipe(i, localPipe + i); //открываем трубу с номером 1
    startListening();
    ///////////////////////////////////

    uint8_t buff[8] = {0};
//    uint8_t buffTx[8] = {0x12, 0x59, 0xA7, 0x6C, 0x4E, 0xF0, 0x70, 0x33};
    int status;

    ESP_LOGI("NRF", "add ext fnction");
    setAddExternal(addNewNRF);//set function on add button

    //init gpio interupt
    gpio_set_direction(GPIO_NRF_IRQ, GPIO_MODE_INPUT);
    gpio_pulldown_en(GPIO_NRF_IRQ);
    gpio_pullup_dis(GPIO_NRF_IRQ);

    while(1)
    {
        vTaskDelay(10/portTICK_RATE_MS);
        if(gpio_get_level(GPIO_NRF_IRQ))//if set 1 rx is empty
            continue;
//        if(!availableMy())
//        {

//        }
//        else
        ESP_LOGI("NRF", "ststus change");
        if(availableMy())
        {
            status = read_payload(buff, 8);
            uint8_t numPipeFromStatus = (status >> 1) & 0x07;
            whatHappened();//clear status

            //if data from new pipe
            if(NRF_ConnectedDevice < numPipeFromStatus)
            {
                NRF_ConnectedDevice++;
                ESP_LOGI("NRF", "add new pipe done %d", NRF_ConnectedDevice);
                //save to flash maybe
            }

            ESP_LOGI("NRF", "pipe:%d status = %d", numPipeFromStatus, status);
            for(int i = 0; i < 8; i++)
            {
                ESP_LOGI("NRF", "data[%d]=%d", i, buff[i]);
            }
            //TODO convet input data
            //write it in display
        }
        else
            whatHappened();//clear status
//        writeData(buffTx, 8);

    }
    //E8E8F0F0E2
    vTaskDelete(NULL);
}

extern EventGroupHandle_t s_wifi_event_group;
OpenWeather weatherCurent, weatherDayli[WHEATHER_DAYS_READ], weatherHourly[WHEATHER_HOUR_READ];

void locationAndWeatherTask(void *p)
{
    (void)p;

    while(xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY) != pdTRUE) {};

    //get time and location
    IpLocation location;

    while(getLocation(&location) != 0)//tru to get location ever 10 minute
    {
        vTaskDelay(10*60000/portTICK_RATE_MS);//10m
    }
    ESP_LOGI("location", "ok get locaion and time");

    struct tm tm;

    tm.tm_year = location.year - 1900;
    tm.tm_mon = location.month - 1;
    tm.tm_mday = location.day;

    tm.tm_hour = location.hour;
    tm.tm_min = location.minutes;
    //set local time
    time_t t = mktime(&tm);
    struct timeval now = { .tv_sec = t };
    settimeofday(&now, NULL);

    //get local time
    time_t _now = time(0);
    // Convert now to tm struct for local timezone
    struct tm* localtm = localtime(&_now);

    ESP_LOGI("time", "The local date and time is: %s", asctime(localtm));

//    printf("Setting time: %s", asctime(&tm));

    //set location for openweather
    setLocation(location.city, location.country_code, location.lat, location.lon);

    if(initOpenWeather() != 0)
    {
        ESP_LOGI("weather", "location is not sets");
        vTaskDelete(NULL);
    }



    while(1)
    {

        ESP_LOGI("weather", "begin read weather");
//        if(askWeatherOneCall(&weatherCurent, weatherDayli, WHEATHER_DAYS_READ, weatherHourly, WHEATHER_HOUR_READ) == 0)
        if(askWeather(&weatherCurent) == 0)
        {
            printOpenWeather(weatherCurent);

            //set update time
            _now = time(0);
            localtm = localtime(&_now);

            setWeatherUpdateTime(localtm->tm_hour, localtm->tm_min);

            //set weather to display
            setWeatherCurentTemp((int)weatherCurent.temp);
            setWeatherCurentTempFell((int)weatherCurent.feels_like);
            setWeatherCurentHummidity((int)weatherCurent.humidity);



//            ESP_LOGI("weather", "day 0");
//            printOpenWeather(weatherDayli[0]);

//            ESP_LOGI("weather", "day 2");
//            printOpenWeather(weatherDayli[2]);

//            ESP_LOGI("weather", "hour 2");
//            printOpenWeather(weatherHourly[2]);

//            ESP_LOGI("weather", "day 7");
//            printOpenWeather(weatherHourly[7]);
        }
        vTaskDelay(60000/portTICK_RATE_MS);//1min
    }
}

static void log_error_if_nonzero(const char * message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("MQTT", "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
            ESP_LOGI("MQTT", "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI("MQTT", "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI("MQTT", "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            ESP_LOGI("MQTT", "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI("MQTT", "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI("MQTT", "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI("MQTT", "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI("MQTT", "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI("MQTT", "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI("MQTT", "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI("MQTT", "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI("MQTT", "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

            }
            break;
        default:
            ESP_LOGI("MQTT", "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD("MQTT", "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void task_MQTT(void *p)
{
    while(xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY) != pdTRUE) {};

    vTaskDelay(10000/portTICK_RATE_MS);
    ESP_LOGI("MQTT", "begin MQTT");

    (void)p;
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_MQTT_SERVER/*"mqtt://192.168.0.100:1883"*/,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);


    while(1)
    {
        vTaskDelay(10000/portTICK_RATE_MS);
        char string[32] = {0};

        memset(string, 0, 32);
        sprintf(string, "%d", (int)temp);
        ESP_LOGI("MQTT", "publish temperature");
        esp_mqtt_client_publish(client, "/meteoCS/temp0", string, strlen(string), 0, 0);

        memset(string, 0, 32);
        sprintf(string, "%d", (int)humidity);
        ESP_LOGI("MQTT", "publish humidity");
        esp_mqtt_client_publish(client, "/meteoCS/humidity0", string, strlen(string), 0, 0);

        memset(string, 0, 32);
        sprintf(string, "%d", (int)pressure);
        ESP_LOGI("MQTT", "publish pressure");
        esp_mqtt_client_publish(client, "/meteoCS/pressure0", string, strlen(string), 0, 0);

        memset(string, 0, 32);
        sprintf(string, "%d", (int)co2Val);
        ESP_LOGI("MQTT", "publish co2Val");
        esp_mqtt_client_publish(client, "/meteoCS/CO2", string, strlen(string), 0, 0);
    }
}

void app_main(void)
{
//    ESP_LOGI("Chip", "Free heap size %d", esp_get_free_heap_size());
//    ESP_LOGI("Chip", "Free internal heap size %d", esp_get_free_internal_heap_size());
//    ESP_LOGI("Chip", "Free minimum heap size %d", esp_get_minimum_free_heap_size());

//    /* Print chip information */
//    esp_chip_info_t chip_info;
//    esp_chip_info(&chip_info);
//    ESP_LOGI("Chip", "This is %s chip with %d CPU core(s), WiFi%s%s, ",
//            CONFIG_IDF_TARGET,
//            chip_info.cores,
//            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
//            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

//    ESP_LOGI("Chip", "silicon revision %d, ", chip_info.revision);

//    ESP_LOGI("Chip", "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
//            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");


    esp_log_level_set("MQTT", ESP_LOG_NONE);
    esp_log_level_set("wifi_manager", ESP_LOG_NONE);
    esp_log_level_set("weathe:", ESP_LOG_NONE);
    esp_log_level_set("HTTP", ESP_LOG_NONE);
    esp_log_level_set("HTTP_REQ", ESP_LOG_NONE);
    esp_log_level_set("TRANS_TCP", ESP_LOG_NONE);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_NONE);

    initOutGPIO();


//    esp_log_level_set("CO2", ESP_LOG_VERBOSE);
//    esp_log_level_set("CO2", ESP_LOG_DEBUG);
//    esp_log_level_set("CO2", ESP_LOG_INFO);
//    esp_log_level_set("CO2", ESP_LOG_WARN);
//    esp_log_level_set("CO2", ESP_LOG_ERROR);


    s_wifi_event_group = xEventGroupCreate();
//    semaphoreDisplayChange = xSemaphoreCreateBinary();
//    semaphoreDisplayNextState = xSemaphoreCreateBinary();
//    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1); 
    xTaskCreate(guiTask, "gui", 4096*2, drowDisplayLVGL, 1, NULL);
    ESP_LOGI("Dislay", "Drow display");

    vTaskDelay(1000);

//    ESP_LOGI("Memory", "Free heap size %d", esp_get_free_heap_size());
//    ESP_LOGI("Memory", "Free internal heap size %d", esp_get_free_internal_heap_size());
//    ESP_LOGI("Memory", "Free minimum heap size %d", esp_get_minimum_free_heap_size());
//    vTaskDelay(1000/portTICK_RATE_MS);


//    xTaskCreate(locationAndWeatherTask, "openWeathre_location", 4096*1, NULL, 1, NULL);
//    xTaskCreate(nRF24_new_task, "nrf", 4096, NULL, 2, NULL);
    xTaskCreate(taskDisplay, "Display", 2048, NULL, 1, NULL);
    xTaskCreate(taskBME, "BME", 2048, NULL, 2, NULL);
    xTaskCreate(taskButton, "Button", 2048, NULL, 2, NULL);
    xTaskCreate(task_co2, "co2", 2048, NULL, 2, NULL);
//    xTaskCreate(task_MQTT, "MQTT", 2560, NULL, 2, NULL);





//    ESP_ERROR_CHECK(nvs_flash_init());
//    ESP_ERROR_CHECK(esp_netif_init());
//    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    //ESP_ERROR_CHECK(example_connect());
    //Initialize NVS
//    esp_err_t ret = nvs_flash_init();
//    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//        ESP_ERROR_CHECK(nvs_flash_erase());
//        ret = nvs_flash_init();
//    }
//    ESP_ERROR_CHECK(ret);


//    nvs_flash_init();
//    nvs_handle_t handle_nvs;
//    nvs_open("espwifimgr", NVS_READWRITE, handle_nvs);
//    nvs_erase_all(handle_nvs);
//    nvs_flash_erase();
//    nvs_flash_erase_partition("espwifimgr");

    vTaskDelay(3000);//delay before start becouse net use display
                    //so i need wait before load all display
    ESP_LOGI(TAG, "Start wifi manager");
    startNet();


//#ifdef CONFIG_EXAMPLE_IPV4
//    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
//#endif
//#ifdef CONFIG_EXAMPLE_IPV6
//    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET6, 5, NULL);
//#endif

//

//    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
}
