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
#include "esp_system.h"
#include "esp_log.h"
//#include "soc/rtc.h"
//#include "driver/mcpwm.h"
#include "driver/gpio.h"
//#include "soc/mcpwm_periph.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "nvs_flash.h"

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



#define GPIO_INPUT_PIN_SEL  ((1ULL << GPIO_INPUT_IO_2) | (1ULL << GPIO_INPUT_IO_4) | (1ULL << GPIO_INPUT_IO_5) | (1ULL << GPIO_INPUT_IO_6))

#define GPIO_MENU           GPIO_INPUT_IO_5
#define GPIO_BACK           GPIO_INPUT_IO_6
#define GPIO_UP             GPIO_INPUT_IO_1
#define GPIO_DOWN           GPIO_INPUT_IO_4
#define GPIO_RIGHT          GPIO_INPUT_IO_2
#define GPIO_LEFT           GPIO_INPUT_IO_3


const char *TAG = "meteoCS";

OpenWeather weather;

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

#include "ST7735.h"

void taskDisplay(void *p)
{
    (void)p;
//    drawMainForm();

//    setHumiditi(100);
//    setTerm(-47);
//    setPa(1250);
//    setCO2(900);

    ESP_LOGI(TAG, "BEGIN display");

    /*st7735_draw_char(80, 100, 'F', ILI9341_COLOR565(0xFF, 0xFF, 0x00),  ILI9341_COLOR565(0x00, 0x00, 0xFF), 2);
    st7735_draw_string(150, 150, "CFD", ILI9341_COLOR565(0xFF, 0xFF, 0x00),  ILI9341_COLOR565(0x00, 0x00, 0xFF), 2);

    st7735_rect(0, 0, 20, 20, ILI9341_COLOR565(0xFF, 0x00, 0x00));
    st7735_rect(0, 20, 20, 20, ILI9341_COLOR565(0x00, 0xFF, 0x00));
    st7735_rect(0, 40, 20, 20, ILI9341_COLOR565(0x00, 0x00, 0xFF));*/
    
    /*st7735_rect(0, 60, 20, 20, ILI9341_COLOR565(0x0F, 0x0F, 0x00));
    st7735_rect(0, 80, 20, 20, ILI9341_COLOR565(0x00, 0x0F, 0x0F));
    st7735_rect(20, 0, 20, 20, ILI9341_COLOR565(0x0F, 0x00, 0x0F));
    st7735_rect(40, 0, 20, 20, ILI9341_COLOR565(0x0F, 0x0F, 0x0F));
    st7735_rect(60, 0, 20, 20, COLOR_YELLOW);

    st7735_rect(80, 0, 20, 20, COLOR_GRAY);*/

//    st7735_draw_char(20, 20, 'V', COLOR_GREEN, COLOR_BLACK, 5);
//    st7735_draw_char(47, 20, 'E', COLOR_GREEN, COLOR_BLACK, 5);
//    st7735_draw_char(74, 20, 'R', COLOR_GREEN, COLOR_BLACK, 5);
//    st7735_draw_char(101, 20, 'A', COLOR_GREEN, COLOR_BLACK, 5);

//    st7735_draw_char(22, 60, 'T', COLOR_GREEN, COLOR_BLACK, 5);
//    st7735_draw_char(57, 60, 'H', COLOR_GREEN, COLOR_BLACK, 5);
//    st7735_draw_char(93, 60, 'E', COLOR_GREEN, COLOR_BLACK, 5);

//    st7735_draw_char(20, 100, 'B', COLOR_GREEN, COLOR_BLACK, 5);
//    st7735_draw_char(47, 100, 'E', COLOR_GREEN, COLOR_BLACK, 5);
//    st7735_draw_char(74, 100, 'S', COLOR_GREEN, COLOR_BLACK, 5);
//    st7735_draw_char(101, 100, 'T', COLOR_GREEN, COLOR_BLACK, 5);

//    st7735_draw_char(60, 60, 'Z', COLOR_GREEN, COLOR_BLACK, 1);

//    st7735_draw_char(80, 60, 'F', COLOR_GREEN, COLOR_BLACK, 5);

//    st7735_draw_string(80, 150, "PT", COLOR_GREEN, COLOR_BLACK, 5);

//    ST7735_COLOR565(0, 0, 0xFF)
//    uint16_t c = 0x0001;
//    uint16_t co2 = 900;
//    uint16_t hum = 0;
//    int temp = -50;
//    int co2 = 0;

//    char buff[100];

    setCO2(2500);

    while(1)
    {
//	if(hum == 100)
//	   hum = 0;
//	else
//	   hum += 10;
	   
//	if(temp == 50)
//	   temp = -50;
//	else
//	   temp += 10;
	   
//	if(co2 == 3000)
//	   co2 = 0;
//	else
//	   co2 += 100;
	   
	   
        vTaskDelay(1000/portTICK_RATE_MS);
        setHumiditi(humidity);
        setTerm(temp);
        setPa(pressure);
//        setCO2(co2);

        vTaskDelay(5000/portTICK_RATE_MS);

        //TODO add form

//        st7735_rect(50, 79, 20, 20, c);
//        ESP_LOGI("Display", "Color 0x%04X", c);
//        c = c << 1;
//        if(c == 0)
//            c = 1;

//        co2 += 100;
//        if(co2 == 2500)
//            co2 = 900;
//        setCO2(co2);
    }
}

void taskButton(void *p)
{
    (void)p;

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL | 0x01/*gpio0*/;
    //disable pull-down mode
    io_conf.pull_down_en = 1;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    gpio_set_direction(GPIO_INPUT_IO_1, GPIO_MODE_INPUT);
    gpio_pulldown_en(GPIO_INPUT_IO_1);
    gpio_pullup_dis(GPIO_INPUT_IO_1);

    gpio_set_direction(GPIO_INPUT_IO_3, GPIO_MODE_INPUT);
    gpio_pulldown_en(GPIO_INPUT_IO_3);
    gpio_pullup_dis(GPIO_INPUT_IO_3);

    bool oldBup = false, oldBleft = false, oldBright = false, oldBdown = false, oldBmenu = false, oldBback = false, oldB0 = false;
    bool Bup = false, Bleft = false, Bright = false, Bdown = false, Bmenu = false, Bback = false, B0 = false;

    while(1)
    {
        vTaskDelay(50/portTICK_RATE_MS);

        Bup = gpio_get_level(GPIO_UP);
        Bright = gpio_get_level(GPIO_RIGHT);
        Bleft = gpio_get_level(GPIO_LEFT);
        Bdown = gpio_get_level(GPIO_DOWN);
        Bmenu = gpio_get_level(GPIO_MENU);
        Bback = gpio_get_level(GPIO_BACK);

        B0 = gpio_get_level(0);

        if(oldBup != Bup)//-
        {
            oldBup = Bup;
            ESP_LOGI("BUTTON", "up if %s", Bup ? "UP" : "DOWN");
        }

        if(oldBleft != Bleft)//-
        {
            oldBleft = Bleft;
            ESP_LOGI("BUTTON", "left if %s", Bleft ? "UP" : "DOWN");
        }

        if(oldBright != Bright)
        {
            oldBright = Bright;
            ESP_LOGI("BUTTON", "right if %s", Bright ? "UP" : "DOWN");
        }

        if(oldBdown != Bdown)
        {
            oldBdown = Bdown;
            ESP_LOGI("BUTTON", "down if %s", Bdown ? "UP" : "DOWN");
        }

        if(oldBmenu != Bmenu)
        {
            oldBmenu = Bmenu;
            ESP_LOGI("BUTTON", "menu if %s", Bmenu ? "UP" : "DOWN");
        }

        if(oldBback != Bback)
        {
            oldBback = Bback;
            ESP_LOGI("BUTTON", "back if %s", Bback ? "UP" : "DOWN");
        }

        if(oldB0 != B0)
        {
            oldB0 = B0;
            ESP_LOGI("BUTTON", "B0 if %s", B0 ? "UP" : "DOWN");
        }
    }
}

uint16_t co2Val;
void task_co2(void *p)
{
    (void)p;

    co2_init();
    vTaskDelay(10000/portTICK_RATE_MS);

    while(1)
    {
        co2Val = co2_read();
        setCO2(co2Val);
        ESP_LOGI("CO2", "CO2 %d", co2Val);
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
    vTaskDelay(10000/portTICK_RATE_MS);
    ESP_LOGI("MQTT", "begin MQTT");

    (void)p;
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = /*MQTT_SERVER*/"mqtt://192.168.0.16:1883",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    while(1)
    {
        vTaskDelay(10000/portTICK_RATE_MS);
        char string[32];

        sprintf(string, "%4d", (int)temp);
        ESP_LOGI("MQTT", "publish temperature");
        esp_mqtt_client_publish(client, "/meteoCS/temp0", string, 4, 0, 0);

        sprintf(string, "%4d", (int)humidity);
        ESP_LOGI("MQTT", "publish humidity");
        esp_mqtt_client_publish(client, "/meteoCS/humidity0", string, 4, 0, 0);

        sprintf(string, "%4d", (int)pressure);
        ESP_LOGI("MQTT", "publish pressure");
        esp_mqtt_client_publish(client, "/meteoCS/pressure0", string, 4, 0, 0);

        sprintf(string, "%4d", (int)co2Val);
        ESP_LOGI("MQTT", "publish co2Val");
        esp_mqtt_client_publish(client, "/meteoCS/CO2", string, 4, 0, 0);
    }
}

static uint16_t pixelRead[10][10];

void app_main(void)
{
    s_wifi_event_group = xEventGroupCreate();
    initDisplay();



//    ESP_LOGI(TAG, "Read display");

//    read_picturte(30, 80, 10, 10, pixelRead);

//    ESP_LOGI(TAG, "Write display");
//    send_picturte(150, 100, 10, 10, pixelRead);
//    ESP_LOGI(TAG, "Write display OK");


//    xTaskCreate(openWeatherTask, "openWeathre_reque", 4096, NULL, 1, NULL);
    xTaskCreate(taskDisplay, "Display", 2048, NULL, 3, NULL);
    xTaskCreate(taskBME, "BME", 2048, NULL, 2, NULL);
//    xTaskCreate(taskButton, "Button", 2048, NULL, 2, NULL);
    xTaskCreate(task_co2, "co2", 2048, NULL, 2, NULL);
//    xTaskCreate(nRF24_task, "nrf", 2048, NULL, 2, NULL);
    xTaskCreate(task_MQTT, "MQTT", 2048, NULL, 2, NULL);





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
