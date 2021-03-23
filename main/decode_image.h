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

typedef enum{
	imager,
	hum0,
	hum1,
	temp0,
	temp1,
    CO2,

    Im1,
    Im2
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
