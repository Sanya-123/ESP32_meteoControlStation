#include "uart2.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "gpioDEF.h"


void uart2_init()
{
    const int uart_num = UART_NUM_2;
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    // Configure UART parameters
    uart_driver_install(UART_NUM_2, 128 * 2, 0, 0, NULL, 0);
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    uart_set_pin(UART_NUM_2, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI("UART2", "ok init uart");
}

//--------------------------------------------------------------------------------------------------

uint8_t uart2_getChar(void)              //прием данных
{
    uint8_t data = 0;                       //переменная для данных

    uart_read_bytes(UART_NUM_2, &data, 1, 100);

    return data;
}

//--------------------------------------------------------------------------------------------------

void uart2_putChar(uint8_t c)            //вывод данных
{
    uart_write_bytes(UART_NUM_2, (const char *)(&c), 1);
    ESP_ERROR_CHECK(uart_wait_tx_done(UART_NUM_2, 10));
}

//--------------------------------------------------------------------------------------------------

void uart2_flush()
{
    uart_flush(UART_NUM_2);
}

