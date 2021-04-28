#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "decode_image.h"

//typedef struct { // For current Day and Day 1, 2, 3, etc
//  String   Dt;
//  String   Period;
//  String   Icon;
//  String   Trend;
//  String   Main0;
//  String   Forecast0;
//  String   Forecast1;
//  String   Forecast2;
//  String   Description;
//  String   Time;
//  String   Country;
//  float    lat;
//  float    lon;
//  float    Temperature;
//  float    Humidity;
//  float    High;
//  float    Low;
//  float    Winddir;
//  float    Windspeed;
//  float    Rainfall;
//  float    Snowfall;
//  float    Pop;
//  float    Pressure;
//  int      Cloudcover;
//  int      Visibility;
//  int      Sunrise;
//  int      Sunset;
//  int      Timezone;
//} Forecast_record_type;

typedef struct
{
    //weather
    char weatherIcon[10];
    int code;
    char weatherName[40];
    //main
    float temp;//note from kelvine to C
    float feels_like;
    float temp_min;//note from kelvine to C
    float temp_max;//note from kelvine to C
    float pressure;
    float humidity;
//    float sea_level;
//    float grnd_level;
    //visibility
    int visibility;
    //wind
    int wind_speed;
    int wind_deg;
    //clouds
    int clouds;
    //sys
    int sunrise;
    int sunset;
    float phaseMoon;
}OpenWeather;

void initOpenWeather();
int askWeather(OpenWeather *veather);
//int askWeatherDayly(OpenWeather *veather);
int askWeatherOneCall(OpenWeather *current, OpenWeather *dayly, int days, OpenWeather *hourly, int houres);
void printOpenWeather(OpenWeather weather);
Image getImageWheather(OpenWeather weather, bool big);
Image getWind(int deg);

