#ifndef IMAGES_H_
#define IMAGES_H_

#include "lvgl.h"

#define WETAHER_IMAGE_SIZES     40


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

extern const struct LV_WEATHER lv_weather[WETAHER_IMAGE_SIZES];

#endif
