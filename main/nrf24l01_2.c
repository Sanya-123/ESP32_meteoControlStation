

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
//#include "delay.h"
#include "nrf24l01.h"
#include <string.h>

#define _TOUINT(x) (*((uint8_t *)(&(x))))
#define NRF_MAX_PAYLOAD		32 // in bytes
#define NRF_ENABLE_CRC		1
#define NRF_CRC_WIDTH		1

gpio_num_t m_cs;
gpio_num_t m_ce;
spi_device_handle_t m_spi_device;
bool m_is_attached;
nrf_mode_t m_mode;

esp_err_t attach_to_spi_bus(spi_host_device_t spiBus);
uint8_t read_reg(uint8_t reg);
uint8_t write_reg(uint8_t reg, uint8_t value);
uint8_t read_bytes(uint8_t cmd, uint8_t *pBuff, uint8_t length);
uint8_t write_bytes(uint8_t cmd, uint8_t *pBuff, uint8_t length);
void write_tx_payload(uint8_t *pBuff, uint8_t length);
void read_rx_payload(uint8_t *pBuff, uint8_t length);
void ce_low(void);
void ce_high(void);
void cs_low(void);
void cs_high(void);

static inline uint8_t spi_transfer_byte(uint8_t byte, spi_device_handle_t device){
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));

	t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
	t.length = 8;

	t.tx_data[0] = byte;

	spi_device_transmit(device, &t);
	return t.rx_data[0];
}

esp_err_t attach_to_spi_bus(spi_host_device_t spiBus){
	spi_device_interface_config_t devcfg;
	memset(&devcfg, 0, sizeof(devcfg));

        devcfg.clock_speed_hz = 8 * 1000 * 1000;
	devcfg.mode = 0;
	devcfg.spics_io_num = -1;
	devcfg.queue_size = 8;
	devcfg.duty_cycle_pos = 128;

	esp_err_t err = spi_bus_add_device(spiBus, &devcfg, &m_spi_device);
	if(err == ESP_OK)
		m_is_attached = true;

	return err;
}

void nrf_init(gpio_num_t ce, gpio_num_t cs, gpio_num_t sck, gpio_num_t mosi, gpio_num_t miso, nrf_mode_t mode) {

//	assert(m_is_attached && "Call AttachToSpiBus first");
	assert(mode == nrf_tx_mode || mode == nrf_rx_mode);

//    spi_bus_config_t buscfg;
//	memset(&buscfg, 0, sizeof(buscfg));

	m_ce = ce;
    m_cs = cs;
//	buscfg.miso_io_num = miso;
//	buscfg.mosi_io_num = mosi;
//	buscfg.sclk_io_num = sck;
//	buscfg.quadhd_io_num = -1;
//	buscfg.quadwp_io_num = -1;
//	buscfg.max_transfer_sz = 4096;

//	esp_err_t err = spi_bus_initialize(VSPI_HOST, &buscfg, 1);
//	assert(err == ESP_OK);
	
        /* Attach to spi bus */
        attach_to_spi_bus(VSPI_HOST);

//    delay_rtos(10);
        vTaskDelay(10/portTICK_RATE_MS);

	ce_low();
	cs_high();

//	delay_rtos(5);
        vTaskDelay(5/portTICK_RATE_MS);

	/* Enable each pipes for receiving */
	write_reg(NRF_REG_ENRXADDR, 0x3F);

	/* Enabled auto acknowledgement */
	write_reg(NRF_REG_ENAA, 0x3F);

	/* Set payload width to each pipe to 32 bytes */
	for(int i = 0; i < 6; i++)
		write_reg(NRF_REG_RX_PW_P0, NRF_MAX_PAYLOAD);

	nrf_flush_tx();
	nrf_flush_rx();

	write_reg(NRF_REG_SETUP_ADDR_W, 0b11);
	nrf_set_auto_retransmission(10, 5);
	nrf_set_channel(77);
	nrf_set_data_rate_and_power(NRF_DATA_RATE_250k, NRF_POWER_0dBm);

	/* Setup config register */
	nrf_reg_config_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.en_crc = NRF_ENABLE_CRC;
	cfg.crco = NRF_CRC_WIDTH;
	cfg.pwr_up = mode == nrf_rx_mode;
	cfg.prim_rx = mode == nrf_rx_mode;
	write_reg(NRF_REG_CONFIG, _TOUINT(cfg));

	if(mode == nrf_rx_mode)
		ce_high();

	m_mode = mode;

	/* Start up delay */
//	delay_rtos(2);
        vTaskDelay(2/portTICK_RATE_MS);
}

uint8_t read_reg(uint8_t reg){
	uint8_t result;
	read_bytes(NRF_CMD_R_REGISTER | reg, &result, 1);
	return result;
}

uint8_t write_reg(uint8_t reg, uint8_t value){
	return write_bytes(NRF_CMD_W_REGISTER | reg, &value, 1);
}

uint8_t read_bytes(uint8_t cmd, uint8_t *pBuff, uint8_t length){
	cs_low();
	ets_delay_us(3);
	uint8_t status = spi_transfer_byte(cmd, m_spi_device);
	while(pBuff && length--){
		ets_delay_us(1);
		*pBuff++ = spi_transfer_byte(0xFF, m_spi_device);
	}
	ets_delay_us(3);
	cs_high();

	return status;
}

uint8_t write_bytes(uint8_t cmd, uint8_t *pBuff, uint8_t length){
	cs_low();
	ets_delay_us(3);
	uint8_t status = spi_transfer_byte(cmd, m_spi_device);
	while(pBuff && length--){
		ets_delay_us(1);
		spi_transfer_byte(*pBuff++, m_spi_device);
	}
	ets_delay_us(3);
	cs_high();

	return status;
}

int8_t nrf_send(uint8_t *pBuff, uint8_t length){
	assert(m_is_attached && "Call AttachToSpiBus first");
	assert(m_mode == nrf_tx_mode && "Tx mode must be set");

	ce_low();
	nrf_flush_tx();

	/* Setup as tx if needed and wakeup */
	uint8_t tmp = read_reg(NRF_REG_CONFIG);
	nrf_reg_config_t cfg = *(nrf_reg_config_t*)&tmp;
	cfg.pwr_up = 1;		/* Wakeup */
	cfg.prim_rx = 0;	/* Set tx mode if needed */
	write_reg(NRF_REG_CONFIG, _TOUINT(cfg));
//	delay_rtos(2); // wakeup delay
        vTaskDelay(2/portTICK_RATE_MS);

	nrf_write_tx_payload(pBuff, length);

	/* Set CE high for 10 us atleast */
	ce_high();
	ets_delay_us(30);
	ce_low();

	/* Wait for success or fail */
	nrf_reg_status_t status;
	do{
		status = nrf_get_status();
		ets_delay_us(20);
	} while(status.max_rt == 0 && status.tx_ds == 0);

	/* Read retries count */
	nrf_reg_observe_tx_t observe;
	memset(&observe, 0, sizeof(observe));
	tmp = read_reg(NRF_REG_OBSERVE_TX);
	observe = *(nrf_reg_observe_tx_t*)&tmp;
    
	/* Set result as -1 on fail, and equal to retries on success */
	uint8_t result;
	if(status.max_rt){
		result = -1;
	} else 
		result = observe.rt_count;

	/* Clear tx_ds and max_rt flags */
	status.rx_dr = 0; // we don't want to clear rx flag
	status.max_rt = 1;
	status.tx_ds = 1;
	write_reg(NRF_REG_STATUS, _TOUINT(status));

	/* Go to sleep mode */
	nrf_set_sleep_tx_mode();

	return result;
}

void nrf_write_tx_payload(uint8_t *pBuff, uint8_t length){
	assert(length <= NRF_MAX_PAYLOAD && "Payload width is limited to 32 bytes");

	uint8_t *tmp = (uint8_t*)malloc(NRF_MAX_PAYLOAD);
	memset(tmp, 1, NRF_MAX_PAYLOAD);
	memcpy(tmp, pBuff, length);

	write_bytes(NRF_CMD_W_TX_PAYLOAD, tmp, NRF_MAX_PAYLOAD);
	free(tmp);
}

void nrf_read_tx_payload(uint8_t *pBuff, uint8_t length){
	assert(length <= NRF_MAX_PAYLOAD && "Payload width is limited to 32 bytes");

	uint8_t *tmp = (uint8_t*)malloc(NRF_MAX_PAYLOAD);
	read_bytes(NRF_CMD_R_RX_PAYLOAD, tmp, NRF_MAX_PAYLOAD);
	memcpy(pBuff, tmp, length);
	free(tmp);
}

void nrf_read(uint8_t *pBuff, uint8_t length){
	assert(m_is_attached && "Call AttachToSpiBus first");
	assert(m_mode == nrf_rx_mode && "Rx mode must be set");

	/* Since we are in rx mode we already have CE high, power on and PRX bit */

	nrf_read_tx_payload(pBuff, length);

	/* Clear flags */
	nrf_reg_status_t status = nrf_get_status();
	status.rx_dr = 1;	// clear
	status.tx_ds = 0;	// don't clear
	status.max_rt = 0;	// don't clear
	write_reg(NRF_REG_STATUS, _TOUINT(status));
}

bool nrf_is_rx_data_available_pipe(uint8_t pipeNo){
	nrf_reg_status_t status = nrf_get_status();
	return status.rx_p_n == pipeNo;
}

bool nrf_is_rx_data_available(void){
	nrf_reg_status_t status = nrf_get_status();
	return status.rx_dr;
}

void nrf_set_sleep_tx_mode(){
	uint8_t tmp = read_reg(NRF_REG_CONFIG);
	nrf_reg_config_t cfg = *(nrf_reg_config_t*)&tmp;
	cfg.pwr_up = 0;
	cfg.prim_rx = 0;	/* Actually rx mode */
	write_reg(NRF_REG_CONFIG, _TOUINT(cfg));
	ce_low();

	m_mode = nrf_tx_mode;
}
void nrf_set_rx_mode(){
	uint8_t tmp = read_reg(NRF_REG_CONFIG);
	nrf_reg_config_t cfg = *(nrf_reg_config_t*)&tmp;
	cfg.pwr_up = 1;
	cfg.prim_rx = 1;	/* Actually rx mode */
	write_reg(NRF_REG_CONFIG, _TOUINT(cfg));
	ce_high();

	m_mode = nrf_rx_mode;
//	delay_rtos(2);
        vTaskDelay(2/portTICK_RATE_MS);
}

void nrf_flush_rx(){
	write_bytes(NRF_CMD_FLUSH_RX, NULL, 0);
}

void nrf_flush_tx(){
	write_bytes(NRF_CMD_FLUSH_TX, NULL, 0);
}

nrf_reg_status_t nrf_get_status(){
	uint8_t tmp = read_reg(NRF_REG_STATUS);
	nrf_reg_status_t status;
	memcpy((void*)&status, &tmp, sizeof(tmp));
	return status;
}

nrf_reg_fifo_status_t  nrf_get_fifo_status(){
	uint8_t tmp = read_reg(NRF_REG_FIFO_STATUS);
	nrf_reg_fifo_status_t status;
	memcpy((void*)&status, &tmp, sizeof(tmp));
	return status;
}

void nrf_set_auto_retransmission(uint8_t count, uint8_t delay){
	assert(count <= 15 && delay <= 15);

	nrf_reg_retries_t rt;
	rt.count = count;
	rt.delay = delay;
	write_reg(NRF_REG_SETUP_RETR, _TOUINT(rt));
}

void nrf_set_channel(uint8_t channel){
	assert(channel < 0x80 && "Note that ESP32 is little-endian chip");

	write_reg(NRF_REG_SETUP_CHANNEL, channel);
}

void nrf_set_data_rate(uint8_t dataRate){
	assert(dataRate < 3);

	uint8_t tmp = read_reg(NRF_REG_SETUP);
	nrf_reg_setup_t setup = *(nrf_reg_setup_t*)&tmp;
	setup.dr_high = (dataRate >> 1) & 1;
	setup.dr_low = dataRate & 1;

	write_reg(NRF_REG_SETUP, _TOUINT(setup));
}

void nrf_set_power(uint8_t power){
	assert(power <= 3);

	uint8_t tmp = read_reg(NRF_REG_SETUP);
	nrf_reg_setup_t setup = *(nrf_reg_setup_t*)&tmp;
	setup.power = power;
	write_reg(NRF_REG_SETUP, _TOUINT(setup));
}

void nrf_set_data_rate_and_power(uint8_t dataRate, uint8_t power){
	assert(dataRate < 3);
	assert(power <= 3);

	uint8_t tmp = read_reg(NRF_REG_SETUP);
	nrf_reg_setup_t setup = *(nrf_reg_setup_t*)&tmp;
	setup.dr_high = (dataRate >> 1) & 1;
	setup.dr_low = dataRate & 1;
	setup.power = power;
	write_reg(NRF_REG_SETUP, _TOUINT(setup));
}

void nrf_set_pipe_addr(uint8_t pipeNo, uint8_t *pAddr, uint8_t length){
	assert(pipeNo < 6);
	assert(length == (pipeNo < 2 ? 5 : 1));

	write_bytes((NRF_CMD_W_REGISTER | NRF_REG_RX_ADDR_P0) + pipeNo, pAddr, length);
}

void nrf_get_pipe_addr(uint8_t pipeNo, uint8_t *pAddr){
	assert(pipeNo < 6);

	uint8_t length = pipeNo < 2 ? 5 : 1;
	read_bytes((NRF_CMD_R_REGISTER | NRF_REG_RX_ADDR_P0) + pipeNo, pAddr, length);
}

void nrf_set_tx_addr(uint8_t *pAddr, uint8_t length){
	assert(length == 5);

	write_bytes(NRF_CMD_W_REGISTER | NRF_REG_TX_ADDR, pAddr, length);
}
void GetTxAddr(uint8_t *pAddr){
	write_bytes(NRF_CMD_R_REGISTER | NRF_REG_TX_ADDR, pAddr, 5);
}

void nrf_scan_channels(uint64_t *firstHalf, uint64_t *secondHalf){
	nrf_mode_t wasMode = m_mode;
	if(wasMode == nrf_tx_mode)
		nrf_set_rx_mode();

	for(int i = 0; i < 64; i++){
		nrf_set_channel(i);
		uint8_t value = read_reg(NRF_REG_RPD);
		*firstHalf |= (value << i);
	}

	for(int i = 0; i < 64; i++){
		nrf_set_channel(64+i);
		uint8_t value = read_reg(NRF_REG_RPD);
		*secondHalf |= (value << i);
	}

	if(wasMode == nrf_tx_mode)
		nrf_set_sleep_tx_mode();
}

void ce_low(void){
	gpio_set_direction(m_ce, GPIO_MODE_OUTPUT);
	gpio_set_level(m_ce, 0);
}
void ce_high(void){
	gpio_set_direction(m_ce, GPIO_MODE_OUTPUT);
	gpio_set_level(m_ce, 1);
}

void cs_low(void){
	gpio_set_direction(m_cs, GPIO_MODE_OUTPUT);
	gpio_set_level(m_cs, 0);
}
void cs_high(void){
	gpio_set_direction(m_cs, GPIO_MODE_OUTPUT);
	gpio_set_level(m_cs, 1);
}
