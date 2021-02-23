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
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define DATA_LENGTH 64                  /*!< Data buffer length of test buffer */
#define RW_TEST_LENGTH 16               /*!< Data length for r/w test, [0,DATA_LENGTH] */
#define DELAY_TIME_BETWEEN_ITEMS_MS 1000 /*!< delay time between different test items */


#define I2C_MASTER_SCL_IO /*CONFIG_I2C_MASTER_SCL*/22               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO /*CONFIG_I2C_MASTER_SDA*/21               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(/*CONFIG_I2C_MASTER_PORT_NUM*/0) /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ /*CONFIG_I2C_MASTER_FREQUENCY*/100000        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */


#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

void i2c_init();
void i2c_writeByte(uint8_t hwAddress, uint8_t regAddress, uint8_t data);
void i2c_writeWord(uint8_t hwAddress, uint8_t regAddress, uint16_t data);
void i2c_writeDWord(uint8_t hwAddress, uint8_t regAddress, uint32_t data);
uint8_t i2c_readByte(uint8_t hwAddress, uint8_t regAddress);
uint16_t i2c_readWord(uint8_t hwAddress, uint8_t regAddress);
uint32_t i2c_readDWord(uint8_t hwAddress, uint8_t regAddress);

