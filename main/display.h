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


void guiTask(void *pvParameter);
