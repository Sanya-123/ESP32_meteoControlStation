#ifndef BME_I2C_H
#define BME_I2C_H

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "bme280_ok.h"

void i2c_master_init();
s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
void BME280_delay_msek(u32 msek);

#endif // BME_I2C_H
