#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#include "tft2/tftfunc.h"

#define TERM_MIN        -50
#define TERM_MAX        50

#define DEFOULE_COLOR       0xFFFF

enum stateDisplay{
    none,
    stateMainForm,          //CO2 humiditi temperature pressure
    stateWeather,           //some data from open weather
    stateExternWeather,     //data from NRF
    stateImputWIFI,         //change wifi and input passworld wifi
};


void initDisplay();
void drawMainForm();
void drawunfillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, int16_t s, uint16_t color, uint16_t colorBG);
void setHumiditi(uint16_t val);
void setTerm(int16_t val);
void setPa(uint16_t val);
void setCO2(uint16_t val);

void print_im1();
void print_im2();

//TODO drawFormOpenWeather
