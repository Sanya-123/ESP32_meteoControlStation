#pragma once

// https://www.imgonline.com.ua/cut-photo-into-pieces-result.php

#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "openWeather.h"
//#include "tft2/tftfunc.h"

bool takeGuiSem(uint32_t timeout);
void giveGuiSem();
void guiTask(void *pvParameter);
