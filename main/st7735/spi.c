#include "spi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include <string.h>
#include "gpioDEF.h"

spi_device_handle_t spi_dev;
bool transmitEnd = false;

//lcd_spi_pre_transfer_callback(spi_transaction_t *t) {
//    int dc = (int)t->user;
//    gpio_set_level(PIN_NUM_DC, dc);
//}

void spi_write(uint8_t *data, uint32_t size) //
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = size;
    t.tx_buffer = data;
    t.user = (void *)0;
    transmitEnd = false;
    spi_device_polling_transmit(spi_dev, &t);
//    spi_device_transmit
    while (transmitEnd == false) { }
}

void endTransmit(spi_transaction_t *t) {
    transmitEnd = true;
}

void spi_init()
{
    esp_err_t ret;
    // esp32 spi configuration
    spi_bus_config_t buscfg = {.miso_io_num = GPIO_SPI_MISO,
                               .mosi_io_num = GPIO_SPI_MOSI,
                               .sclk_io_num = GPIO_SPI_CLK,
                               .quadwp_io_num = -1,  // unused
                               .quadhd_io_num = -1,  // unused
                               .max_transfer_sz = 128 * 160 * 2};

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,       // Clock out at 10 MHz
        .mode = 0,                                // SPI mode 0
        .spics_io_num = -1,               // CS pin
        .queue_size = 7,                          // We want to be able to queue 7 transactions at a time
        .pre_cb = endTransmit,  // Specify pre-transfer callback to handle D/C line
    };

    spi_host_device_t spi_host = VSPI_HOST;
//#ifdef CONFIG_ST7735_HOST_HSPI
//    spi_host = HSPI_HOST;
//#endif

    // Initialize the SPI bus
    ret = spi_bus_initialize(spi_host, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(spi_host, &devcfg, &spi_dev);
    ESP_ERROR_CHECK(ret);
}

