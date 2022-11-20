/* SPI Master example: jpeg decoder.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
The image used for the effect on the LCD in the SPI master example is stored in flash
as a jpeg file. This file contains the decode_image routine, which uses the tiny JPEG
decoder library to decode this JPEG into a format that can be sent to the display.

Keep in mind that the decoder library cannot handle progressive files (will give
``Image decoder: jd_prepare failed (8)`` as an error) so make sure to save in the correct
format if you want to use a different image file.
*/

#include "decode_image.h"
#include "tjpgd.h"
#include "esp_log.h"
#include <string.h>

//Reference the binary-included jpeg file
//extern const uint8_t image_jpg_start[] asm("_binary_imager_jpg_start");
////extern const uint8_t image_jpg_end[] asm("_binary_imager_jpg_end");

//extern const uint8_t hum0_jpg_start[] asm("_binary_humidity0_jpg_start");
////extern const uint8_t hum0_jpg_end[] asm("_binary_humidity0_jpg_end");

//extern const uint8_t hum1_jpg_start[] asm("_binary_humidity1_jpg_start");
////extern const uint8_t hum1_jpg_end[] asm("_binary_humidity1_jpg_end");

//extern const uint8_t temp0_jpg_start[] asm("_binary_temperature0_jpg_start");
////extern const uint8_t temp0_jpg_end[] asm("_binary_temperature0_jpg_end");

//extern const uint8_t temp1_jpg_start[] asm("_binary_temperature1_jpg_start");
////extern const uint8_t temp1_jpg_end[] asm("_binary_temperature1_jpg_end");

//extern const uint8_t co2_jpg_start[] asm("_binary_co2_jpg_start");
////extern const uint8_t co2_jpg_end[] asm("_binary_co2_jpg_end");

//extern const uint8_t im1_jpg_start[] asm("_binary_im1_jpg_start");
////extern const uint8_t im1_jpg_end[] asm("_binary_im1_jpg_end");

//extern const uint8_t im2_jpg_start[] asm("_binary_im2_jpg_start");
////extern const uint8_t im2_jpg_end[] asm("_binary_im2_jpg_end");

//wind
extern const uint8_t windE_jpg_start[] asm("_binary_E_jpg_start");
extern const uint8_t windN_jpg_start[] asm("_binary_N_jpg_start");
extern const uint8_t windNE_jpg_start[] asm("_binary_NE_jpg_start");
extern const uint8_t windNW_jpg_start[] asm("_binary_NW_jpg_start");
extern const uint8_t windS_jpg_start[] asm("_binary_S_jpg_start");
extern const uint8_t windSE_jpg_start[] asm("_binary_SE_jpg_start");
extern const uint8_t windSW_jpg_start[] asm("_binary_SW_jpg_start");
extern const uint8_t windW_jpg_start[] asm("_binary_W_jpg_start");

//wheather
extern const uint8_t w_clearDay_jpg_start[] asm("_binary_clear_day_jpg_start");
extern const uint8_t w_clearNight_jpg_start[] asm("_binary_clear_night_jpg_start");
extern const uint8_t w_cludly_jpg_start[] asm("_binary_cloudy_jpg_start");
extern const uint8_t w_drizzle_jpg_start[] asm("_binary_drizzle_jpg_start");
extern const uint8_t w_fog_jpg_start[] asm("_binary_fog_jpg_start");
extern const uint8_t w_hail_jpg_start[] asm("_binary_hail_jpg_start");
extern const uint8_t w_lightRain_jpg_start[] asm("_binary_lightRain_jpg_start");
extern const uint8_t w_partly_cloudy_day_jpg_start[] asm("_binary_partly_cloudy_day_jpg_start");
extern const uint8_t w_partly_cloudy_night_jpg_start[] asm("_binary_partly_cloudy_night_jpg_start");
extern const uint8_t w_rain_jpg_start[] asm("_binary_rain_jpg_start");
extern const uint8_t w_sleet_jpg_start[] asm("_binary_sleet_jpg_start");
extern const uint8_t w_snow_jpg_start[] asm("_binary_snow_jpg_start");
extern const uint8_t w_thunderstorm_jpg_start[] asm("_binary_thunderstorm_jpg_start");
extern const uint8_t w_wind_jpg_start[] asm("_binary_wind_jpg_start");
extern const uint8_t w_unknown_jpg_start[] asm("_binary_unknown_jpg_start");

extern const uint8_t b_w_clearDay_jpg_start[] asm("_binary_B_clear_day_jpg_start");
extern const uint8_t b_w_clearNight_jpg_start[] asm("_binary_B_clear_night_jpg_start");
extern const uint8_t b_w_cludly_jpg_start[] asm("_binary_B_cloudy_jpg_start");
extern const uint8_t b_w_drizzle_jpg_start[] asm("_binary_B_drizzle_jpg_start");
extern const uint8_t b_w_fog_jpg_start[] asm("_binary_B_fog_jpg_start");
extern const uint8_t b_w_hail_jpg_start[] asm("_binary_B_hail_jpg_start");
extern const uint8_t b_w_lightRain_jpg_start[] asm("_binary_B_lightRain_jpg_start");
extern const uint8_t b_w_partly_cloudy_day_jpg_start[] asm("_binary_B_partly_cloudy_day_jpg_start");
extern const uint8_t b_w_partly_cloudy_night_jpg_start[] asm("_binary_B_partly_cloudy_night_jpg_start");
extern const uint8_t b_w_rain_jpg_start[] asm("_binary_B_rain_jpg_start");
extern const uint8_t b_w_sleet_jpg_start[] asm("_binary_B_sleet_jpg_start");
extern const uint8_t b_w_snow_jpg_start[] asm("_binary_B_snow_jpg_start");
extern const uint8_t b_w_thunderstorm_jpg_start[] asm("_binary_B_thunderstorm_jpg_start");
extern const uint8_t b_w_wind_jpg_start[] asm("_binary_B_wind_jpg_start");
extern const uint8_t b_w_unknown_jpg_start[] asm("_binary_B_unknown_jpg_start");

//moon
extern const uint8_t moonphase_L0_jpg_start[] asm("_binary_moonphase_L0_jpg_start");
extern const uint8_t moonphase_L1_jpg_start[] asm("_binary_moonphase_L1_jpg_start");
extern const uint8_t moonphase_L2_jpg_start[] asm("_binary_moonphase_L2_jpg_start");
extern const uint8_t moonphase_L3_jpg_start[] asm("_binary_moonphase_L3_jpg_start");
extern const uint8_t moonphase_L4_jpg_start[] asm("_binary_moonphase_L4_jpg_start");
extern const uint8_t moonphase_L5_jpg_start[] asm("_binary_moonphase_L5_jpg_start");
extern const uint8_t moonphase_L6_jpg_start[] asm("_binary_moonphase_L6_jpg_start");
extern const uint8_t moonphase_L7_jpg_start[] asm("_binary_moonphase_L7_jpg_start");
extern const uint8_t moonphase_L8_jpg_start[] asm("_binary_moonphase_L8_jpg_start");
extern const uint8_t moonphase_L9_jpg_start[] asm("_binary_moonphase_L9_jpg_start");
extern const uint8_t moonphase_L10_jpg_start[] asm("_binary_moonphase_L10_jpg_start");
extern const uint8_t moonphase_L11_jpg_start[] asm("_binary_moonphase_L11_jpg_start");
extern const uint8_t moonphase_L12_jpg_start[] asm("_binary_moonphase_L12_jpg_start");
extern const uint8_t moonphase_L13_jpg_start[] asm("_binary_moonphase_L13_jpg_start");
extern const uint8_t moonphase_L14_jpg_start[] asm("_binary_moonphase_L14_jpg_start");
extern const uint8_t moonphase_L15_jpg_start[] asm("_binary_moonphase_L15_jpg_start");
extern const uint8_t moonphase_L16_jpg_start[] asm("_binary_moonphase_L16_jpg_start");
extern const uint8_t moonphase_L17_jpg_start[] asm("_binary_moonphase_L17_jpg_start");
extern const uint8_t moonphase_L18_jpg_start[] asm("_binary_moonphase_L18_jpg_start");
extern const uint8_t moonphase_L19_jpg_start[] asm("_binary_moonphase_L19_jpg_start");
extern const uint8_t moonphase_L20_jpg_start[] asm("_binary_moonphase_L20_jpg_start");
extern const uint8_t moonphase_L21_jpg_start[] asm("_binary_moonphase_L21_jpg_start");
extern const uint8_t moonphase_L22_jpg_start[] asm("_binary_moonphase_L22_jpg_start");
extern const uint8_t moonphase_L23_jpg_start[] asm("_binary_moonphase_L23_jpg_start");

//test
extern const uint8_t test1_001_jpg_start[] asm("_binary_test1_001_jpg_start");
extern const uint8_t test1_002_jpg_start[] asm("_binary_test1_002_jpg_start");
extern const uint8_t test1_003_jpg_start[] asm("_binary_test1_003_jpg_start");
extern const uint8_t test1_004_jpg_start[] asm("_binary_test1_004_jpg_start");
extern const uint8_t test2_001_jpg_start[] asm("_binary_test2_001_jpg_start");
extern const uint8_t test2_002_jpg_start[] asm("_binary_test2_002_jpg_start");
extern const uint8_t test2_003_jpg_start[] asm("_binary_test2_003_jpg_start");
extern const uint8_t test2_004_jpg_start[] asm("_binary_test2_004_jpg_start");
extern const uint8_t test3_001_jpg_start[] asm("_binary_test3_001_jpg_start");
extern const uint8_t test3_002_jpg_start[] asm("_binary_test3_002_jpg_start");
extern const uint8_t test3_003_jpg_start[] asm("_binary_test3_003_jpg_start");
extern const uint8_t test3_004_jpg_start[] asm("_binary_test3_004_jpg_start");
extern const uint8_t test4_001_jpg_start[] asm("_binary_test4_001_jpg_start");
extern const uint8_t test4_002_jpg_start[] asm("_binary_test4_002_jpg_start");
extern const uint8_t test4_003_jpg_start[] asm("_binary_test4_003_jpg_start");
extern const uint8_t test4_004_jpg_start[] asm("_binary_test4_004_jpg_start");
extern const uint8_t test_tiger_001_jpg_start[] asm("_binary_tiger_001_jpg_start");
extern const uint8_t test_tiger_002_jpg_start[] asm("_binary_tiger_002_jpg_start");
extern const uint8_t test_tiger_003_jpg_start[] asm("_binary_tiger_003_jpg_start");
extern const uint8_t test_tiger_004_jpg_start[] asm("_binary_tiger_004_jpg_start");
extern const uint8_t ny_test0_001_jpg_start[] asm("_binary_ny_test0_001_jpg_start");
extern const uint8_t ny_test0_002_jpg_start[] asm("_binary_ny_test0_002_jpg_start");
extern const uint8_t ny_test0_003_jpg_start[] asm("_binary_ny_test0_003_jpg_start");
extern const uint8_t ny_test0_004_jpg_start[] asm("_binary_ny_test0_004_jpg_start");
extern const uint8_t ny_test1_001_jpg_start[] asm("_binary_ny_test1_001_jpg_start");
extern const uint8_t ny_test1_002_jpg_start[] asm("_binary_ny_test1_002_jpg_start");
extern const uint8_t ny_test1_003_jpg_start[] asm("_binary_ny_test1_003_jpg_start");
extern const uint8_t ny_test1_004_jpg_start[] asm("_binary_ny_test1_004_jpg_start");
extern const uint8_t ny_test2_001_jpg_start[] asm("_binary_ny_test2_001_jpg_start");
extern const uint8_t ny_test2_002_jpg_start[] asm("_binary_ny_test2_002_jpg_start");
extern const uint8_t ny_test2_003_jpg_start[] asm("_binary_ny_test2_003_jpg_start");
extern const uint8_t ny_test2_004_jpg_start[] asm("_binary_ny_test2_004_jpg_start");
extern const uint8_t ny_test3_001_jpg_start[] asm("_binary_ny_test3_001_jpg_start");
extern const uint8_t ny_test3_002_jpg_start[] asm("_binary_ny_test3_002_jpg_start");
extern const uint8_t ny_test3_003_jpg_start[] asm("_binary_ny_test3_003_jpg_start");
extern const uint8_t ny_test3_004_jpg_start[] asm("_binary_ny_test3_004_jpg_start");

extern const uint8_t g1_test_001_jpg_start[] asm("_binary_group1_001_jpg_start");
extern const uint8_t g1_test_002_jpg_start[] asm("_binary_group1_002_jpg_start");
extern const uint8_t g1_test_003_jpg_start[] asm("_binary_group1_003_jpg_start");
extern const uint8_t g1_test_004_jpg_start[] asm("_binary_group1_004_jpg_start");

extern const uint8_t g2_test_001_jpg_start[] asm("_binary_group2_001_jpg_start");
extern const uint8_t g2_test_002_jpg_start[] asm("_binary_group2_002_jpg_start");
extern const uint8_t g2_test_003_jpg_start[] asm("_binary_group2_003_jpg_start");
extern const uint8_t g2_test_004_jpg_start[] asm("_binary_group2_004_jpg_start");

//Define the height and width of the jpeg file. Make sure this matches the actual jpeg
//dimensions.
#define IMAGE_W 336
#define IMAGE_H 256

const char *TAG_DEC = "ImageDec";

//Data that is passed from the decoder function to the infunc/outfunc functions.
typedef struct {
    const unsigned char *inData; //Pointer to jpeg data
    uint16_t inPos;              //Current position in jpeg data
    uint16_t **outData;          //Array of IMAGE_H pointers to arrays of IMAGE_W 16-bit pixel values
    int outW;                    //Width of the resulting file
    int outH;                    //Height of the resulting file
} JpegDev;

//Input function for jpeg decoder. Just returns bytes from the inData field of the JpegDev structure.
static uint16_t infunc(JDEC *decoder, uint8_t *buf, uint16_t len)
{
    //Read bytes from input file
    JpegDev *jd = (JpegDev *)decoder->device;
    if (buf != NULL) {
        memcpy(buf, jd->inData + jd->inPos, len);
    }
    jd->inPos += len;
    return len;
}

//Output function. Re-encodes the RGB888 data from the decoder as big-endian RGB565 and
//stores it in the outData array of the JpegDev structure.
static uint16_t outfunc(JDEC *decoder, void *bitmap, JRECT *rect)
{
    JpegDev *jd = (JpegDev *)decoder->device;
    uint8_t *in = (uint8_t *)bitmap;
    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++) {
            //We need to convert the 3 bytes in `in` to a rgb565 value.
            uint16_t v = 0;
            v |= ((in[0] >> 3) << 11);
            v |= ((in[1] >> 2) << 5);
            v |= ((in[2] >> 3) << 0);
            //The LCD wants the 16-bit value in big-endian, so swap bytes
            v = (v >> 8) | (v << 8);
            jd->outData[y][x] = v;
            in += 3;
        }
    }
    return 1;
}

//Size of the work space for the jpeg decoder.
#define WORKSZ 3100

//Decode the embedded image into pixel lines that can be used with the rest of the logic.
int heightPixel, widthPixel;
esp_err_t decode_image(uint16_t ***pixels, Image im)
{
    char *work = NULL;
    int r;
    JDEC decoder;
    JpegDev jd;
    *pixels = NULL;
    esp_err_t ret = ESP_OK;
    

    //Alocate pixel memory. Each line is an array of IMAGE_W 16-bit pixels; the `*pixels` array itself contains pointers to these lines.
    switch(im)
    {
//    	case(imager): heightPixel = IMAGE_H;  widthPixel = IMAGE_W; jd.inData = image_jpg_start; break;
//    	case(hum0): heightPixel = HUMIDITY_H;  widthPixel = HUMIDITY_W; jd.inData = hum0_jpg_start; break;
//    	case(hum1): heightPixel = HUMIDITY_H;  widthPixel = HUMIDITY_W; jd.inData = hum1_jpg_start; break;
//    	case(temp0): heightPixel = TEMPERATURE_H;  widthPixel = TEMPERATURE_W; jd.inData = temp0_jpg_start; break;
//    	case(temp1): heightPixel = TEMPERATURE_H;  widthPixel = TEMPERATURE_W; jd.inData = temp1_jpg_start; break;
//        case(CO2): heightPixel = CO2_H;  widthPixel = CO2_W; jd.inData = co2_jpg_start; break;

//        case(Im1): heightPixel = IM1_H;  widthPixel = IM1_W; jd.inData = im1_jpg_start; break;
//        case(Im2): heightPixel = IM2_H;  widthPixel = IM2_W; jd.inData = im2_jpg_start; break;

        case(WindE): heightPixel = WIND_H;  widthPixel = WIND_W; jd.inData = windE_jpg_start; break;
        case(WindN): heightPixel = WIND_H;  widthPixel = WIND_W; jd.inData = windN_jpg_start; break;
        case(WindNE): heightPixel = WIND_H;  widthPixel = WIND_W; jd.inData = windNE_jpg_start; break;
        case(WindNW): heightPixel = WIND_H;  widthPixel = WIND_W; jd.inData = windNW_jpg_start; break;
        case(WindS): heightPixel = WIND_H;  widthPixel = WIND_W; jd.inData = windS_jpg_start; break;
        case(WindSE): heightPixel = WIND_H;  widthPixel = WIND_W; jd.inData = windSE_jpg_start; break;
        case(WindSW): heightPixel = WIND_H;  widthPixel = WIND_W; jd.inData = windSW_jpg_start; break;
        case(WindW): heightPixel = WIND_H;  widthPixel = WIND_W; jd.inData = windW_jpg_start; break;

        case(Wclear_day): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_clearDay_jpg_start; break;
        case(Wclear_night): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_clearNight_jpg_start; break;
        case(Wcloudy): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_cludly_jpg_start; break;
        case(Wdrizzle): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_drizzle_jpg_start; break;
        case(Wfog): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_fog_jpg_start; break;
        case(Whail): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_hail_jpg_start; break;
        case(WlightRain): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_lightRain_jpg_start; break;
        case(Wparly_cloudy_day): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_partly_cloudy_day_jpg_start; break;
        case(Wparly_cloudy_night): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_partly_cloudy_night_jpg_start; break;
        case(Wrain): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_rain_jpg_start; break;
        case(Wsleet): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_sleet_jpg_start; break;
        case(Wsnow): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_snow_jpg_start; break;
        case(Wthunderstorm): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_thunderstorm_jpg_start; break;
        case(Wwind): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_wind_jpg_start; break;
        case(Wunknown): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; jd.inData = w_unknown_jpg_start; break;

        case(B_Wclear_day): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_clearDay_jpg_start; break;
        case(B_Wclear_night): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_clearNight_jpg_start; break;
        case(B_Wcloudy): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_cludly_jpg_start; break;
        case(B_Wdrizzle): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_drizzle_jpg_start; break;
        case(B_Wfog): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_fog_jpg_start; break;
        case(B_Whail): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_hail_jpg_start; break;
        case(B_WlightRain): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_lightRain_jpg_start; break;
        case(B_Wparly_cloudy_day): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_partly_cloudy_day_jpg_start; break;
        case(B_Wparly_cloudy_night): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_partly_cloudy_night_jpg_start; break;
        case(B_Wrain): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_rain_jpg_start; break;
        case(B_Wsleet): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_sleet_jpg_start; break;
        case(B_Wsnow): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_snow_jpg_start; break;
        case(B_Wthunderstorm): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_thunderstorm_jpg_start; break;
        case(B_Wwind): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_wind_jpg_start; break;
        case(B_Wunknown): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; jd.inData = b_w_unknown_jpg_start; break;

        case(Moon_L0): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L0_jpg_start; break;
        case(Moon_L1): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L1_jpg_start; break;
        case(Moon_L2): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L2_jpg_start; break;
        case(Moon_L3): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L3_jpg_start; break;
        case(Moon_L4): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L4_jpg_start; break;
        case(Moon_L5): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L5_jpg_start; break;
        case(Moon_L6): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L6_jpg_start; break;
        case(Moon_L7): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L7_jpg_start; break;
        case(Moon_L8): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L8_jpg_start; break;
        case(Moon_L9): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L9_jpg_start; break;
        case(Moon_L10): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L10_jpg_start; break;
        case(Moon_L11): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L11_jpg_start; break;
        case(Moon_L12): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L12_jpg_start; break;
        case(Moon_L13): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L13_jpg_start; break;
        case(Moon_L14): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L14_jpg_start; break;
        case(Moon_L15): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L15_jpg_start; break;
        case(Moon_L16): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L16_jpg_start; break;
        case(Moon_L17): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L17_jpg_start; break;
        case(Moon_L18): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L18_jpg_start; break;
        case(Moon_L19): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L19_jpg_start; break;
        case(Moon_L20): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L20_jpg_start; break;
        case(Moon_L21): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L21_jpg_start; break;
        case(Moon_L22): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L22_jpg_start; break;
        case(Moon_L23): heightPixel = MOON_H;  widthPixel = MOON_W; jd.inData = moonphase_L23_jpg_start; break;

//        case(ImTest1_001): heightPixel = IM_TEST_1_H;  widthPixel = IM_TEST_1_W; jd.inData = test1_001_jpg_start; break;
//        case(ImTest1_002): heightPixel = IM_TEST_2_H;  widthPixel = IM_TEST_2_W; jd.inData = test1_002_jpg_start; break;
//        case(ImTest1_003): heightPixel = IM_TEST_3_H;  widthPixel = IM_TEST_3_W; jd.inData = test1_003_jpg_start; break;
//        case(ImTest1_004): heightPixel = IM_TEST_4_H;  widthPixel = IM_TEST_4_W; jd.inData = test1_004_jpg_start; break;
//        case(ImTest2_001): heightPixel = IM_TEST_1_H;  widthPixel = IM_TEST_1_W; jd.inData = test2_001_jpg_start; break;
//        case(ImTest2_002): heightPixel = IM_TEST_2_H;  widthPixel = IM_TEST_2_W; jd.inData = test2_002_jpg_start; break;
//        case(ImTest2_003): heightPixel = IM_TEST_3_H;  widthPixel = IM_TEST_3_W; jd.inData = test2_003_jpg_start; break;
//        case(ImTest2_004): heightPixel = IM_TEST_4_H;  widthPixel = IM_TEST_4_W; jd.inData = test2_004_jpg_start; break;
//        case(ImTest3_001): heightPixel = IM_TEST_1_H;  widthPixel = IM_TEST_1_W; jd.inData = test3_001_jpg_start; break;
//        case(ImTest3_002): heightPixel = IM_TEST_2_H;  widthPixel = IM_TEST_2_W; jd.inData = test3_002_jpg_start; break;
//        case(ImTest3_003): heightPixel = IM_TEST_3_H;  widthPixel = IM_TEST_3_W; jd.inData = test3_003_jpg_start; break;
//        case(ImTest3_004): heightPixel = IM_TEST_4_H;  widthPixel = IM_TEST_4_W; jd.inData = test3_004_jpg_start; break;
//        case(ImTest4_001): heightPixel = IM_TEST_1_H;  widthPixel = IM_TEST_1_W; jd.inData = test4_001_jpg_start; break;
//        case(ImTest4_002): heightPixel = IM_TEST_2_H;  widthPixel = IM_TEST_2_W; jd.inData = test4_002_jpg_start; break;
//        case(ImTest4_003): heightPixel = IM_TEST_3_H;  widthPixel = IM_TEST_3_W; jd.inData = test4_003_jpg_start; break;
//        case(ImTest4_004): heightPixel = IM_TEST_4_H;  widthPixel = IM_TEST_4_W; jd.inData = test4_004_jpg_start; break;

//        case(ImTestTiger_001): heightPixel = IM_TEST_TIGER_H;  widthPixel = IM_TEST_TIGER_W; jd.inData = test_tiger_001_jpg_start; break;
//        case(ImTestTiger_002): heightPixel = IM_TEST_TIGER_H;  widthPixel = IM_TEST_TIGER_W; jd.inData = test_tiger_002_jpg_start; break;
//        case(ImTestTiger_003): heightPixel = IM_TEST_TIGER_H;  widthPixel = IM_TEST_TIGER_W; jd.inData = test_tiger_003_jpg_start; break;
//        case(ImTestTiger_004): heightPixel = IM_TEST_TIGER_H;  widthPixel = IM_TEST_TIGER_W; jd.inData = test_tiger_004_jpg_start; break;

//        case(ImTestNy0_001): heightPixel = IM_TEST_NU0_H;  widthPixel = IM_TEST_NU0_W; jd.inData = ny_test0_001_jpg_start; break;
//        case(ImTestNy0_002): heightPixel = IM_TEST_NU0_H;  widthPixel = IM_TEST_NU0_W; jd.inData = ny_test0_002_jpg_start; break;
//        case(ImTestNy0_003): heightPixel = IM_TEST_NU0_H;  widthPixel = IM_TEST_NU0_W; jd.inData = ny_test0_003_jpg_start; break;
//        case(ImTestNy0_004): heightPixel = IM_TEST_NU0_H;  widthPixel = IM_TEST_NU0_W; jd.inData = ny_test0_004_jpg_start; break;

//        case(ImTestNy1_001): heightPixel = IM_TEST_NU1_H;  widthPixel = IM_TEST_NU1_W; jd.inData = ny_test1_001_jpg_start; break;
//        case(ImTestNy1_002): heightPixel = IM_TEST_NU1_H;  widthPixel = IM_TEST_NU1_W; jd.inData = ny_test1_002_jpg_start; break;
//        case(ImTestNy1_003): heightPixel = IM_TEST_NU1_H;  widthPixel = IM_TEST_NU1_W; jd.inData = ny_test1_003_jpg_start; break;
//        case(ImTestNy1_004): heightPixel = IM_TEST_NU1_H;  widthPixel = IM_TEST_NU1_W; jd.inData = ny_test1_004_jpg_start; break;

//        case(ImTestNy2_001): heightPixel = IM_TEST_NU2_12_H;  widthPixel = IM_TEST_NU2_W; jd.inData = ny_test2_001_jpg_start; break;
//        case(ImTestNy2_002): heightPixel = IM_TEST_NU2_12_H;  widthPixel = IM_TEST_NU2_W; jd.inData = ny_test2_002_jpg_start; break;
//        case(ImTestNy2_003): heightPixel = IM_TEST_NU2_34_H;  widthPixel = IM_TEST_NU2_W; jd.inData = ny_test2_003_jpg_start; break;
//        case(ImTestNy2_004): heightPixel = IM_TEST_NU2_34_H;  widthPixel = IM_TEST_NU2_W; jd.inData = ny_test2_004_jpg_start; break;

//        case(ImTestNy3_001): heightPixel = IM_TEST_NU3_H;  widthPixel = IM_TEST_NU3_W; jd.inData = ny_test3_001_jpg_start; break;
//        case(ImTestNy3_002): heightPixel = IM_TEST_NU3_H;  widthPixel = IM_TEST_NU3_W; jd.inData = ny_test3_002_jpg_start; break;
//        case(ImTestNy3_003): heightPixel = IM_TEST_NU3_H;  widthPixel = IM_TEST_NU3_W; jd.inData = ny_test3_003_jpg_start; break;
//        case(ImTestNy3_004): heightPixel = IM_TEST_NU3_H;  widthPixel = IM_TEST_NU3_W; jd.inData = ny_test3_004_jpg_start; break;

//        case(ImTestG1_001): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; jd.inData = g1_test_001_jpg_start; break;
//        case(ImTestG1_002): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; jd.inData = g1_test_002_jpg_start; break;
//        case(ImTestG1_003): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; jd.inData = g1_test_003_jpg_start; break;
//        case(ImTestG1_004): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; jd.inData = g1_test_004_jpg_start; break;

//        case(ImTestG2_001): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; jd.inData = g2_test_001_jpg_start; break;
//        case(ImTestG2_002): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; jd.inData = g2_test_002_jpg_start; break;
//        case(ImTestG2_003): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; jd.inData = g2_test_003_jpg_start; break;
//        case(ImTestG2_004): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; jd.inData = g2_test_004_jpg_start; break;

    	default:goto err;
    
    }
    
    *pixels = calloc(heightPixel, sizeof(uint16_t *));
    if (*pixels == NULL) {
        ESP_LOGE(TAG_DEC, "Error allocating memory for lines");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }
    for (int i = 0; i < heightPixel; i++) {
        (*pixels)[i] = malloc(widthPixel * sizeof(uint16_t));
        if ((*pixels)[i] == NULL) {
            ESP_LOGE(TAG_DEC, "Error allocating memory for line %d", i);
            ret = ESP_ERR_NO_MEM;
            goto err;
        }
    }

    //Allocate the work space for the jpeg decoder.
    work = calloc(WORKSZ, 1);
    if (work == NULL) {
        ESP_LOGE(TAG_DEC, "Cannot allocate workspace");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }

    //Populate fields of the JpegDev struct.
    //jd.inData = image_jpg_start;
    jd.inPos = 0;
    jd.outData = *pixels;
    jd.outW = widthPixel;
    jd.outH = heightPixel;

    //Prepare and decode the jpeg.
    r = jd_prepare(&decoder, infunc, work, WORKSZ, (void *)&jd);
    if (r != JDR_OK) {
        ESP_LOGE(TAG_DEC, "Image decoder: jd_prepare failed (%d)", r);
        ret = ESP_ERR_NOT_SUPPORTED;
        goto err;
    }
    r = jd_decomp(&decoder, outfunc, 0);
    if (r != JDR_OK && r != JDR_FMT1) {
        ESP_LOGE(TAG_DEC, "Image decoder: jd_decode failed (%d)", r);
        ret = ESP_ERR_NOT_SUPPORTED;
        goto err;
    }

    //All done! Free the work area (as we don't need it anymore) and return victoriously.
    free(work);
    return ret;
err:
    //Something went wrong! Exit cleanly, de-allocating everything we allocated.
    if (*pixels != NULL) {
        for (int i = 0; i < heightPixel; i++) {
            free((*pixels)[i]);
        }
        free(*pixels);
    }
    free(work);
    return ret;
}

void free_image(uint16_t ***pixels, Image im)
{
    //Alocate pixel memory. Each line is an array of IMAGE_W 16-bit pixels; the `*pixels` array itself contains pointers to these lines.
    switch(im)
    {
//        case(imager): heightPixel = IMAGE_H;  widthPixel = IMAGE_W; break;
//        case(hum0): heightPixel = HUMIDITY_H;  widthPixel = HUMIDITY_W; break;
//        case(hum1): heightPixel = HUMIDITY_H;  widthPixel = HUMIDITY_W; break;
//        case(temp0): heightPixel = TEMPERATURE_H;  widthPixel = TEMPERATURE_W; break;
//        case(temp1): heightPixel = TEMPERATURE_H;  widthPixel = TEMPERATURE_W; break;
//        case(CO2): heightPixel = CO2_H;  widthPixel = CO2_W; break;

//        case(Im1): heightPixel = IM1_H;  widthPixel = IM1_W; break;
//        case(Im2): heightPixel = IM2_H;  widthPixel = IM2_W; break;

        case(WindE): heightPixel = WIND_H;  widthPixel = WIND_W; break;
        case(WindN): heightPixel = WIND_H;  widthPixel = WIND_W; break;
        case(WindNE): heightPixel = WIND_H;  widthPixel = WIND_W; break;
        case(WindNW): heightPixel = WIND_H;  widthPixel = WIND_W; break;
        case(WindS): heightPixel = WIND_H;  widthPixel = WIND_W; break;
        case(WindSE): heightPixel = WIND_H;  widthPixel = WIND_W; break;
        case(WindSW): heightPixel = WIND_H;  widthPixel = WIND_W; break;
        case(WindW): heightPixel = WIND_H;  widthPixel = WIND_W; break;

        case(Wclear_day): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wclear_night): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wcloudy): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wdrizzle): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wfog): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Whail): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(WlightRain): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wparly_cloudy_day): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wparly_cloudy_night): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wrain): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wsleet): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wsnow): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wthunderstorm): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wwind): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;
        case(Wunknown): heightPixel = WEATHER_H;  widthPixel = WEATHER_W; break;

        case(B_Wclear_day): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wclear_night): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wcloudy): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wdrizzle): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wfog): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Whail): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_WlightRain): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wparly_cloudy_day): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wparly_cloudy_night): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wrain): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wsleet): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wsnow): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wthunderstorm): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wwind): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;
        case(B_Wunknown): heightPixel = WEATHERB_H;  widthPixel = WEATHERB_W; break;

        case(Moon_L0):
        case(Moon_L1):
        case(Moon_L2):
        case(Moon_L3):
        case(Moon_L4):
        case(Moon_L5):
        case(Moon_L6):
        case(Moon_L7):
        case(Moon_L8):
        case(Moon_L9):
        case(Moon_L10):
        case(Moon_L11):
        case(Moon_L12):
        case(Moon_L13):
        case(Moon_L14):
        case(Moon_L15):
        case(Moon_L16):
        case(Moon_L17):
        case(Moon_L18):
        case(Moon_L19):
        case(Moon_L20):
        case(Moon_L21):
        case(Moon_L22):
        case(Moon_L23): heightPixel = MOON_H;  widthPixel = MOON_W; break;

//        case(ImTest1_001): heightPixel = IM_TEST_1_H;  widthPixel = IM_TEST_1_W; break;
//        case(ImTest1_002): heightPixel = IM_TEST_2_H;  widthPixel = IM_TEST_2_W; break;
//        case(ImTest1_003): heightPixel = IM_TEST_3_H;  widthPixel = IM_TEST_3_W; break;
//        case(ImTest1_004): heightPixel = IM_TEST_4_H;  widthPixel = IM_TEST_4_W; break;
//        case(ImTest2_001): heightPixel = IM_TEST_1_H;  widthPixel = IM_TEST_1_W; break;
//        case(ImTest2_002): heightPixel = IM_TEST_2_H;  widthPixel = IM_TEST_2_W; break;
//        case(ImTest2_003): heightPixel = IM_TEST_3_H;  widthPixel = IM_TEST_3_W; break;
//        case(ImTest2_004): heightPixel = IM_TEST_4_H;  widthPixel = IM_TEST_4_W; break;
//        case(ImTest3_001): heightPixel = IM_TEST_1_H;  widthPixel = IM_TEST_1_W; break;
//        case(ImTest3_002): heightPixel = IM_TEST_2_H;  widthPixel = IM_TEST_2_W; break;
//        case(ImTest3_003): heightPixel = IM_TEST_3_H;  widthPixel = IM_TEST_3_W; break;
//        case(ImTest3_004): heightPixel = IM_TEST_4_H;  widthPixel = IM_TEST_4_W; break;
//        case(ImTest4_001): heightPixel = IM_TEST_1_H;  widthPixel = IM_TEST_1_W; break;
//        case(ImTest4_002): heightPixel = IM_TEST_2_H;  widthPixel = IM_TEST_2_W; break;
//        case(ImTest4_003): heightPixel = IM_TEST_3_H;  widthPixel = IM_TEST_3_W; break;
//        case(ImTest4_004): heightPixel = IM_TEST_4_H;  widthPixel = IM_TEST_4_W; break;

//        case(ImTestTiger_001): heightPixel = IM_TEST_TIGER_H;  widthPixel = IM_TEST_TIGER_W; break;
//        case(ImTestTiger_002): heightPixel = IM_TEST_TIGER_H;  widthPixel = IM_TEST_TIGER_W; break;
//        case(ImTestTiger_003): heightPixel = IM_TEST_TIGER_H;  widthPixel = IM_TEST_TIGER_W; break;
//        case(ImTestTiger_004): heightPixel = IM_TEST_TIGER_H;  widthPixel = IM_TEST_TIGER_W; break;

//        case(ImTestNy0_001): heightPixel = IM_TEST_NU0_H;  widthPixel = IM_TEST_NU0_W; break;
//        case(ImTestNy0_002): heightPixel = IM_TEST_NU0_H;  widthPixel = IM_TEST_NU0_W; break;
//        case(ImTestNy0_003): heightPixel = IM_TEST_NU0_H;  widthPixel = IM_TEST_NU0_W; break;
//        case(ImTestNy0_004): heightPixel = IM_TEST_NU0_H;  widthPixel = IM_TEST_NU0_W; break;
//        case(ImTestNy1_001): heightPixel = IM_TEST_NU1_H;  widthPixel = IM_TEST_NU1_W; break;
//        case(ImTestNy1_002): heightPixel = IM_TEST_NU1_H;  widthPixel = IM_TEST_NU1_W; break;
//        case(ImTestNy1_003): heightPixel = IM_TEST_NU1_H;  widthPixel = IM_TEST_NU1_W; break;
//        case(ImTestNy1_004): heightPixel = IM_TEST_NU1_H;  widthPixel = IM_TEST_NU1_W; break;
//        case(ImTestNy2_001): heightPixel = IM_TEST_NU2_12_H;  widthPixel = IM_TEST_NU2_W; break;
//        case(ImTestNy2_002): heightPixel = IM_TEST_NU2_12_H;  widthPixel = IM_TEST_NU2_W; break;
//        case(ImTestNy2_003): heightPixel = IM_TEST_NU2_34_H;  widthPixel = IM_TEST_NU2_W; break;
//        case(ImTestNy2_004): heightPixel = IM_TEST_NU2_34_H;  widthPixel = IM_TEST_NU2_W; break;
//        case(ImTestNy3_001): heightPixel = IM_TEST_NU3_H;  widthPixel = IM_TEST_NU3_W; break;
//        case(ImTestNy3_002): heightPixel = IM_TEST_NU3_H;  widthPixel = IM_TEST_NU3_W; break;
//        case(ImTestNy3_003): heightPixel = IM_TEST_NU3_H;  widthPixel = IM_TEST_NU3_W; break;
//        case(ImTestNy3_004): heightPixel = IM_TEST_NU3_H;  widthPixel = IM_TEST_NU3_W; break;

//        case(ImTestG1_001): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; break;
//        case(ImTestG1_002): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; break;
//        case(ImTestG1_003): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; break;
//        case(ImTestG1_004): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; break;

//        case(ImTestG2_001): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; break;
//        case(ImTestG2_002): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; break;
//        case(ImTestG2_003): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; break;
//        case(ImTestG2_004): heightPixel = IM_TEST_GROUP_H;  widthPixel = IM_TEST_GROUP_W; break;


        default:return;

    }

    for (int i = 0; i < heightPixel; i++) {
        free((*pixels)[i]);
    }
    free((*pixels));
}
