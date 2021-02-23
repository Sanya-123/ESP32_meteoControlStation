#ifndef _BME280_H
#define _BME280_H

#include <stdint.h>
#include <stdbool.h>

//--------------------------------------------------------------------------------------------------
// defines

// Oversampling Setting
#define  _OVERS_T1              0x20
#define  _OVERS_T2              0x40
#define  _OVERS_T4              0x60
#define  _OVERS_T8              0x80
#define  _OVERS_T16             0xA0

#define  _OVERS_P1              0x04
#define  _OVERS_P2              0x08
#define  _OVERS_P4              0x0C
#define  _OVERS_P8              0x10
#define  _OVERS_P16             0x14

#define  _OVERS_H1              0x01
#define  _OVERS_H2              0x02
#define  _OVERS_H4              0x03
#define  _OVERS_H8              0x04
#define  _OVERS_H16             0x05

//Power Modes. This lib uses NORMAL mode only!
//#define _SLEEP_MODE           0x00
#define _FORCED_MODE          0x01
//#define  _NORMAL_MODE           0x03

#define  _TSB_0_5               0x00
#define  _TSB_62_5              0x20
#define  _TSB_125               0x40
#define  _TSB_250               0x60
#define  _TSB_500               0x80
#define  _TSB_1000              0xA0
#define  _TSB_2000              0xC0
#define  _TSB_4000              0xE0

#define  _FILTER_OFF            0x00
#define  _FILTER_COEFFICIENT2   0x04
#define  _FILTER_COEFFICIENT4   0x08
#define  _FILTER_COEFFICIENT8   0x0C
#define  _FILTER_COEFFICIENT16  0x10

#define  _SPI_OFF               0x00
#define  _SPI_ON                0x01

//--------------------------------------------------------------------------------------------------

// настройки
//#define  _BME280_CONFIG_SET         (_TSB_4000 | _FILTER_COEFFICIENT16 | _SPI_OFF)
//#define  _BME280_CONTROL_MEAS_SET   (_OVERS_T16 | _OVERS_P16 | _NORMAL_MODE)
//#define  _BME280_CONTROL_HUM_SET    (_OVERS_H16)

#define  _BME280_CONFIG_SET         (_TSB_0_5 | _FILTER_OFF | _SPI_OFF)
#define  _BME280_CONTROL_MEAS_SET   (_OVERS_T1 | _OVERS_P1 | _FORCED_MODE)
#define  _BME280_CONTROL_HUM_SET    (_OVERS_H1)


void bme280_init();
uint32_t bme280_readTemperature();
uint32_t bme280_readPressure();
uint32_t bme280_readHumidity();


#endif // _BME280_H
