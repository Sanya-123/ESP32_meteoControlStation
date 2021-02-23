

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include "nrf24l01_hal.h"

typedef enum{
	nrf_mode_none,
	nrf_rx_mode,
	nrf_tx_mode
} nrf_mode_t;

void nrf_init(gpio_num_t ce, gpio_num_t cs, gpio_num_t sck, gpio_num_t mosi, gpio_num_t miso, nrf_mode_t mode);
int8_t nrf_send(uint8_t *pBuff, uint8_t length);
void nrf_read(uint8_t *pBuff, uint8_t length);
void nrf_set_sleep_tx_mode(void);
void nrf_set_rx_mode(void);
void nrf_flush_rx(void);
void nrf_flush_tx(void);
nrf_reg_status_t nrf_get_status(void);
nrf_reg_fifo_status_t nrf_get_fifo_status(void);
bool nrf_is_rx_data_available_pipe(uint8_t pipeNo);
bool nrf_is_rx_data_available(void);
void nrf_set_auto_retransmission(uint8_t count, uint8_t delay);
void nrf_set_channel(uint8_t channel);
void nrf_set_data_rate(uint8_t dataRate);
void nrf_set_power(uint8_t power);
void nrf_set_data_rate_and_power(uint8_t dataRate, uint8_t power);
void nrf_set_pipe_addr(uint8_t pipeNo, uint8_t *pAddr, uint8_t length);
void nrf_get_pipe_addr(uint8_t pipeNo, uint8_t *pAddr);
void nrf_set_tx_addr(uint8_t *pAddr, uint8_t length);
void nrf_get_tx_addr(uint8_t *pAddr);
void nrf_scan_channels(uint64_t *firstHalf, uint64_t *secondHalf);
void nrf_write_tx_payload(uint8_t *pBuff, uint8_t length);
void nrf_read_tx_payload(uint8_t *pBuff, uint8_t length);