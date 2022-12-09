/*
 * nrf24l01.c
 *
 *  Created on: 1 авг. 2019 г.
 *      Author: dima
 */
#include "RF24.h"
//#include "tim.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DEFOULT_BUFF_NRF_SIZE   256

#define HIGH 1
#define LOW  0

rf24_bus_cfg_t hspi1;

bool p_variant; /** False for RF24L01 and true for RF24L01P */
uint8_t payload_size = 0; /**< Fixed size of payloads */
bool dynamic_payloads_enabled; /**< Whether dynamic payloads are enabled. */
uint8_t pipe0_reading_address[5] = {0,}; /**< Last address set on pipe 0 for reading. */
uint8_t addr_width = 0; /**< The address width to use - 3,4 or 5 bytes. */
uint8_t txDelay = 0;

void delay_us(uint16_t us)
{
//    htim21.Instance->CNT = 0;
//    while(htim21.Instance->CNT <= us);
    vTaskDelay(us > 1000 ? us/1000 : 1);
}

esp_err_t esp_transmiteResiveSPI(uint8_t *dataTx, uint16_t sizeTx, uint8_t *dataRx, uint16_t sizeRx, uint8_t *status)
{
    uint8_t buffRx[256], buffTx[256];

    spi_transaction_t t = {
//        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
        .length = (sizeTx + sizeRx)*8,
    };

    t.rx_buffer = buffRx;
    t.tx_buffer = buffTx;

    memset(buffTx, 0xFF, sizeTx + sizeRx);
    if(dataTx)
        memcpy(buffTx, dataTx, sizeTx);

    csn(LOW);
    //spi_device_polling_transmit ; spi_device_transmit
    esp_err_t ret = spi_device_transmit(hspi1.spi_dev, &t);
    csn(HIGH);
    if(ret == ESP_OK)
    {
        if(dataRx)
            memcpy(dataRx, &buffRx[sizeTx], sizeRx);
        if(status)
            *status = buffRx[0];
    }


    return ret;
}

void csn(uint8_t level)
{
//    HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, level);
    //delay_us(5);

//    gpio_set_level(hspi1.cs_io_num, level);
}

void ce(uint8_t level)
{
//    HAL_GPIO_WritePin(CE_GPIO_Port, CE_Pin, level);
    gpio_set_level(hspi1.ce_io_num, level);
}

uint8_t read_register(uint8_t reg)
{
    uint8_t addr = R_REGISTER | (REGISTER_MASK & reg);
//    uint8_t dt = 0;


    uint8_t rxData;
    esp_err_t ret = esp_transmiteResiveSPI(&addr, 1, &rxData, 1, NULL);

    if (ret != ESP_OK) {
        ESP_LOGW("NRF", "rf24_read_reg: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return 0;
    }

    return rxData;
}

uint8_t write_registerMy(uint8_t reg, const uint8_t* buf, uint8_t len)
{
    uint8_t status = 0;
    uint8_t addr = W_REGISTER | (REGISTER_MASK & reg);

    ESP_LOGI("nrf", "write_registerMy len=%d", (1 + len)*8);//TODO

    uint8_t txData[DEFOULT_BUFF_NRF_SIZE];
    txData[0] = addr;
    memcpy(&txData[1], buf, len);
    esp_err_t ret = esp_transmiteResiveSPI(txData, len + 1, NULL, 0, &status);

    if (ret != ESP_OK) {
        ESP_LOGW("NRF", "rf24_cmd: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return 0;
    }



    return status;
}

uint8_t write_register(uint8_t reg, uint8_t value)
{
    uint8_t status = 0;
    uint8_t addr = W_REGISTER | (REGISTER_MASK & reg);

    uint8_t txData[2];
    txData[0] = addr;
    txData[1] = value;
    esp_err_t ret = esp_transmiteResiveSPI(txData, 2, NULL, 0, &status);

    if (ret != ESP_OK) {
        ESP_LOGW("NRF", "rf24_cmd: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return 0;
    }

    return status;
}

uint8_t write_payload(const void* buf, uint8_t data_len, const uint8_t writeType)
{
    uint8_t status = 0;
    const uint8_t* current = (const uint8_t*)buf;
    uint8_t addr = writeType;

    data_len = rf24_min(data_len, payload_size);
    uint8_t blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;


    ESP_LOGI("nrf", "write_payload len=%d", (blank_len + data_len + 1)*8);//TODO
//    esp_err_t ret;
//    csn(LOW);
////    spi_transaction_t t = {
////        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
////        .length = (blank_len + data_len + 1)*8,
//////        .tx_buffer = &cmd,
//////        .rx_buffer = res
////    };
////    t.tx_data[0] = addr;
//////    t.tx_data[1] = value;
////    memcpy(&(t.tx_data[1]), current, data_len);

//////    ret = spi_device_polling_transmit(hspi1.spi_dev, &t);
////    ret = spi_device_transmit(hspi1.spi_dev, &t);

//    spi_transaction_t t = {
//        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
//        .length = (1 + 0)*8,
////        .tx_buffer = &cmd,
////        .rx_buffer = res
//    };
////    int lenP = (blank_len + data_len)*8;
//    t.tx_data[0] = addr;
//    ret = spi_device_transmit(hspi1.spi_dev, &t);
//    status = t.rx_data[0];

//    memcpy(&(t.tx_data[0]), current, 4);

//    t.length = 4*8;
//    ret = spi_device_transmit(hspi1.spi_dev, &t);

//    memcpy(&(t.tx_data[0]), current + 4, 4);

//    t.length = 4*8;
//    ret = spi_device_transmit(hspi1.spi_dev, &t);


//    csn(HIGH);

    uint8_t txData[DEFOULT_BUFF_NRF_SIZE];
    txData[0] = addr;
    memcpy(&txData[1], current, data_len);
    memset(&txData[1 + data_len], 0x00, blank_len);
    esp_err_t ret = esp_transmiteResiveSPI(txData, blank_len + data_len + 1, NULL, 0, &status);

    if (ret != ESP_OK) {
        ESP_LOGW("NRF", "rf24_cmd: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return 0;
    }

    return status;
}

uint8_t read_payload(void* buf, uint8_t data_len)
{
    uint8_t status = 0;
//    uint8_t* current = (uint8_t*)buf;

    if(data_len > payload_size)
    {
        data_len = payload_size;
    }

    uint8_t blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;

    uint8_t addr = R_RX_PAYLOAD;

    uint8_t rxData[DEFOULT_BUFF_NRF_SIZE];
    esp_err_t ret = esp_transmiteResiveSPI(&addr, 1, rxData, blank_len + data_len, &status);
    memcpy(buf, &rxData[0], data_len);

    if (ret != ESP_OK) {
        ESP_LOGW("NRF", "rf24_cmd: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return 0;
    }

    return status;
}

uint8_t flush_rx(void)
{
    return spiTrans(FLUSH_RX);
}

uint8_t flush_tx(void)
{
    return spiTrans(FLUSH_TX);
}

uint8_t spiTrans(uint8_t cmd)
{
    uint8_t status = 0;

    esp_err_t ret = esp_transmiteResiveSPI(&cmd, 1, NULL, 0, &status);

    if (ret != ESP_OK) {
        ESP_LOGW("NRF", "rf24_cmd: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return 0;
    }

    return status;
}

uint8_t get_status(void)
{
    return spiTrans(NOP);
}

void setChannel(uint8_t channel)
{
    write_register(RF_CH, channel);
}

uint8_t getChannel()
{
    return read_register(RF_CH);
}

void setPayloadSize(uint8_t size)
{
    payload_size = rf24_min(size, 32);
}

uint8_t getPayloadSize(void)
{
    return payload_size;
}

uint8_t NRF_Init(rf24_bus_cfg_t bus)
{

    ESP_LOGI("NRF", "rf24_init");

    esp_err_t ret = ESP_OK;

    // Initialize bus if needed.
    if (bus.init_host) {
        spi_bus_config_t spi_buscfg = {
            .mosi_io_num = bus.mosi_io_num,
            .miso_io_num = bus.miso_io_num,
            .sclk_io_num = bus.sclk_io_num,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1
        };

        ret = spi_bus_initialize(bus.spi_host, &spi_buscfg, 1);

        if (ret != ESP_OK) {
            ESP_LOGW("NRF", "rf24_init: spi_bus_initialize failed: %s", esp_err_to_name(ret));
//            goto cleanup;
        }
//        rf_dev->bus_initialized = true;
    }

    // Initialize device.
    spi_device_interface_config_t spi_devcfg={
        .clock_speed_hz = 8000000,
        .mode = 0,
        .spics_io_num = bus.cs_io_num/*-1*/,
        .queue_size = 8
    };

    ESP_LOGI("NRF", "rf24_init: spi_bus_add_device");
    spi_device_handle_t spi_dev;
    ret = spi_bus_add_device(bus.spi_host, &spi_devcfg, &spi_dev);

    if (ret != ESP_OK) {
        ESP_LOGW("NRF", "rf24_init: spi_bus_add_device failed: %s", esp_err_to_name(ret));
//        goto cleanup;
        return 0;
    }
//    rf_dev->spi_dev = spi_dev;

    // Initialize GPIO for CE pin.
    gpio_set_direction(bus.ce_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(bus.ce_io_num, 0);

//    gpio_config_t io_conf;
//    //disable interrupt
//    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
//    //set as output mode
//    io_conf.mode = GPIO_MODE_OUTPUT;
//    //bit mask of the pins that you want to set,e.g.GPIO18/19
//    io_conf.pin_bit_mask = (1ULL << bus.ce_io_num);
//    //disable pull-down mode
//    io_conf.pull_down_en = 0;
//    //disable pull-up mode
//    io_conf.pull_up_en = 0;
//    //configure GPIO with the given settings
//    gpio_config(&io_conf);

//    gpio_set_level(bus.ce_io_num, 0);
//    gpio_set_level(bus.cs_io_num, 1);

//    *handle = rf_dev;
    bus.spi_dev = spi_dev;
    hspi1 = bus;
//    return ret;


    uint8_t setup = 0;
    p_variant = false;
    payload_size = 8;
    dynamic_payloads_enabled = false;
    addr_width = 5;
    pipe0_reading_address[0] = 0;

    ce(LOW);
    csn(HIGH);
//    HAL_Delay(5);
    vTaskDelay(5/portTICK_RATE_MS);

    write_register(NRF_CONFIG, 0x0C); // Reset NRF_CONFIG and enable 16-bit CRC.
    setRetries(0, 0);
    setPALevel(RF24_PA_MAX); // Reset value is MAX

    if(setDataRate(RF24_250KBPS)) // check for connected module and if this is a p nRF24l01 variant
    {
        p_variant = true;
    }

    setup = read_register(RF_SETUP);
    setDataRate(RF24_250KBPS); // Then set the data rate to the slowest (and most reliable) speed supported by all hardware.

    // Disable dynamic payloads, to match dynamic_payloads_enabled setting - Reset value is 0
    toggle_features();
    write_register(FEATURE, 0);
    write_register(DYNPD, 0);
    dynamic_payloads_enabled = false;

    // Reset current status. Notice reset and flush is the last thing we do
    write_register(NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
    setChannel(19);
    flush_rx();
    flush_tx();
    powerUp(); //Power up by default when begin() is called
    write_register(NRF_CONFIG, (read_register(NRF_CONFIG)) & ~(1 << PRIM_RX));
    return (setup != 0 && setup != 0xff);
}

bool isChipConnected(void)
{
    uint8_t setup = read_register(SETUP_AW);

    if(setup >= 1 && setup <= 3)
    {
        return true;
    }

    return false;
}

void startListening(void)
{
    powerUp();

    write_register(NRF_CONFIG, read_register(NRF_CONFIG) | (1 << PRIM_RX));
    write_register(NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
    ce(HIGH);
    // Restore the pipe0 adddress, if exists
    if(pipe0_reading_address[0] > 0)
    {
        write_registerMy(RX_ADDR_P0, pipe0_reading_address, addr_width);
    }
    else
    {
        closeReadingPipe(0);
    }

    if(read_register(FEATURE) & (1 << EN_ACK_PAY))
    {
        flush_tx();
    }
}


static const uint8_t child_pipe_enable[] = {ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5};

void stopListening(void)
{
    ce(LOW);
    delay_us(txDelay);

    if(read_register(FEATURE) & (1 << EN_ACK_PAY))
    {
        delay_us(txDelay); //200
        flush_tx();
    }

    write_register(NRF_CONFIG, (read_register(NRF_CONFIG)) & ~(1 << PRIM_RX));
    write_register(EN_RXADDR, read_register(EN_RXADDR) | (1 << child_pipe_enable[0])); // Enable RX on pipe0
}

void powerDown(void)
{
    ce(LOW); // Guarantee CE is low on powerDown
    write_register(NRF_CONFIG, read_register(NRF_CONFIG) & ~(1 << PWR_UP));
}

//Power up now. Radio will not power down unless instructed by MCU for config changes etc.
void powerUp(void)
{
    uint8_t cfg = read_register(NRF_CONFIG);
    // if not powered up then power up and wait for the radio to initialize
    if(!(cfg & (1 << PWR_UP)))
    {
        write_register(NRF_CONFIG, cfg | (1 << PWR_UP));
//        HAL_Delay(5);
        vTaskDelay(5/portTICK_RATE_MS);
    }
}


//Similar to the previous write, clears the interrupt flags
bool writeData(const void* buf, uint8_t len)
{
    startFastWrite(buf, len, 0, 1);

    while(!(get_status() & ((1 << TX_DS) | (1 << MAX_RT))))
    {}

    ce(LOW);
    /*
        uint8_t status = write_register(NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

        if(status & (1 << MAX_RT))
        {
                flush_tx(); //Only going to be 1 packet int the FIFO at a time using this method, so just flush
                return 0;
        }
        */

    //TX OK 1 or 0
    return 1;
}


bool TX_Pack(const void* buf, uint8_t len)
{
    //uint8_t regval=0x00;

    //regval = read_register(NRF_CONFIG);

    //regval |= (1<<PWR_UP);
    //regval &= ~(1<<PRIM_RX);
    //write_register(NRF_CONFIG, regval);
    //delay_us(150);

//    const uint8_t* current = (const uint8_t*)buf;
    uint8_t addr = W_TX_PAYLOAD_NO_ACK;

    uint8_t txData[DEFOULT_BUFF_NRF_SIZE];
    txData[0] = addr;
    memcpy(&txData[1], buf, len);
    esp_err_t ret = esp_transmiteResiveSPI(txData, len + 1, NULL, 0, NULL);

    if (ret != ESP_OK) {
        ESP_LOGW("NRF", "rf24_cmd: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return 0;
    }

    ce(HIGH);
    delay_us(15);
    ce(LOW);
    return 1;
}



void startFastWrite(const void* buf, uint8_t len, const bool multicast, bool startTx)
{
    write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);

    if(startTx)
    {
        ce(HIGH);
    }
}

void maskIRQ(bool tx, bool fail, bool rx)
{
    uint8_t config = read_register(NRF_CONFIG);
    config &= ~(1 << MASK_MAX_RT | 1 << MASK_TX_DS | 1 << MASK_RX_DR); //clear the interrupt flags
    config |= fail << MASK_MAX_RT | tx << MASK_TX_DS | rx << MASK_RX_DR; // set the specified interrupt flags
    write_register(NRF_CONFIG, config);
}

uint8_t getDynamicPayloadSize(void)
{
    uint8_t result = 0, addr;
    addr = R_RX_PL_WID;

    uint8_t rxData;
    esp_err_t ret = esp_transmiteResiveSPI(&addr, 1, &rxData, 1, &result);

    if (ret != ESP_OK) {
        ESP_LOGW("NRF", "rf24_cmd: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return 0;
    }


    if(/*result*/rxData > 32)
    {
        flush_rx();
//        HAL_Delay(2);
        vTaskDelay(2/portTICK_RATE_MS);
        return 0;
    }

    return /*result*/rxData;
}

bool availableMy(void)
{
    return available(NULL);
}

bool available(uint8_t* pipe_num)
{
    if(!(read_register(FIFO_STATUS) & (1 << RX_EMPTY)))
    {
        if(pipe_num) // If the caller wants the pipe number, include that
        {
            uint8_t status = get_status();
            *pipe_num = (status >> RX_P_NO) & 0x07;
        }

        return 1;
    }

    return 0;
}

void readData(void* buf, uint8_t len)
{
    read_payload(buf, len);
    write_register(NRF_STATUS, (1 << RX_DR) | (1 << MAX_RT) | (1 << TX_DS));
}


uint8_t whatHappened(void)
{
    uint8_t status = write_register(NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
    /*uint8_t tx_ok = status & (1 << TX_DS);
        uint8_t tx_fail = status & (1 << MAX_RT);
        uint8_t rx_ready = status & (1 << RX_DR);*/
    return status;
}

void openWritingPipe(uint64_t value)
{
    write_registerMy(RX_ADDR_P0, (uint8_t*)&value, addr_width);
    write_registerMy(TX_ADDR, (uint8_t*)&value, addr_width);
    write_register(RX_PW_P0, payload_size);
}


static const uint8_t child_pipe[] = {RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};

static const uint8_t child_payload_size[] = {RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5};


void openReadingPipe(uint8_t child, uint64_t address)
{
    if(child == 0)
    {
        memcpy(pipe0_reading_address, &address, addr_width);
    }

    if(child <= 6)
    {
        // For pipes 2-5, only write the LSB
        if(child < 2)
            write_registerMy(child_pipe[child], (const uint8_t*)&address, addr_width);
        else
            write_registerMy(child_pipe[child], (const uint8_t*)&address, 1);

        write_register(child_payload_size[child], payload_size);
        write_register(EN_RXADDR, read_register(EN_RXADDR) | (1 << child_pipe_enable[child]));
    }
}

void setAddressWidth(uint8_t a_width)
{
    if(a_width -= 2)
    {
        write_register(SETUP_AW, a_width%4);
        addr_width = (a_width%4) + 2;
    }
    else
    {
        write_register(SETUP_AW, 0);
        addr_width = 2;
    }
}

void closeReadingPipe(uint8_t pipe)
{
    write_register(EN_RXADDR, read_register(EN_RXADDR) & ~(1 << child_pipe_enable[pipe]));
}

void toggle_features(void)
{
    uint8_t addr = ACTIVATE;

    uint8_t txData[2];
    txData[0] = addr;
    txData[1] = 0x73;
    esp_err_t ret = esp_transmiteResiveSPI(txData, 2, NULL, 0, NULL);

    if (ret != ESP_OK) {
        ESP_LOGW("NRF", "rf24_cmd: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return;
    }
}

void enableDynamicPayloads(void)
{
    write_register(FEATURE, read_register(FEATURE) | (1 << EN_DPL));
    write_register(DYNPD, read_register(DYNPD) | (1 << DPL_P5) | (1 << DPL_P4) | (1 << DPL_P3) | (1 << DPL_P2) | (1 << DPL_P1) | (1 << DPL_P0));
    dynamic_payloads_enabled = true;
}

void disableDynamicPayloads(void)
{
    write_register(FEATURE, 0);
    write_register(DYNPD, 0);
    dynamic_payloads_enabled = false;
}

void enableAckPayload(void)
{
    write_register(FEATURE, read_register(FEATURE) | (1 << EN_ACK_PAY) | (1 << EN_DPL));
    write_register(DYNPD, read_register(DYNPD) | (1 << DPL_P1) | (1 << DPL_P0));
    dynamic_payloads_enabled = true;
}

void enableDynamicAck(void)
{
    write_register(FEATURE, read_register(FEATURE) | (1 << EN_DYN_ACK));
}

void writeAckPayload(uint8_t pipe, const void* buf, uint8_t len)
{
    const uint8_t* current = (const uint8_t*)buf;
    uint8_t data_len = rf24_min(len, 32);
    uint8_t addr = W_ACK_PAYLOAD | (pipe & 0x07);

    ESP_LOGI("nrf", "writeAckPayload len=%d", (data_len + 1)*8);

    uint8_t txData[DEFOULT_BUFF_NRF_SIZE];
    txData[0] = addr;
    memcpy(&txData[1], current, data_len);
    esp_err_t ret = esp_transmiteResiveSPI(txData, data_len + 1, NULL, 0, NULL);

    if (ret != ESP_OK) {
        ESP_LOGW("NRF", "rf24_cmd: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return;
    }
}

bool isAckPayloadAvailable(void)
{
    return !(read_register(FIFO_STATUS) & (1 << RX_EMPTY));
}

bool isPVariant(void)
{
    return p_variant;
}

void setAutoAck(bool enable)
{
    if(enable)
        write_register(EN_AA, 0x3F);
    else
        write_register(EN_AA, 0);
}

void setAutoAckPipe(uint8_t pipe, bool enable)
{
    if(pipe <= 6)
    {
        uint8_t en_aa = read_register(EN_AA);

        if(enable)
        {
            en_aa |= (1 << pipe);
        }
        else
        {
            en_aa &= ~(1 << pipe);
        }

        write_register(EN_AA, en_aa);
    }
}

void setPALevel(uint8_t level)
{
    uint8_t setup = read_register(RF_SETUP) & 0xF8;

    if(level > 3) // If invalid level, go to max PA
    {
        level = (RF24_PA_MAX << 1) + 1;		// +1 to support the SI24R1 chip extra bit
    }
    else
    {
        level = (level << 1) + 1;	 		// Else set level as requested
    }

    write_register(RF_SETUP, setup |= level);	// Write it to the chip
}

uint8_t getPALevel(void)
{
    return (read_register(RF_SETUP) & ((1 << RF_PWR_LOW) | (1 << RF_PWR_HIGH))) >> 1;
}

bool setDataRate(rf24_datarate_e speed)
{
    bool result = false;
    uint8_t setup = read_register(RF_SETUP);
    setup &= ~((1 << RF_DR_LOW) | (1 << RF_DR_HIGH));
    txDelay = 85;

    if(speed == RF24_250KBPS)
    {
        setup |= (1 << RF_DR_LOW);
        txDelay = 155;
    }
    else
    {
        if(speed == RF24_2MBPS)
        {
            setup |= (1 << RF_DR_HIGH);
            txDelay = 65;
        }
    }

    write_register(RF_SETUP, setup);
    uint8_t ggg = read_register(RF_SETUP);

    if(ggg == setup)
    {
        result = true;
    }

    return result;
}

rf24_datarate_e getDataRate(void)
{
    rf24_datarate_e result ;
    uint8_t dr = read_register(RF_SETUP) & ((1 << RF_DR_LOW) | (1 << RF_DR_HIGH));

    // switch uses RAM (evil!)
    // Order matters in our case below
    if(dr == (1 << RF_DR_LOW))
    {
        result = RF24_250KBPS;
    }
    else if(dr == (1 << RF_DR_HIGH))
    {
        result = RF24_2MBPS;
    }
    else
    {
        result = RF24_1MBPS;
    }

    return result;
}

void setCRCLength(rf24_crclength_e length)
{
    uint8_t config = read_register(NRF_CONFIG) & ~((1 << CRCO) | (1 << EN_CRC));

    if(length == RF24_CRC_DISABLED)
    {
        // Do nothing, we turned it off above.
    }
    else if(length == RF24_CRC_8)
    {
        config |= (1 << EN_CRC);
    }
    else
    {
        config |= (1 << EN_CRC);
        config |= (1 << CRCO);
    }

    write_register(NRF_CONFIG, config);
}

rf24_crclength_e getCRCLength(void)
{
    rf24_crclength_e result = RF24_CRC_DISABLED;

    uint8_t config = read_register(NRF_CONFIG) & ((1 << CRCO) | (1 << EN_CRC));
    uint8_t AA = read_register(EN_AA);

    if(config & (1 << EN_CRC) || AA)
    {
        if(config & (1 << CRCO))
            result = RF24_CRC_16;
        else
            result = RF24_CRC_8;
    }

    return result;
}

void disableCRC(void)
{
    uint8_t disable = read_register(NRF_CONFIG) & ~(1 << EN_CRC);
    write_register(NRF_CONFIG, disable);
}

void setRetries(uint8_t delay, uint8_t count)
{
    write_register(SETUP_RETR, (delay&0xf)<<ARD | (count&0xf)<<ARC);
}
