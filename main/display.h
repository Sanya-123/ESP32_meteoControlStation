#pragma once

// https://www.imgonline.com.ua/cut-photo-into-pieces-result.php

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "openWeather.h"
//#include "tft2/tftfunc.h"

#define TERM_MIN        -50
#define TERM_MAX        50

#define DEFOULE_COLOR       0xFFFF

enum stateDisplay{
    none,
    stateMainForm,          //CO2 humiditi temperature pressure
    stateWeather,           //some data from open weather
    stateWeatherTeat,       //some data from open weather
    stateExternWeather,     //data from NRF
//    stateImputWIFI,         //change wifi and input passworld wifi
    stateDipForm,           //Form that divired on range
    stateDip2Form,          //Form that divired on range

    stateTest1,
    stateTest2,
    stateTest3,
    stateTest4,
    stateTestTiger,
    stateTestNu0,
    stateTestNu1,
    stateTestNu2,
    stateTestNu3,
};


void initDisplay();
void drawMainForm();
void drawDipForm();
void drawDip2Form();
void drawWheather();
void drawWheatherTest();
void testMoon();
void drawunfillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, int16_t s, uint16_t color, uint16_t colorBG);
void setHumiditi(uint16_t val);
void setTerm(int16_t val);
void setPa(uint16_t val);
void setCO2(uint16_t val);
void setWheather(OpenWeather *wheather);

//void printOpenWheather();

void print_im1();
void print_im2();

void print_imTest1();
void print_imTest2();
void print_imTest3();
void print_imTest4();
void print_imTestTiger();
void print_imTestNy0();
void print_imTestNy1();
void print_imTestNy2();
void print_imTestNy3();

//TODO drawFormOpenWeather
