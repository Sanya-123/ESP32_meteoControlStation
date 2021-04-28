/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once
#include <stdint.h>
#include "esp_err.h"


#define HUMIDITY_W 98
#define HUMIDITY_H 151

#define TEMPERATURE_W 57
#define TEMPERATURE_H 207

#define CO2_W 120
#define CO2_H 120

#define IM1_W   200
#define IM1_H   137

#define IM2_W   200
#define IM2_H   200

#define WIND_W  50
#define WIND_H  50

#define WEATHER_W  50
#define WEATHER_H  50

#define WEATHERB_W  100
#define WEATHERB_H  100

#define MOON_W  60
#define MOON_H  60

#define IM_TEST_1_W 160
#define IM_TEST_1_H 107
#define IM_TEST_2_W 160
#define IM_TEST_2_H 107

#define IM_TEST_3_W 160
#define IM_TEST_3_H 106
#define IM_TEST_4_W 160
#define IM_TEST_4_H 106

#define IM_TEST_TIGER_W 160
#define IM_TEST_TIGER_H 120

#define IM_TEST_NU0_W 160
#define IM_TEST_NU0_H 90
//#define IM_TEST_NU0_W 320
//#define IM_TEST_NU0_H 180

#define IM_TEST_NU1_W 160
#define IM_TEST_NU1_H 118

#define IM_TEST_NU2_W 138
#define IM_TEST_NU2_12_H 92
#define IM_TEST_NU2_34_H 91

#define IM_TEST_NU3_W 135
#define IM_TEST_NU3_H 120

typedef enum{
	imager,
	hum0,
	hum1,
	temp0,
	temp1,
    CO2,

    Im1,
    Im2,

    WindE,
    WindN,
    WindNE,
    WindNW,
    WindS,
    WindSE,
    WindSW,
    WindW,

    Wclear_day,
    Wclear_night,
    Wcloudy,
    Wdrizzle,
    Wfog,
    Whail,
    WlightRain,
    Wparly_cloudy_day,
    Wparly_cloudy_night,
    Wrain,
    Wsleet,
    Wsnow,
    Wthunderstorm,
    Wwind,
    Wunknown,

    B_Wclear_day,
    B_Wclear_night,
    B_Wcloudy,
    B_Wdrizzle,
    B_Wfog,
    B_Whail,
    B_WlightRain,
    B_Wparly_cloudy_day,
    B_Wparly_cloudy_night,
    B_Wrain,
    B_Wsleet,
    B_Wsnow,
    B_Wthunderstorm,
    B_Wwind,
    B_Wunknown,

    Moon_L0,
    Moon_L1,
    Moon_L2,
    Moon_L3,
    Moon_L4,
    Moon_L5,
    Moon_L6,
    Moon_L7,
    Moon_L8,
    Moon_L9,
    Moon_L10,
    Moon_L11,
    Moon_L12,
    Moon_L13,
    Moon_L14,
    Moon_L15,
    Moon_L16,
    Moon_L17,
    Moon_L18,
    Moon_L19,
    Moon_L20,
    Moon_L21,
    Moon_L22,
    Moon_L23,

    //test
    ImTest1,
    ImTest1_001,
    ImTest1_002,
    ImTest1_003,
    ImTest1_004,

    ImTest2,
    ImTest2_001,
    ImTest2_002,
    ImTest2_003,
    ImTest2_004,

    ImTest3,
    ImTest3_001,
    ImTest3_002,
    ImTest3_003,
    ImTest3_004,

    ImTest4,
    ImTest4_001,
    ImTest4_002,
    ImTest4_003,
    ImTest4_004,

    ImTestTiger,
    ImTestTiger_001,
    ImTestTiger_002,
    ImTestTiger_003,
    ImTestTiger_004,

    ImTestNy0,
    ImTestNy0_001,
    ImTestNy0_002,
    ImTestNy0_003,
    ImTestNy0_004,

    ImTestNy1,
    ImTestNy1_001,
    ImTestNy1_002,
    ImTestNy1_003,
    ImTestNy1_004,

    ImTestNy2,
    ImTestNy2_001,
    ImTestNy2_002,
    ImTestNy2_003,
    ImTestNy2_004,

    ImTestNy3,
    ImTestNy3_001,
    ImTestNy3_002,
    ImTestNy3_003,
    ImTestNy3_004,
}Image;


/**
 * @brief Decode the jpeg ``image.jpg`` embedded into the program file into pixel data.
 *
 * @param pixels A pointer to a pointer for an array of rows, which themselves are an array of pixels.
 *        Effectively, you can get the pixel data by doing ``decode_image(&myPixels); pixelval=myPixels[ypos][xpos];``
 * @return - ESP_ERR_NOT_SUPPORTED if image is malformed or a progressive jpeg file
 *         - ESP_ERR_NO_MEM if out of memory
 *         - ESP_OK on succesful decode
 */
esp_err_t decode_image(uint16_t ***pixels, Image im);

void free_image(uint16_t ***pixels, Image im);
