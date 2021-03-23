#include "display.h"
#include "ST7735.h"
#include "decode_image.h"
#include "esp_log.h"
//#include "tft2/tftfunc.h"
//#include "tft2/spi_master.h"
//#include "tft/tft.h"
//#include "gpioDEF.h"

//#define SPI_BUS VSPI_HOST
//#define DISPLAY_READ 1
//#define DELAY 0x80

#define SLIM                2
#define WIDTH_RECT          75
#define HEIGHT_RECT         24
#define WIDTH_IN_RECT       (WIDTH_RECT-2*SLIM)
#define HEIGHT_IN_RECT      (HEIGHT_RECT-2*SLIM)
#define X_HUMIDITI          (10)
#define Y_HUMIDITI          (50)
#define X_TERM              (250)
#define Y_TERM              (25)
#define X_P                 200
#define Y_P                 50
#define X_CO2               110
#define Y_CO2               130
#define H_CO2               110
#define W_CO2               120
#define X_WORLD_CO2         190
#define Y_WORLD_CO2         140
#define X_CO2_VAL           213
#define Y_CO2_VAL           150//80 + 2*2*5

enum stateDisplay stateD = none;
uint16_t **pixels_hum0;
uint16_t **pixels_hum1;

uint16_t **pixels_temp0;
uint16_t **pixels_temp1;

uint16_t **pixels_co2;

uint16_t **pixels_im1;
uint16_t **pixels_im2;

void initDisplay()
{
    tft_init();
    if(decode_image(&pixels_hum0, hum0))
    {
        ESP_LOGI("Display", "Can't ionit picture hum");
    }
    if(decode_image(&pixels_hum1, hum1))
    {
        ESP_LOGI("Display", "Can't ionit picture hum1");
    }
    if(decode_image(&pixels_temp0, temp0))
    {
        ESP_LOGI("Display", "Can't ionit picture temp0");
    }
    if(decode_image(&pixels_temp1, temp1))
    {
       ESP_LOGI("Display", "Can't ionit picture temp1");
    }
    if(decode_image(&pixels_co2, CO2))
    {
       ESP_LOGI("Display", "Can't ionit picture co2");
    }

//    if(decode_image(&pixels_im1, Im1))
//    {
//       ESP_LOGI("Display", "Can't ionit picture Im1");
//    }
//    if(decode_image(&pixels_im2, Im2))
//    {
//       ESP_LOGI("Display", "Can't ionit picture Im2");
//    }

    tft_fill_screen(DEFOULE_COLOR);
    
    send_picturte(X_HUMIDITI, Y_HUMIDITI, HUMIDITY_W, HUMIDITY_H, pixels_hum1);
    send_picturte(X_TERM, Y_TERM, TEMPERATURE_W, TEMPERATURE_H, pixels_temp1);
    send_picturte(X_CO2, Y_CO2, CO2_W, CO2_H, pixels_co2);

//    if(decode_image(&pixels_im2, Im2))
//    {
//        ESP_LOGI("Display", "Can't ionit picture Im2");
//    }
//    send_picturte(20, 20, IM1_W, IM1_H, pixels_im1);
//    free_image(&pixels_im1, Im1);
    
    stateD = stateMainForm;
}


/*
 *  _____________________
 * | __              __  |
 * ||  |            |  | |
 * ||  |            |  | |
 * ||  |            |  | |
 * ||  |  ________  |  | |
 * ||__| |________| |__| |
 * | ___________________ |
 * ||___________________||
 * |_____________________|
*/

void drawMainForm()
{
    stateD = stateMainForm;
    tft_fill_screen(DEFOULE_COLOR);
    //humiditi
    drawunfillRectangle(X_HUMIDITI - SLIM, Y_HUMIDITI - SLIM, WIDTH_RECT, HEIGHT_RECT, SLIM, COLOR_WHITE, DEFOULE_COLOR);//правая колонка
    tft_draw_string(5, Y_HUMIDITI + HEIGHT_IN_RECT, "h,%", COLOR_GREEN, DEFOULE_COLOR, 1);

    //temp
    drawunfillRectangle(X_TERM - SLIM, Y_TERM - SLIM, WIDTH_RECT, HEIGHT_RECT, SLIM, COLOR_WHITE, DEFOULE_COLOR);//левая колонка
    tft_draw_string(5, Y_TERM + HEIGHT_IN_RECT, "t,C", COLOR_YELLOW, DEFOULE_COLOR, 1);

    //CO2
    tft_rect(X_CO2, Y_CO2, H_CO2, W_CO2, COLOR_GREEN);
    tft_draw_string(X_WORLD_CO2, Y_WORLD_CO2, "CO2", DEFOULE_COLOR, COLOR_GREEN, 3);


    //menu
    tft_rect(99, 0, 28, 160, COLOR_BLUE);
    tft_draw_string(99, 159, "menu", DEFOULE_COLOR, COLOR_BLUE, 2);
    tft_draw_string(99, 79, "graph", DEFOULE_COLOR, COLOR_BLUE, 2);
}

void drawunfillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, int16_t s, uint16_t color, uint16_t colorBG)
{
    tft_rect(x, y, w, h, colorBG);

    tft_rect(x, y, s, h, color);//верхняя линия
    tft_rect(x + w - s, y, s, h, color);//нижняя
    tft_rect(x, y, w, s, color);//правая
    tft_rect(x, y + h - s, w, s, color);//правая
}

void setHumiditi(uint16_t val)
{//ему принадлежит левая колонка
    if(stateD != stateMainForm) return;
    if(val > 100)
        val = 100;

    /*st7735_rect(X_HUMIDITI - 4, 0, WIDTH_IN_RECT + 7, Y_HUMIDITI - SLIM, DEFOULE_COLOR);//clear old value

    uint16_t fill = WIDTH_IN_RECT*val/100;
    st7735_rect(X_HUMIDITI, Y_HUMIDITI, (WIDTH_IN_RECT - fill), HEIGHT_IN_RECT, DEFOULE_COLOR);
    st7735_rect(X_HUMIDITI + (WIDTH_IN_RECT - fill), Y_HUMIDITI, fill, HEIGHT_IN_RECT, COLOR_GREEN);*/
    
    uint16_t fill = HUMIDITY_H*val/100;
    send_picturte(X_HUMIDITI, Y_HUMIDITI+fill, HUMIDITY_W, HUMIDITY_H-fill, pixels_hum0);
    send_picturte(X_HUMIDITI, Y_HUMIDITI, HUMIDITY_W, fill, &pixels_hum1[HUMIDITY_H-fill]);
    
    
    char buff[10];
    sprintf(buff, "%3d%%", val);
    tft_draw_string(70, 8, buff, ILI9341_COLOR565(0x4E, 0xAD, 0xC1), DEFOULE_COLOR, 2);
    //st7735_draw_string(50, 10, buff, ILI9341_COLOR565(0x4E, 0xAD, 0xFF), DEFOULE_COLOR, 2);
}

void setTerm(int16_t val)
{
    if(stateD != stateMainForm) return;
    if(val > TERM_MAX)
        val = val;
    if(val < TERM_MIN)
        val = TERM_MIN;
    /*uint16_t fill = WIDTH_IN_RECT*(val - TERM_MIN)/(TERM_MAX - TERM_MIN);
    uint16_t color = 0x0000;

    if(val >= 30)
        color = COLOR_RED;
    else if(val >= 10)
        color = COLOR_YELLOW;
    else
        color = COLOR_BLUE;

    st7735_rect(X_TERM - 4, Y_TERM - SLIM + HEIGHT_RECT, WIDTH_IN_RECT + 7, Y_HUMIDITI - SLIM, DEFOULE_COLOR);//clear old value

    st7735_rect(X_TERM, Y_TERM, (WIDTH_IN_RECT - fill), HEIGHT_IN_RECT, DEFOULE_COLOR);
    st7735_rect(X_TERM + (WIDTH_IN_RECT - fill), Y_TERM, fill, HEIGHT_IN_RECT, color);*/
    
    uint16_t fill = TEMPERATURE_H*(val - TERM_MIN)/(TERM_MAX - TERM_MIN);
    send_picturte(X_TERM, Y_TERM+fill, TEMPERATURE_W, TEMPERATURE_H-fill, pixels_temp0);
    send_picturte(X_TERM, Y_TERM, TEMPERATURE_W, fill, &pixels_temp1[TEMPERATURE_H-fill]);
    
    char buff[10];
    sprintf(buff, "%3d C", val);
    tft_draw_string(300, 8, buff, /*color*/ILI9341_COLOR565(0xEF, 0xE0, 0x01), DEFOULE_COLOR, 2);
}

void setPa(uint16_t val)
{
    if(stateD != stateMainForm) return;
    tft_rect(X_P-71, Y_P, 80, 14, DEFOULE_COLOR);//clear old value

    char buff[20];
    sprintf(buff, "P=%d", val);
    tft_draw_string(val < 1000 ? (X_P + 5*0) : X_P, (Y_P), buff, ILI9341_COLOR565(0x9F, 0x4F, 0x74), DEFOULE_COLOR, 2);
}

void setCO2(uint16_t val)
{
    if(stateD != stateMainForm) return;
    uint16_t color = COLOR_GREEN;
    if(val > 1100)
        color = COLOR_YELLOW;
    if(val > 1600)
        color = ILI9341_COLOR565(0xFF, 0xFF, 0x00);//orange
    if(val > 2000)
        color = COLOR_RED;

    tft_rect(X_CO2_VAL-76, Y_CO2_VAL, 76, 30, ILI9341_COLOR565(0x00, 0x00, 0x00));
//    tft_draw_string(X_WORLD_CO2, Y_WORLD_CO2, "CO2", DEFOULE_COLOR, color, 5);

    char buff[20];
    sprintf(buff, "%4d", val);

    tft_draw_string(X_CO2_VAL, Y_CO2_VAL, buff, 0xFFFF, 0x0000, 4);
}

void print_im1()
{
    tft_fill_screen(DEFOULE_COLOR);
    if(decode_image(&pixels_im1, Im1))
    {
        ESP_LOGI("Display", "Can't ionit picture Im1");
    }
    else
    {
        send_picturte(20, 20, IM1_W, IM1_H, pixels_im1);
        free_image(&pixels_im1, Im1);
    }
}

void print_im2()
{
    tft_fill_screen(DEFOULE_COLOR);
    if(decode_image(&pixels_im2, Im2))
    {
        ESP_LOGI("Display", "Can't ionit picture Im2");
    }
    else
    {
        send_picturte(20, 20, IM2_W, IM2_H, pixels_im2);
        free_image(&pixels_im2, Im2);
    }
}
