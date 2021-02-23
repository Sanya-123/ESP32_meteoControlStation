#include "bme280.h"
#include "i2c.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// округлить до ближайшего целого для этого умножаем на 2, прибавляем 1, делим на 2
#define ROUNDING(a,b) ((2 * a / b + 1) >> 1)



#ifdef HW_V2 // TODO автопоиск
    #define BME280          0xEE // HW i2c address
#else
    #define BME280          0xEC // HW i2c address
#endif

// registers map
#define REG_HUM_LSB     0xFE
#define REG_HUM_MSB     0xFD
#define REG_TEMP_XLSB   0xFC
#define REG_TEMP_LSB    0xFB
#define REG_TEMP_MSB    0xFA
#define REG_PRESS_XLSB  0xF9
#define REG_PRESS_LSB   0xF8
#define REG_PRESS_MSB   0xF7
#define REG_CONFIG      0xF5
#define REG_CTRL_MEAS   0xF4
#define REG_STATUS      0xF3
#define REG_CTRL_HUM    0xF2
#define REG_RESET       0xE0
#define REG_ID          0xD0

#define TEMP_CALIB_DIG_T1_LSB   (0x88)
#define TEMP_CALIB_DIG_T1_MSB   (0x89)
#define TEMP_CALIB_DIG_T2_LSB   (0x8A)
#define TEMP_CALIB_DIG_T2_MSB   (0x8B)
#define TEMP_CALIB_DIG_T3_LSB   (0x8C)
#define TEMP_CALIB_DIG_T3_MSB   (0x8D)
#define PRESS_CALIB_DIG_P1_LSB  (0x8E)
#define PRESS_CALIB_DIG_P1_MSB  (0x8F)
#define PRESS_CALIB_DIG_P2_LSB  (0x90)
#define PRESS_CALIB_DIG_P2_MSB  (0x91)
#define PRESS_CALIB_DIG_P3_LSB  (0x92)
#define PRESS_CALIB_DIG_P3_MSB  (0x93)
#define PRESS_CALIB_DIG_P4_LSB  (0x94)
#define PRESS_CALIB_DIG_P4_MSB  (0x95)
#define PRESS_CALIB_DIG_P5_LSB  (0x96)
#define PRESS_CALIB_DIG_P5_MSB  (0x97)
#define PRESS_CALIB_DIG_P6_LSB  (0x98)
#define PRESS_CALIB_DIG_P6_MSB  (0x99)
#define PRESS_CALIB_DIG_P7_LSB  (0x9A)
#define PRESS_CALIB_DIG_P7_MSB  (0x9B)
#define PRESS_CALIB_DIG_P8_LSB  (0x9C)
#define PRESS_CALIB_DIG_P8_MSB  (0x9D)
#define PRESS_CALIB_DIG_P9_LSB  (0x9E)
#define PRESS_CALIB_DIG_P9_MSB  (0x9F)
#define HUM_CALIB_DIG_H1        (0xA1)
#define HUM_CALIB_DIG_H2_LSB    (0xE1)
#define HUM_CALIB_DIG_H2_MSB    (0xE2)
#define HUM_CALIB_DIG_H3        (0xE3)
#define HUM_CALIB_DIG_H4_MSB    (0xE4)
#define HUM_CALIB_DIG_H4_LSB    (0xE5)
#define HUM_CALIB_DIG_H5_MSB    (0xE6)
#define HUM_CALIB_DIG_H6        (0xE7)

#define	BME280_MASK_DIG_H4          (0x0F)
#define RESET_KEY                   0xB6 // write in REG_RESET for reset

//--------------------------------------------------------------------------------------------------

uint16_t dig_T1; // calibration T1 data
int16_t dig_T2;  // calibration T2 data
int16_t dig_T3;  // calibration T3 data
uint16_t dig_P1; // calibration P1 data
int16_t dig_P2;  // calibration P2 data
int16_t dig_P3;  // calibration P3 data
int16_t dig_P4;  // calibration P4 data
int16_t dig_P5;  // calibration P5 data
int16_t dig_P6;  // calibration P6 data
int16_t dig_P7;  // calibration P7 data
int16_t dig_P8;  // calibration P8 data
int16_t dig_P9;  // calibration P9 data
uint8_t dig_H1;  // calibration H1 data
int16_t dig_H2;  // calibration H2 data
uint8_t dig_H3;  // calibration H3 data
int16_t dig_H4;  // calibration H4 data
int16_t dig_H5;  // calibration H5 data
int8_t  dig_H6;  // calibration H6 data
int32_t t_fine;

#if 1 // USE INT (1) OR FLOAT (0)

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
int32_t BME280_compensate_T_int32(int32_t adc_T)
{
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
            ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and
// 8 fractional bits). Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
uint32_t BME280_compensate_P_int64(int32_t adc_P)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and
// 10 fractional bits). Output value of “47445” represents 47445/1024 = 46.333 %RH
uint32_t bme280_compensate_H_int32(int32_t adc_H)
{
    int32_t v_x1_u32r;
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return (uint32_t)(v_x1_u32r >> 12);
}

#else

// Returns temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC.
// t_fine carries fine temperature as global value
double BME280_compensate_T_double(int32_t adc_T)
{
    double var1, var2, T;
    var1 = (((double)adc_T) / 16384.0 - ((double)dig_T1) / 1024.0) * ((double)dig_T2);
    var2 = ((((double)adc_T) / 131072.0 - ((double)dig_T1) / 8192.0) * (((double)adc_T) / 131072.0 - ((double) dig_T1) / 8192.0)) * ((double)dig_T3);
    t_fine = (int32_t)(var1 + var2);
    T = (var1 + var2) / 5120.0;
    return T;
}
// Returns pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
double BME280_compensate_P_double(int32_t adc_P)
{
    double var1, var2, p;
    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
    var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);
    if (var1 == 0.0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576.0 - (double)adc_P;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)dig_P9) * p * p / 2147483648.0;
    var2 = p * ((double)dig_P8) / 32768.0;
    p = p + (var1 + var2 + ((double)dig_P7)) / 16.0;
    return p;
}

// Returns humidity in %rH as as double. Output value of “46.332” represents 46.332 %rH
double bme280_compensate_H_double(int32_t adc_H)
{
    double var_H;
    var_H = (((double)t_fine) - 76800.0);
    var_H = (adc_H - (((double)dig_H4) * 64.0 + ((double)dig_H5) / 16384.0 * var_H)) * (((double)dig_H2) / 65536.0 * (1.0 + ((double)dig_H6) / 67108864.0 * var_H * (1.0 + ((double)dig_H3) / 67108864.0 * var_H)));
    var_H = var_H * (1.0 - ((double)dig_H1) * var_H / 524288.0);
    if (var_H > 100.0)
    {
        var_H = 100.0;
    }
    else if (var_H < 0.0)
    {
        var_H = 0.0;
    }
    return var_H;
}

#endif

void bme280_waitForReady()
{
    while (i2c_readByte(BME280, REG_STATUS) & 0x09);
}

void bme280_init()
{
    i2c_init();

    i2c_writeByte(BME280, REG_RESET, RESET_KEY);                    // Reset
    vTaskDelay(1000);

    ESP_LOGI("BME280", "\n\tChip ID:0x%X\n", i2c_readByte(BME280, REG_ID));    // Read ID
    bme280_waitForReady();

    //  parameter | Register address |   bit
    //------------|------------------|----------------
    //  dig_T1    |  0x88 and 0x89   | from 0 : 7 to 8: 15
    //  dig_T2    |  0x8A and 0x8B   | from 0 : 7 to 8: 15
    //  dig_T3    |  0x8C and 0x8D   | from 0 : 7 to 8: 15
    //  dig_P1    |  0x8E and 0x8F   | from 0 : 7 to 8: 15
    //  dig_P2    |  0x90 and 0x91   | from 0 : 7 to 8: 15
    //  dig_P3    |  0x92 and 0x93   | from 0 : 7 to 8: 15
    //  dig_P4    |  0x94 and 0x95   | from 0 : 7 to 8: 15
    //  dig_P5    |  0x96 and 0x97   | from 0 : 7 to 8: 15
    //  dig_P6    |  0x98 and 0x99   | from 0 : 7 to 8: 15
    //  dig_P7    |  0x9A and 0x9B   | from 0 : 7 to 8: 15
    //  dig_P8    |  0x9C and 0x9D   | from 0 : 7 to 8: 15
    //  dig_P9    |  0x9E and 0x9F   | from 0 : 7 to 8: 15
    //  dig_H1    |         0xA1     | from 0 to 7
    //  dig_H2    |  0xE1 and 0xE2   | from 0 : 7 to 8: 15
    //  dig_H3    |         0xE3     | from 0 to 7
    //  dig_H4    |  0xE4 and 0xE5   | from 4 : 11 to 0: 3
    //  dig_H5    |  0xE5 and 0xE6   | from 0 : 3 to 4: 11
    //  dig_H6    |         0xE7     | from 0 to 7

    dig_T1 = (uint16_t)((((uint16_t)((uint8_t)i2c_readByte(BME280, TEMP_CALIB_DIG_T1_MSB))) << 8) | i2c_readByte(BME280, TEMP_CALIB_DIG_T1_LSB));
    dig_T2 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, TEMP_CALIB_DIG_T2_MSB))) << 8) | i2c_readByte(BME280, TEMP_CALIB_DIG_T2_LSB));
    dig_T3 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, TEMP_CALIB_DIG_T3_MSB))) << 8) | i2c_readByte(BME280, TEMP_CALIB_DIG_T3_LSB));
    dig_P1 = (uint16_t)((((uint16_t)((uint8_t)i2c_readByte(BME280, PRESS_CALIB_DIG_P1_MSB))) << 8) | i2c_readByte(BME280, PRESS_CALIB_DIG_P1_LSB));
    dig_P2 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, PRESS_CALIB_DIG_P2_MSB))) << 8) | i2c_readByte(BME280, PRESS_CALIB_DIG_P2_LSB));
    dig_P3 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, PRESS_CALIB_DIG_P3_MSB))) << 8) | i2c_readByte(BME280, PRESS_CALIB_DIG_P3_LSB));
    dig_P4 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, PRESS_CALIB_DIG_P4_MSB))) << 8) | i2c_readByte(BME280, PRESS_CALIB_DIG_P4_LSB));
    dig_P5 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, PRESS_CALIB_DIG_P5_MSB))) << 8) | i2c_readByte(BME280, PRESS_CALIB_DIG_P5_LSB));
    dig_P6 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, PRESS_CALIB_DIG_P6_MSB))) << 8) | i2c_readByte(BME280, PRESS_CALIB_DIG_P6_LSB));
    dig_P7 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, PRESS_CALIB_DIG_P7_MSB))) << 8) | i2c_readByte(BME280, PRESS_CALIB_DIG_P7_LSB));
    dig_P8 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, PRESS_CALIB_DIG_P8_MSB))) << 8) | i2c_readByte(BME280, PRESS_CALIB_DIG_P8_LSB));
    dig_P9 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, PRESS_CALIB_DIG_P9_MSB))) << 8) | i2c_readByte(BME280, PRESS_CALIB_DIG_P9_LSB));
    dig_H1 = i2c_readByte(BME280, HUM_CALIB_DIG_H1);
    dig_H2 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, HUM_CALIB_DIG_H2_MSB))) << 8) | i2c_readByte(BME280, HUM_CALIB_DIG_H2_LSB));
    dig_H3 = i2c_readByte(BME280, HUM_CALIB_DIG_H3);
    dig_H4 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, HUM_CALIB_DIG_H4_MSB))) << 4) | (((uint8_t)BME280_MASK_DIG_H4) & i2c_readByte(BME280, HUM_CALIB_DIG_H4_LSB)));
    dig_H5 = (int16_t)((((int16_t)((int8_t)i2c_readByte(BME280, HUM_CALIB_DIG_H5_MSB))) << 4) | (i2c_readByte(BME280, HUM_CALIB_DIG_H4_LSB) >> 4));
    dig_H6 = (int8_t)i2c_readByte(BME280, HUM_CALIB_DIG_H6);

    i2c_writeByte(BME280, REG_CONFIG, _BME280_CONFIG_SET);
    i2c_writeByte(BME280, REG_CTRL_HUM,  _BME280_CONTROL_HUM_SET);
    i2c_writeByte(BME280, REG_CTRL_MEAS, _BME280_CONTROL_MEAS_SET);

    ESP_LOGI("BME280", "\tT1: %d\n", dig_T1);
    ESP_LOGI("BME280", "\tT2: %d\n", dig_T2);
    ESP_LOGI("BME280", "\tT3: %d\n", dig_T3);
    ESP_LOGI("BME280", "\tP1: %d\n", dig_P1);
    ESP_LOGI("BME280", "\tP2: %d\n", dig_P2);
    ESP_LOGI("BME280", "\tP3: %d\n", dig_P3);
    ESP_LOGI("BME280", "\tP4: %d\n", dig_P4);
    ESP_LOGI("BME280", "\tP5: %d\n", dig_P5);
    ESP_LOGI("BME280", "\tP6: %d\n", dig_P6);
    ESP_LOGI("BME280", "\tP7: %d\n", dig_P7);
    ESP_LOGI("BME280", "\tP8: %d\n", dig_P8);
    ESP_LOGI("BME280", "\tP9: %d\n", dig_P9);
    ESP_LOGI("BME280", "\tH1: %d\n", dig_H1);
    ESP_LOGI("BME280", "\tH2: %d\n", dig_H2);
    ESP_LOGI("BME280", "\tH3: %d\n", dig_H3);
    ESP_LOGI("BME280", "\tH4: %d\n", dig_H4);
    ESP_LOGI("BME280", "\tH5: %d\n", dig_H5);
    ESP_LOGI("BME280", "\tH6: %d\n", dig_H6);
}

uint32_t bme280_readTemperature()
{
#ifdef _FORCED_MODE
     i2c_writeByte(BME280, REG_CTRL_MEAS, _BME280_CONTROL_MEAS_SET);
#endif

    int32_t data = i2c_readDWord(BME280, REG_TEMP_MSB) >> 12;
    data = BME280_compensate_T_int32(data);
    //ESP_LOGI("BME280", "Traw:%ld\n", data);
    return  ROUNDING(data, 10); // = temp*10
}

uint32_t bme280_readPressure()
{
    int32_t data = i2c_readDWord(BME280, REG_PRESS_MSB) >> 12;
    data = BME280_compensate_P_int64(data);
    //ESP_LOGI("BME280", "Praw:%ld\n", data);
    return  ROUNDING(data, 34131);  // == data /(256*133.322365)
}

uint32_t bme280_readHumidity()
{
    uint32_t data = i2c_readWord(BME280, REG_HUM_MSB);
    data = bme280_compensate_H_int32(data);
    //ESP_LOGI("BME280", "Hraw:%ld\n", data);
    return  ROUNDING(data, 1024); // = data / 1024
}
