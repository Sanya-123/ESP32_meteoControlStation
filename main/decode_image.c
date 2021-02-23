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
extern const uint8_t image_jpg_start[] asm("_binary_imager_jpg_start");
extern const uint8_t image_jpg_end[] asm("_binary_imager_jpg_end");

extern const uint8_t hum0_jpg_start[] asm("_binary_humidity0_jpg_start");
extern const uint8_t hum0_jpg_end[] asm("_binary_humidity0_jpg_end");

extern const uint8_t hum1_jpg_start[] asm("_binary_humidity1_jpg_start");
extern const uint8_t hum1_jpg_end[] asm("_binary_humidity1_jpg_end");

extern const uint8_t temp0_jpg_start[] asm("_binary_temperature0_jpg_start");
extern const uint8_t temp0_jpg_end[] asm("_binary_temperature0_jpg_end");

extern const uint8_t temp1_jpg_start[] asm("_binary_temperature1_jpg_start");
extern const uint8_t temp1_jpg_end[] asm("_binary_temperature1_jpg_end");

extern const uint8_t co2_jpg_start[] asm("_binary_co2_jpg_start");
extern const uint8_t co2_jpg_end[] asm("_binary_co2_jpg_end");
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
    	case(imager): heightPixel = IMAGE_H;  widthPixel = IMAGE_W; jd.inData = image_jpg_start; break;
    	case(hum0): heightPixel = HUMIDITY_H;  widthPixel = HUMIDITY_W; jd.inData = hum0_jpg_start; break;
    	case(hum1): heightPixel = HUMIDITY_H;  widthPixel = HUMIDITY_W; jd.inData = hum1_jpg_start; break;
    	case(temp0): heightPixel = TEMPERATURE_H;  widthPixel = TEMPERATURE_W; jd.inData = temp0_jpg_start; break;
    	case(temp1): heightPixel = TEMPERATURE_H;  widthPixel = TEMPERATURE_W; jd.inData = temp1_jpg_start; break;
        case(CO2): heightPixel = CO2_H;  widthPixel = CO2_W; jd.inData = co2_jpg_start; break;
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
