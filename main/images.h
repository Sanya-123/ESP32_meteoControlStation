#ifndef IMAGES_H_
#define IMAGES_H_

#include "lvgl.h"


struct WEATHER
{
    char * weathername;
    char * filename;
};

struct LV_WEATHER
{
    char * weathername;
    const lv_img_dsc_t * img_dsc;
};

extern const struct LV_WEATHER lv_weather[40];

#endif
