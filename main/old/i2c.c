/* i2c - Example

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   See README.md file to get detailed usage of this example.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include "i2c.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

extern const char *TAG;



/**
 * @brief test code to read esp-i2c-slave
 *        We need to fill the buffer of esp slave device, then master can read them out.
 *
 * _______________________________________________________________________________________
 * | start | slave_addr + rd_bit +ack | read n-1 bytes + ack | read 1 byte + nack | stop |
 * --------|--------------------------|----------------------|--------------------|------|
 *
 */
//esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t *data_rd, size_t size)
//{
//    if (size == 0) {
//        return ESP_OK;
//    }
//    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//    i2c_master_start(cmd);
//    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
//    if (size > 1) {
//        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
//    }
//    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
//    i2c_master_stop(cmd);
//    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
//    i2c_cmd_link_delete(cmd);
//    return ret;
//}

/**
 * @brief Test code to write esp-i2c-slave
 *        Master device write data to slave(both esp32),
 *        the data will be stored in slave buffer.
 *        We can read them out from slave buffer.
 *
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|
 *
 */
//esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t addres, uint8_t *data_wr, size_t size)
//{
//    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//    i2c_master_start(cmd);
//    i2c_master_write_byte(cmd, (addres << 1) | WRITE_BIT, ACK_CHECK_EN);
//    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
//    i2c_master_stop(cmd);
//    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
//    i2c_cmd_link_delete(cmd);
//    return ret;
//}

/**
 * @brief i2c master initialization
 */
esp_err_t i2c_master_init(void)
{
    i2c_port_t i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void i2c_init()
{
    i2c_master_init();
}

i2c_cmd_handle_t cmd_handl;

#define DO_BEGIN_I2C    do{ cmd_handl = i2c_cmd_link_create(); \
                            i2c_master_start(cmd_handl); \
                            i2c_master_write_byte(cmd_handl, hwAddress | WRITE_BIT, ACK_CHECK_EN); \
                            i2c_master_write_byte(cmd_handl, regAddress, ACK_CHECK_EN);\
                            }while(0);

#define DO_END_I2C      do{ i2c_master_stop(cmd_handl);\
                            i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handl, 1000 / portTICK_RATE_MS);\
                            i2c_cmd_link_delete(cmd_handl);}while(0);

void i2c_writeByte(uint8_t hwAddress, uint8_t regAddress, uint8_t data)
{
//    cmd_handl = i2c_cmd_link_create();
//    i2c_master_start(cmd);
//    i2c_master_write_byte(cmd, hwAddress | WRITE_BIT, ACK_CHECK_EN);
//    i2c_master_write_byte(cmd, regAddress, ACK_CHECK_EN);
    DO_BEGIN_I2C
    i2c_master_write_byte(cmd_handl, data, ACK_CHECK_EN);
    DO_END_I2C
//    i2c_master_stop(cmd);
//    /*esp_err_t ret = */i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
//    i2c_cmd_link_delete(cmd);
}

void i2c_writeWord(uint8_t hwAddress, uint8_t regAddress, uint16_t data)
{
    DO_BEGIN_I2C
    i2c_master_write_byte(cmd_handl, (uint8_t)(data >> 8), ACK_CHECK_EN);
    i2c_master_write_byte(cmd_handl, (uint8_t)data, ACK_CHECK_EN);
    DO_END_I2C
}

void i2c_writeDWord(uint8_t hwAddress, uint8_t regAddress, uint32_t data)
{
    DO_BEGIN_I2C
    i2c_master_write_byte(cmd_handl, (uint8_t)(data >> 24), ACK_CHECK_EN);
    i2c_master_write_byte(cmd_handl, (uint8_t)(data >> 16), ACK_CHECK_EN);
    i2c_master_write_byte(cmd_handl, (uint8_t)(data >> 8), ACK_CHECK_EN);
    i2c_master_write_byte(cmd_handl, (uint8_t)data, ACK_CHECK_EN);
    DO_END_I2C
}

uint8_t i2c_readByte(uint8_t hwAddress, uint8_t regAddress)
{
    uint8_t data;
    DO_BEGIN_I2C
    i2c_master_start(cmd_handl);
    i2c_master_write_byte(cmd_handl, hwAddress | READ_BIT, ACK_CHECK_EN);//rbegin read data
    i2c_master_read_byte(cmd_handl, &data, NACK_VAL);
    DO_END_I2C
    return data;
}

uint16_t i2c_readWord(uint8_t hwAddress, uint8_t regAddress)
{
    uint8_t data[2];
    DO_BEGIN_I2C
    i2c_master_start(cmd_handl);
    i2c_master_write_byte(cmd_handl, hwAddress | READ_BIT, ACK_CHECK_EN);//rbegin read data
    i2c_master_read_byte(cmd_handl, &data[0], ACK_VAL);
    i2c_master_read_byte(cmd_handl, &data[1], NACK_VAL);
    DO_END_I2C
    return (((uint16_t)data[0]) << 8) | ((uint16_t)data[1]);
}

uint32_t i2c_readDWord(uint8_t hwAddress, uint8_t regAddress)
{
    uint8_t data[4];
    DO_BEGIN_I2C
    i2c_master_start(cmd_handl);
    i2c_master_write_byte(cmd_handl, hwAddress | READ_BIT, ACK_CHECK_EN);//rbegin read data
    i2c_master_read_byte(cmd_handl, &data[0], ACK_VAL);
    i2c_master_read_byte(cmd_handl, &data[1], ACK_VAL);
    i2c_master_read_byte(cmd_handl, &data[2], ACK_VAL);
    i2c_master_read_byte(cmd_handl, &data[3], NACK_VAL);
    DO_END_I2C
    return (((uint32_t)data[0]) << 24) | (((uint32_t)data[1]) << 16) | (((uint32_t)data[2]) << 8) | ((uint32_t)data[3]);
}


