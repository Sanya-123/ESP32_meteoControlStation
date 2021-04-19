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
//#include "soc/rtc.h"
//#include "driver/mcpwm.h"
#include "driver/gpio.h"
//#include "soc/mcpwm_periph.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "nvs_flash.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#include "gpioDEF.h"

#include "net.h"
#include "bme280_ok.h"
#include "display.h"
#include "mh-z19.h"
#include "gpioDEF.h"

//#include "lwip/err.h"
//#include "lwip/sockets.h"
//#include "lwip/sys.h"
//#include "lwip/netdb.h"
//#include "lwip/dns.h"
//#include "esp32-rf24.h"
#include "RF24.h"
//#include "nrf24l01.h"
#include "openWeather.h"

#include "mqtt_client.h"



#define GPIO_INPUT_PIN_SEL  ((1ULL << GPIO_INPUT_IO_3) | (1ULL << GPIO_INPUT_IO_4))

#define GPIO_MENU           GPIO_INPUT_IO_3
#define GPIO_BACK           GPIO_INPUT_IO_4
//#define GPIO_UP             GPIO_INPUT_IO_1
//#define GPIO_DOWN           GPIO_INPUT_IO_4
#define GPIO_RIGHT          GPIO_INPUT_IO_2
#define GPIO_LEFT           GPIO_INPUT_IO_1


const char *TAG = "meteoCS";

OpenWeather weather;
static SemaphoreHandle_t semaphoreDisplayChange;
static enum stateDisplay stateDis = stateDip2Form;


#define I2C_MASTER_ACK 0
#define I2C_MASTER_NACK 1

void i2c_master_init()
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 21,
        .scl_io_num = 22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 1000000
    };
    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
    s32 iError = BME280_INIT_VALUE;

    esp_err_t espRc;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write(cmd, reg_data, cnt, true);
    i2c_master_stop(cmd);

    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (espRc == ESP_OK) {
        iError = SUCCESS;
    } else {
        iError = FAIL;
    }
    i2c_cmd_link_delete(cmd);

    return (s8)iError;
}

s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
    s32 iError = BME280_INIT_VALUE;
    esp_err_t espRc;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

    if (cnt > 1) {
        i2c_master_read(cmd, reg_data, cnt-1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, reg_data+cnt-1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (espRc == ESP_OK) {
        iError = SUCCESS;
    } else {
        iError = FAIL;
    }

    i2c_cmd_link_delete(cmd);

    return (s8)iError;
}

void BME280_delay_msek(u32 msek)
{
    vTaskDelay(msek/*/portTICK_PERIOD_MS*/);
}

double temp = 0.0, pressure = 0.0, humidity = 0.0;
uint16_t co2Val;

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

#include "tft.h"

void taskDisplay(void *p)
{
    (void)p;
//    drawMainForm();


    ESP_LOGI(TAG, "BEGIN display");

//    char buff[100];

    vTaskDelay(1000/portTICK_RATE_MS);

    drawDip2Form();

//    setCO2(0);
    vTaskDelay(1000/portTICK_RATE_MS);

    while(1)
    {
        if(xSemaphoreTake(semaphoreDisplayChange, 5000/portTICK_RATE_MS) == pdTRUE)
        {
            if(stateDis == stateMainForm)
                drawMainForm();
            else if(stateDis == stateDipForm)
                drawDipForm();
            else if(stateDis == stateDip2Form)
                drawDip2Form();
        }
//        vTaskDelay(1000/portTICK_RATE_MS);
        setHumiditi(humidity);
        setTerm(temp);
        setPa(pressure);
        setCO2(co2Val);

//        vTaskDelay(5000/portTICK_RATE_MS);

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

    bool oldBup = false, oldBleft = false, oldBright = false, oldBdown = false, oldBmenu = false, oldBback = false, oldB0 = false;
    bool Bup = false, Bleft = false, Bright = false, Bdown = false, Bmenu = false, Bback = false, B0 = false;

    while(1)
    {
        vTaskDelay(50/portTICK_RATE_MS);

        Bright = gpio_get_level(GPIO_RIGHT);
        Bleft = gpio_get_level(GPIO_LEFT);
        Bmenu = !gpio_get_level(GPIO_MENU);
        Bback = gpio_get_level(GPIO_BACK);


        if(oldBleft != Bleft)
        {
            oldBleft = Bleft;
            ESP_LOGI("BUTTON", "left if %s", Bleft ? "DOWN" : "UP");
            gpio_set_level(GPIO_LED_GREEN, Bleft ? 1 : 0);
            if(!Bleft)
            {
                if(stateDis == stateMainForm)
                    stateDis = stateDipForm;
                else if(stateDis == stateDipForm)
                    stateDis = stateDip2Form;
                else
                    stateDis = stateMainForm;
                xSemaphoreGive(semaphoreDisplayChange);
            }
        }

        if(oldBright != Bright)
        {
            oldBright = Bright;
            ESP_LOGI("BUTTON", "right if %s", Bright ? "DOWN" : "UP");
            gpio_set_level(GPIO_LED_YELLOW, Bright ? 1 : 0);
            if(!Bright)
            {
                if(stateDis == stateMainForm)
                    stateDis = stateDip2Form;
                else if(stateDis == stateDip2Form)
                    stateDis = stateDipForm;
                else
                    stateDis = stateMainForm;
                xSemaphoreGive(semaphoreDisplayChange);
            }
        }

        if(oldBmenu != Bmenu)
        {
            oldBmenu = Bmenu;
            ESP_LOGI("BUTTON", "menu if %s", Bmenu ? "DOWN" : "UP");
            gpio_set_level(GPIO_LED_ORANGE, Bmenu ? 1 : 0);
            gpio_set_level(GPIO_BUZZ_ON, Bmenu ? 1 : 0);
//            mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM0A, Bmenu ? 10.0 : 0.0);
        }

        if(oldBback != Bback)
        {
            oldBback = Bback;
            ESP_LOGI("BUTTON", "back if %s", Bback ? "DOWN" : "UP");
            gpio_set_level(GPIO_LED_RED, Bback ? 1 : 0);
            gpio_set_level(GPIO_BUZZ_ON, Bback ? 1 : 0);
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
    
    gpio_set_level(GPIO_EN_CO2, 1);

    co2_init();
    vTaskDelay(10000/portTICK_RATE_MS);
    
    

    while(1)
    {
        gpio_set_level(GPIO_EN_CO2, 1);
        
        vTaskDelay(10000/portTICK_RATE_MS);

        co2_init();
        vTaskDelay(240000/portTICK_RATE_MS);
        co2_read();
        //vTaskDelay(240000/portTICK_RATE_MS);
//        for(int i = 0; i < 60; i++)
        while(1)
        {
            co2Val = co2_read();
            ESP_LOGI("CO2", "CO2 %d", co2Val);
            vTaskDelay(60000/portTICK_RATE_MS);
        }
        gpio_set_level(GPIO_EN_CO2, 0);
        vTaskDelay(60000/portTICK_RATE_MS);
    }

}

//extern spi_bus_config_t buscfg;

void nRF24_task(void *pvParameters)
{
    vTaskDelay(1000/portTICK_RATE_MS);
    (void)pvParameters;
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
    const uint64_t pipe1 = 0xE8E8F0F0E2LL;  //идентификатор трубы с номером 1
    ////////////// SET ////////////////
    //enableAckPayload(); //отключаем полезную нагрузку в автоответе
    setAutoAck(false); //отключаем автоответе
    disableDynamicPayloads(); //отключаем динамический размер нагрузки
    //disableCRC();
    setPayloadSize(8); //размер нагрузки 8 байт
    setChannel(100); //канал 19
//    openWritingPipe(pipe1); //открываем трубу с номером 1
    openReadingPipe(1, pipe1); //открываем трубу с номером 1
    startListening();
    ///////////////////////////////////

    uint8_t buff[8] = {0};
    uint8_t buffTx[8] = {0x12, 0x59, 0xA7, 0x6C, 0x4E, 0xF0, 0x70, 0x33};
    int status;
//    int i = 0;

    while(1)
    {
        vTaskDelay(10/portTICK_RATE_MS);
        if(!availableMy())
        {
//            i++;
//            if(i == 100)
//            {
//                i = 0;
//                status = get_status();
//                ESP_LOGI("nrf", "status = %d", status);
//            }
//            continue;
        }
        else
        {
            status = read_payload(buff, 8);
    //        write_register(NRF_STATUS, (1 << RX_DR) | (1 << MAX_RT) | (1 << TX_DS));
            ESP_LOGI("nrf", "status = %d", status);
            for(int i = 0; i < 8; i++)
            {
                ESP_LOGI("NRF", "data[%d]=%d", i, buff[i]);
            }
        }
//        vTaskDelay(2500/portTICK_RATE_MS);
//        writeData(buffTx, 8);

    }
//    //E8E8F0F0E2
//    uint8_t addr[5] = {0xE8, 0xE8, 0xF0, 0xF0, 0xE2};
//    uint8_t buff[32] = {0};
//    nrf_init(GPIO_NRF_CE, GPIO_NRF_CS, PIN_NUM_CLK, PIN_NUM_MOSI, PIN_NUM_MISO, nrf_rx_mode);
//    nrf_set_pipe_addr(0, addr, 5);
//    ESP_LOGI("nrf", "Rx mode\n");
//    for(;;) {
//        vTaskDelay(10/portTICK_RATE_MS);


////        ESP_LOGI("nrf", "wait Rx data\n");

//        if (!nrf_is_rx_data_available())
//            continue;

//        nrf_read(buff, 8);
//        ESP_LOGI("nrf", "Received: ");
//        for(int i = 0; i < 8; i++)
//             ESP_LOGI("nrf", "%d ", buff[i]);
//        ESP_LOGI("nrf", "\n");
//    }
    vTaskDelete(NULL);
}

extern EventGroupHandle_t s_wifi_event_group;

void openWeatherTask(void *p)
{
    (void)p;
    initOpenWeather();

    while(1)
    {

        ESP_LOGI("weather", "begin read weather");
        if(askWeather(&weather) == 0)
        {
            printOpenWeather(weather);
        }
        vTaskDelay(60000/portTICK_RATE_MS);
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
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);

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

static uint16_t pixelRead[10][10];

void app_main(void)
{
    esp_log_level_set("MQTT", ESP_LOG_NONE);
    esp_log_level_set("wifi_manager", ESP_LOG_NONE);
    esp_log_level_set("weathe:", ESP_LOG_NONE);
    esp_log_level_set("TRANS_TCP", ESP_LOG_NONE);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_NONE);

    initOutGPIO();

//    esp_log_level_set("CO2", ESP_LOG_VERBOSE);
//    esp_log_level_set("CO2", ESP_LOG_DEBUG);
//    esp_log_level_set("CO2", ESP_LOG_INFO);
//    esp_log_level_set("CO2", ESP_LOG_WARN);
//    esp_log_level_set("CO2", ESP_LOG_ERROR);


    s_wifi_event_group = xEventGroupCreate();
    semaphoreDisplayChange = xSemaphoreCreateBinary();
    initDisplay();


//    ESP_LOGI(TAG, "Read display");

//    read_picturte(30, 80, 10, 10, pixelRead);

//    ESP_LOGI(TAG, "Write display");
//    send_picturte(150, 100, 10, 10, pixelRead);
//    ESP_LOGI(TAG, "Write display OK");

//    xTaskCreate(openWeatherTask, "openWeathre_reque", 3072, NULL, 1, NULL);
    xTaskCreate(taskDisplay, "Display", 2048, NULL, 3, NULL);
    xTaskCreate(taskBME, "BME", 2048, NULL, 2, NULL);
    xTaskCreate(taskButton, "Button", 2048, NULL, 2, NULL);
    xTaskCreate(task_co2, "co2", 2048, NULL, 2, NULL);
    xTaskCreate(nRF24_task, "nrf", 2048, NULL, 1, NULL);
    xTaskCreate(task_MQTT, "MQTT", 2560, NULL, 2, NULL);





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


    nvs_flash_init();

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
