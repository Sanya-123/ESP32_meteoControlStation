#include "display.h"
#include "tft.h"
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

//DIP forum
#define LINE1_X             0
#define LINE1_Y             80
#define LINE1_W             320
#define LINE1_H             1

#define LINE2_X             106
#define LINE2_Y             0
#define LINE2_W             1
#define LINE2_H             80

#define LINE3_X             212
#define LINE3_Y             0
#define LINE3_W             1
#define LINE3_H             80

#define LINE4_X             106
#define LINE4_Y             0
#define LINE4_W             1
#define LINE4_H             240

#define DIP_X_WORLD_T       275
#define DIP_Y_WORLD_T       60
#define DIP_X_WORLD_H       165
#define DIP_Y_WORLD_H       60
#define DIP_X_WORLD_P       60
#define DIP_Y_WORLD_P       60
#define DIP_X_WORLD_CO2     250
#define DIP_Y_WORLD_CO2     200

#define DIP_X_TERM_VAL      300
#define DIP_Y_TERM_VAL      20
#define DIP_X_HUM_VAL       190
#define DIP_Y_HUM_VAL       20
#define DIP_X_P_VAL         80
#define DIP_Y_P_VAL         20
#define DIP_X_CO2_VAL       300
#define DIP_Y_CO2_VAL       100

//open wheather
#define WHEATHER_IMG_NOW_X  140
#define WHEATHER_IMG_NOW_Y  140
#define WIND_IMG_NOW_X  245
#define WIND_IMG_NOW_Y  160
#define WIND_SPEED_NOW_X 300
#define WIND_SPEED_NOW_Y 130
#define TEMP_NOW_X      130
#define TEMP_NOW_Y      180
#define MOON_X          20
#define MOON_Y          120
#define TEMP_FEEL_X      300
#define TEMP_FEEL_Y      100
#define WHEATHER_HUM_X      300
#define WHEATHER_HUM_Y      70
#define WHEATHER_PRES_X      300
#define WHEATHER_PRES_Y      10
#define WHEATHER_VIS_X      300
#define WHEATHER_VIS_Y      40
#define WHEATHER_DISCRIPT_X      180
#define WHEATHER_DISCRIPT_Y      110

#define WHEATHER_IMG_FORC1_X 245
#define WHEATHER_IMG_FORC1_Y 50
#define WHEATHER_TEMP_FORC1_X 290
#define WHEATHER_TEMP_FORC1_Y 30
#define WHEATHER_IMG_FORC2_X 175
#define WHEATHER_IMG_FORC2_Y 50
#define WHEATHER_TEMP_FORC2_X 220
#define WHEATHER_TEMP_FORC2_Y 30
#define WHEATHER_IMG_FORC3_X 105
#define WHEATHER_IMG_FORC3_Y 50
#define WHEATHER_TEMP_FORC3_X 150
#define WHEATHER_TEMP_FORC3_Y 30
#define WHEATHER_IMG_FORC4_X 35
#define WHEATHER_IMG_FORC4_Y 50
#define WHEATHER_TEMP_FORC4_X 80
#define WHEATHER_TEMP_FORC4_Y 30


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

//    drawMainForm();

//    if(decode_image(&pixels_im2, Im2))
//    {
//        ESP_LOGI("Display", "Can't ionit picture Im2");
//    }
//    send_picturte(20, 20, IM1_W, IM1_H, pixels_im1);
//    free_image(&pixels_im1, Im1);
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

    tft_send_picturte(X_HUMIDITI, Y_HUMIDITI, HUMIDITY_W, HUMIDITY_H, pixels_hum1);
    tft_send_picturte(X_TERM, Y_TERM, TEMPERATURE_W, TEMPERATURE_H, pixels_temp1);
    tft_send_picturte(X_CO2, Y_CO2, CO2_W, CO2_H, pixels_co2);
//    tft_fill_screen(DEFOULE_COLOR);
//    //humiditi
//    drawunfillRectangle(X_HUMIDITI - SLIM, Y_HUMIDITI - SLIM, WIDTH_RECT, HEIGHT_RECT, SLIM, COLOR_WHITE, DEFOULE_COLOR);//правая колонка
//    tft_draw_string(5, Y_HUMIDITI + HEIGHT_IN_RECT, "h,%", COLOR_GREEN, DEFOULE_COLOR, 1);

//    //temp
//    drawunfillRectangle(X_TERM - SLIM, Y_TERM - SLIM, WIDTH_RECT, HEIGHT_RECT, SLIM, COLOR_WHITE, DEFOULE_COLOR);//левая колонка
//    tft_draw_string(5, Y_TERM + HEIGHT_IN_RECT, "t,C", COLOR_YELLOW, DEFOULE_COLOR, 1);

//    //CO2
//    tft_rect(X_CO2, Y_CO2, H_CO2, W_CO2, COLOR_GREEN);
//    tft_draw_string(X_WORLD_CO2, Y_WORLD_CO2, "CO2", DEFOULE_COLOR, COLOR_GREEN, 3);


//    //menu
//    tft_rect(99, 0, 28, 160, COLOR_BLUE);
//    tft_draw_string(99, 159, "menu", DEFOULE_COLOR, COLOR_BLUE, 2);
//    tft_draw_string(99, 79, "graph", DEFOULE_COLOR, COLOR_BLUE, 2);
}

void drawDipForm()
{
    stateD = stateDipForm;
    tft_fill_screen(DEFOULE_COLOR);

    tft_rect(LINE1_X, LINE1_Y, LINE1_W, LINE1_H, 0x0000);
    tft_rect(LINE2_X, LINE2_Y, LINE2_W, LINE2_H, 0x0000);
    tft_rect(LINE3_X, LINE3_Y, LINE3_W, LINE3_H, 0x0000);
    tft_rect(LINE4_X, LINE4_Y, LINE4_W, LINE4_H, 0x0000);

    tft_draw_char(DIP_X_WORLD_T, DIP_Y_WORLD_T, 't', 0x0000, DEFOULE_COLOR, 2);
    tft_draw_char(DIP_X_WORLD_H, DIP_Y_WORLD_H, 'H', 0x0000, DEFOULE_COLOR, 2);
    tft_draw_char(DIP_X_WORLD_P, DIP_Y_WORLD_P, 'P', 0x0000, DEFOULE_COLOR, 2);
    tft_draw_string(DIP_X_WORLD_CO2, DIP_Y_WORLD_CO2, "CO2", 0x0000, DEFOULE_COLOR, 4);
}

void drawDip2Form()
{
    stateD = stateDip2Form;
    tft_fill_screen(DEFOULE_COLOR);

    tft_rect(LINE1_X, LINE1_Y, LINE1_W, LINE1_H, 0x0000);
    tft_rect(LINE2_X, LINE2_Y, LINE2_W, LINE2_H, 0x0000);
    tft_rect(LINE3_X, LINE3_Y, LINE3_W, LINE3_H, 0x0000);
    tft_rect(LINE4_X, LINE4_Y, LINE4_W, LINE4_H, 0x0000);

    vTaskDelay(1);
    tft_rect(LINE4_X + 1, LINE1_Y + 1, LCD_WIDTH - LINE4_X - 1, LCD_HEIGHT - LINE1_Y - 1, COLOR_GREEN);
    vTaskDelay(1);
    tft_rect(LINE3_X + 1, 0, LCD_WIDTH - LINE3_X - 1, LINE1_Y, COLOR_YELLOW);
    vTaskDelay(1);
    tft_rect(LINE2_X + 1, 0, LINE3_X - LINE2_X - 1, LINE1_Y, COLOR_BLUE);
    vTaskDelay(1);
    tft_rect(0, 0, LINE2_X, LINE1_Y, COLOR_MAGENTA);

    tft_draw_char(DIP_X_WORLD_T, DIP_Y_WORLD_T, 't', 0x0000, COLOR_YELLOW, 2);
    tft_draw_char(DIP_X_WORLD_H, DIP_Y_WORLD_H, 'H', 0x0000, COLOR_BLUE, 2);
    tft_draw_char(DIP_X_WORLD_P, DIP_Y_WORLD_P, 'P', 0x0000, COLOR_MAGENTA, 2);
    tft_draw_string(DIP_X_WORLD_CO2, DIP_Y_WORLD_CO2, "CO2", 0x0000, COLOR_GREEN, 4);
}

void drawWheatherTest()
{
    tft_fill_screen(0x0000);

    stateD = stateWeatherTeat;
    uint16_t **pixels_image;
    if(decode_image(&pixels_image, B_Wclear_day))
    {
       ESP_LOGI("Display", "Can't ionit picture");
    }
    else
    {
        tft_send_picturte(WHEATHER_IMG_NOW_X, WHEATHER_IMG_NOW_Y, WEATHERB_W, WEATHERB_H, pixels_image);
        free_image(&pixels_image, B_Wclear_day);
    }

    if(decode_image(&pixels_image, WindNE))
    {
       ESP_LOGI("Display", "Can't ionit picture");
    }
    else
    {
        tft_send_picturte(WIND_IMG_NOW_X, WIND_IMG_NOW_Y, WIND_W, WIND_H, pixels_image);
        free_image(&pixels_image, WindNE);
    }

     tft_draw_string(WIND_SPEED_NOW_X, WIND_SPEED_NOW_Y, "2.1 m/s", ILI9341_COLOR565(0xF8, 0xB4, 0x00), 0x0000, 2);

     tft_draw_string(TEMP_NOW_X, TEMP_NOW_Y, "+27 C", ILI9341_COLOR565(0xF8, 0xB4, 0x00), 0x0000, 4);

     testMoon();


     if(decode_image(&pixels_image, Wclear_night))
     {
        ESP_LOGI("Display", "Can't ionit picture");
     }
     else
     {
         tft_send_picturte(WHEATHER_IMG_FORC1_X, WHEATHER_IMG_FORC1_Y, WEATHER_W, WEATHER_H, pixels_image);
         free_image(&pixels_image, Wclear_night);
     }

     tft_draw_string(WHEATHER_TEMP_FORC1_X, WHEATHER_TEMP_FORC1_Y, "+27 C", ILI9341_COLOR565(0xF8, 0xB4, 0x00), 0x0000, 2);

     if(decode_image(&pixels_image, Wcloudy))
     {
        ESP_LOGI("Display", "Can't ionit picture");
     }
     else
     {
         tft_send_picturte(WHEATHER_IMG_FORC2_X, WHEATHER_IMG_FORC2_Y, WEATHER_W, WEATHER_H, pixels_image);
         free_image(&pixels_image, Wcloudy);
     }

     tft_draw_string(WHEATHER_TEMP_FORC2_X, WHEATHER_TEMP_FORC2_Y, "+10 C", ILI9341_COLOR565(0xF8, 0xB4, 0x00), 0x0000, 2);

     if(decode_image(&pixels_image, WlightRain))
     {
        ESP_LOGI("Display", "Can't ionit picture");
     }
     else
     {
         tft_send_picturte(WHEATHER_IMG_FORC3_X, WHEATHER_IMG_FORC3_Y, WEATHER_W, WEATHER_H, pixels_image);
         free_image(&pixels_image, WlightRain);
     }

     tft_draw_string(WHEATHER_TEMP_FORC3_X, WHEATHER_TEMP_FORC3_Y, "+8 C", ILI9341_COLOR565(0xF8, 0xB4, 0x00), 0x0000, 2);

     if(decode_image(&pixels_image, Wthunderstorm))
     {
        ESP_LOGI("Display", "Can't ionit picture");
     }
     else
     {
         tft_send_picturte(WHEATHER_IMG_FORC4_X, WHEATHER_IMG_FORC4_Y, WEATHER_W, WEATHER_H, pixels_image);
         free_image(&pixels_image, Wthunderstorm);
     }

     tft_draw_string(WHEATHER_TEMP_FORC4_X, WHEATHER_TEMP_FORC4_Y, "+16 C", ILI9341_COLOR565(0xF8, 0xB4, 0x00), 0x0000, 2);

}

void drawWheather()
{
    tft_fill_screen(0x0000);

    stateD = stateWeather;
}

void testMoon()
{
    if(stateD != stateWeatherTeat)
        return;

    uint16_t **pixels_image;

    Image testMoon[] = {    Moon_L0,
                            Moon_L1,
                            Moon_L2,
                            Moon_L3,
                            Moon_L4,
                            Moon_L5,
                            Moon_L6,
                            Moon_L7,
                            Moon_L8,
                            Moon_L9,
                            Moon_L10,
                            Moon_L11,
                            Moon_L12,
                            Moon_L13,
                            Moon_L14,
                            Moon_L15,
                            Moon_L16,
                            Moon_L17,
                            Moon_L18,
                            Moon_L19,
                            Moon_L20,
                            Moon_L21,
                            Moon_L22,
                            Moon_L23,};

    for(int i = 0; i < (sizeof (testMoon)/sizeof (testMoon[0])); i++)
    {
        if(decode_image(&pixels_image, testMoon[i]))
        {
           ESP_LOGI("Display", "Can't ionit picture");
        }
        else
        {
            tft_send_picturte(MOON_X, MOON_Y, MOON_W, MOON_H, pixels_image);
            free_image(&pixels_image, testMoon[i]);
        }
        vTaskDelay(1);
    }
}

void drawMoon(int phase)
{
    if(stateD != stateWeatherTeat)
        return;

    uint16_t **pixels_image;

    Image masMoon[] = {    Moon_L0,
                            Moon_L1,
                            Moon_L2,
                            Moon_L3,
                            Moon_L4,
                            Moon_L5,
                            Moon_L6,
                            Moon_L7,
                            Moon_L8,
                            Moon_L9,
                            Moon_L10,
                            Moon_L11,
                            Moon_L12,
                            Moon_L13,
                            Moon_L14,
                            Moon_L15,
                            Moon_L16,
                            Moon_L17,
                            Moon_L18,
                            Moon_L19,
                            Moon_L20,
                            Moon_L21,
                            Moon_L22,
                            Moon_L23,};

        if(decode_image(&pixels_image, masMoon[phase]))
        {
           ESP_LOGI("Display", "Can't ionit picture");
        }
        else
        {
            tft_send_picturte(MOON_X, MOON_Y, MOON_W, MOON_H, pixels_image);
            free_image(&pixels_image, masMoon[phase]);
        }
}

void drawunfillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, int16_t s, uint16_t color, uint16_t colorBG)
{
    tft_rect(x, y, w, h, colorBG);

    tft_rect(x, y, s, h, color);//верхняя линия
    tft_rect(x + w - s, y, s, h, color);//нижняя
    tft_rect(x, y, w, s, color);//правая
    tft_rect(x, y + h - s, w, s, color);//правая
}

void setTerm(int16_t val)
{
    if(val > TERM_MAX)
        val = val;
    if(val < TERM_MIN)
        val = TERM_MIN;

    if(stateD == stateMainForm)
    {
        /*uint16_t fill = WIDTH_IN_RECT*(val - TERM_MIN)/(TERM_MAX - TERM_MIN);

        st7735_rect(X_TERM - 4, Y_TERM - SLIM + HEIGHT_RECT, WIDTH_IN_RECT + 7, Y_HUMIDITI - SLIM, DEFOULE_COLOR);//clear old value

        st7735_rect(X_TERM, Y_TERM, (WIDTH_IN_RECT - fill), HEIGHT_IN_RECT, DEFOULE_COLOR);
        st7735_rect(X_TERM + (WIDTH_IN_RECT - fill), Y_TERM, fill, HEIGHT_IN_RECT, color);*/

        uint16_t fill = TEMPERATURE_H*(val - TERM_MIN)/(TERM_MAX - TERM_MIN);
        tft_send_picturte(X_TERM, Y_TERM+fill, TEMPERATURE_W, TEMPERATURE_H-fill, pixels_temp0);
        tft_send_picturte(X_TERM, Y_TERM, TEMPERATURE_W, fill, &pixels_temp1[TEMPERATURE_H-fill]);

        char buff[10];
        sprintf(buff, "%3d C", val);
        tft_draw_string(300, 8, buff, /*color*/ILI9341_COLOR565(0xEF, 0xE0, 0x01), DEFOULE_COLOR, 2);
    }
    else if(stateD == stateDipForm)
    {
        uint16_t color = 0x0000;

        if(val >= 30)
            color = COLOR_RED;
        else if(val >= 10)
            color = COLOR_YELLOW;
        else
            color = COLOR_BLUE;

        char buff[20];
        sprintf(buff, "%4d", val);
        tft_draw_string(DIP_X_TERM_VAL, DIP_Y_TERM_VAL, buff, color, DEFOULE_COLOR, 3);
    }
    else if(stateD == stateDip2Form)
    {
        uint16_t color = 0x0000;

        if(val >= 30)
            color = COLOR_RED;
        else if(val >= 10)
            color = COLOR_YELLOW;
        else
            color = COLOR_BLUE;

        tft_rect(LINE3_X + 1, 0, LCD_WIDTH - LINE3_X - 1, LINE1_Y, color);
        tft_draw_char(DIP_X_WORLD_T, DIP_Y_WORLD_T, 't', 0x0000, color, 2);

        char buff[20];
        sprintf(buff, "%4d", val);
        tft_draw_string(DIP_X_TERM_VAL, DIP_Y_TERM_VAL, buff, 0x0000, color, 3);
    }
}

void setHumiditi(uint16_t val)
{//ему принадлежит левая колонка
    if(val > 100)
        val = 100;
    if(stateD == stateMainForm)
    {

        /*st7735_rect(X_HUMIDITI - 4, 0, WIDTH_IN_RECT + 7, Y_HUMIDITI - SLIM, DEFOULE_COLOR);//clear old value

        uint16_t fill = WIDTH_IN_RECT*val/100;
        st7735_rect(X_HUMIDITI, Y_HUMIDITI, (WIDTH_IN_RECT - fill), HEIGHT_IN_RECT, DEFOULE_COLOR);
        st7735_rect(X_HUMIDITI + (WIDTH_IN_RECT - fill), Y_HUMIDITI, fill, HEIGHT_IN_RECT, COLOR_GREEN);*/

        uint16_t fill = HUMIDITY_H*val/100;
        tft_send_picturte(X_HUMIDITI, Y_HUMIDITI+fill, HUMIDITY_W, HUMIDITY_H-fill, pixels_hum0);
        tft_send_picturte(X_HUMIDITI, Y_HUMIDITI, HUMIDITY_W, fill, &pixels_hum1[HUMIDITY_H-fill]);


        char buff[10];
        sprintf(buff, "%3d%%", val);
        tft_draw_string(70, 8, buff, ILI9341_COLOR565(0x4E, 0xAD, 0xC1), DEFOULE_COLOR, 2);
        //st7735_draw_string(50, 10, buff, ILI9341_COLOR565(0x4E, 0xAD, 0xFF), DEFOULE_COLOR, 2);
    }
    else if(stateD == stateDipForm)
    {
        char buff[20];
        sprintf(buff, "%3d%%", val);
        tft_draw_string(DIP_X_HUM_VAL, DIP_Y_HUM_VAL, buff, ILI9341_COLOR565(0x2E, 0x0F, 0xF1), DEFOULE_COLOR, 3);
    }
    else if(stateD == stateDip2Form)
    {
        char buff[20];
        sprintf(buff, "%3d%%", val);
        tft_draw_string(DIP_X_HUM_VAL, DIP_Y_HUM_VAL, buff, 0x0000, COLOR_BLUE, 3);
    }
}

void setPa(uint16_t val)
{
    if(stateD == stateMainForm)
    {
        tft_rect(X_P-71, Y_P, 80, 14, DEFOULE_COLOR);//clear old value

        char buff[20];
        sprintf(buff, "P=%d", val);
        tft_draw_string(val < 1000 ? (X_P + 5*0) : X_P, (Y_P), buff, ILI9341_COLOR565(0x9F, 0x4F, 0x74), DEFOULE_COLOR, 2);
    }
    else if(stateD == stateDipForm)
    {
        char buff[20];
        sprintf(buff, "%4d", val);
        tft_draw_string(DIP_X_P_VAL, DIP_Y_P_VAL, buff, 0x0000, DEFOULE_COLOR, 3);
    }
    else if(stateD == stateDip2Form)
    {
        char buff[20];
        sprintf(buff, "%4d", val);
        tft_draw_string(DIP_X_P_VAL, DIP_Y_P_VAL, buff, 0x0000, COLOR_MAGENTA, 3);
    }
}

void setCO2(uint16_t val)
{
    uint16_t color = COLOR_GREEN;
    if(val > 1100)
        color = COLOR_YELLOW;
    if(val > 1600)
        color = ILI9341_COLOR565(0xFF, 0xFF, 0x00);//orange
    if(val > 2000)
        color = COLOR_RED;

    if(stateD == stateMainForm)
    {

        tft_rect(X_CO2_VAL-76, Y_CO2_VAL, 76, 30, ILI9341_COLOR565(0x00, 0x00, 0x00));
    //    tft_draw_string(X_WORLD_CO2, Y_WORLD_CO2, "CO2", DEFOULE_COLOR, color, 5);

        char buff[20];
        sprintf(buff, "%4d", val);

        tft_draw_string(X_CO2_VAL, Y_CO2_VAL, buff, 0xFFFF, 0x0000, 4);
    }
    else if(stateD == stateDipForm)
    {
        char buff[20];
        sprintf(buff, "%4d", val);

        tft_draw_string(DIP_X_CO2_VAL, DIP_Y_CO2_VAL, buff, color, DEFOULE_COLOR, 8);
    }
    else if(stateD == stateDip2Form)
    {
        char buff[20];
        sprintf(buff, "%4d", val);

        tft_rect(LINE4_X + 1, LINE1_Y + 1, LCD_WIDTH - LINE4_X - 1, LCD_HEIGHT - LINE1_Y - 1, color);
        tft_draw_string(DIP_X_WORLD_CO2, DIP_Y_WORLD_CO2, "CO2", 0x0000, color, 4);

        tft_draw_string(DIP_X_CO2_VAL, DIP_Y_CO2_VAL, buff, 0x0000, color, 8);
    }
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
        tft_send_picturte(20, 20, IM1_W, IM1_H, pixels_im1);
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
        tft_send_picturte(20, 20, IM2_W, IM2_H, pixels_im2);
        free_image(&pixels_im2, Im2);
    }
}

void setWheather(OpenWeather *wheather)
{
    if(stateD != stateWeather)
        return;

    uint16_t **pixels_image;
    Image im;
    im = getImageWheather(*wheather, true);
    if(decode_image(&pixels_image, im))
    {
       ESP_LOGI("Display", "Can't ionit picture");
    }
    else
    {
        tft_send_picturte(WHEATHER_IMG_NOW_X, WHEATHER_IMG_NOW_Y, WEATHERB_W, WEATHERB_H, pixels_image);
        free_image(&pixels_image, im);
    }

    im = getWind(wheather->wind_deg);
    if(decode_image(&pixels_image, im))
    {
       ESP_LOGI("Display", "Can't ionit picture");
    }
    else
    {
        tft_send_picturte(WIND_IMG_NOW_X, WIND_IMG_NOW_Y, WIND_W, WIND_H, pixels_image);
        free_image(&pixels_image, im);
    }

    char buffer[60] = {0};

    sprintf(buffer, "%d m/s", wheather->wind_speed);
    tft_draw_string(WIND_SPEED_NOW_X, WIND_SPEED_NOW_Y, buffer, ILI9341_COLOR565(0xF8, 0xB4, 0x00), 0x0000, 2);

    memset(buffer, 0, 60);
    sprintf(buffer, "%d C", (int)(wheather->temp));
    tft_draw_string(TEMP_NOW_X, TEMP_NOW_Y, buffer, ILI9341_COLOR565(0xFF, 0xFF, 0x00), 0x0000, 4);

    memset(buffer, 0, 60);
    sprintf(buffer, "Feel %d C", (int)(wheather->feels_like));
    tft_draw_string(TEMP_FEEL_X, TEMP_FEEL_Y, buffer, ILI9341_COLOR565(0xF8, 0xB4, 0x00), 0x0000, 2);

    memset(buffer, 0, 60);
    sprintf(buffer, "Hum. %d", (int)(wheather->humidity));
    tft_draw_string(WHEATHER_HUM_X, WHEATHER_HUM_Y, buffer, ILI9341_COLOR565(0xF8, 0xB4, 0x00), 0x0000, 2);

    memset(buffer, 0, 60);
    sprintf(buffer, "Pres. %d mPa/%d mmHg", (int)(wheather->pressure), (int)(wheather->pressure*0.7500617));
    tft_draw_string(WHEATHER_PRES_X, WHEATHER_PRES_Y, buffer, ILI9341_COLOR565(0xF8, 0xB4, 0x00), 0x0000, 2);

    memset(buffer, 0, 60);
    sprintf(buffer, "Vis. %d m", (int)(wheather->visibility));
    tft_draw_string(WHEATHER_VIS_X, WHEATHER_VIS_Y, buffer, ILI9341_COLOR565(0xF8, 0xB4, 0x00), 0x0000, 2);

    memset(buffer, 0, 60);
    sprintf(buffer, "%s", wheather->weatherName);
    tft_draw_string(WHEATHER_DISCRIPT_X, WHEATHER_DISCRIPT_Y, buffer, ILI9341_COLOR565(0xFF, 0xFF, 0x00), 0x0000, 3);
}

void prontTestIm(Image path0, Image path1, Image path2, Image path3, int W, int H0, int H1)
{
    uint16_t **pixels_image;
    if(decode_image(&pixels_image, path0))
    {
       ESP_LOGI("Display", "Can't ionit picture");
    }
    else
    {
        tft_send_picturte(0, H1, W, H0, pixels_image);
        free_image(&pixels_image, path0);
    }

    if(decode_image(&pixels_image, path1))
    {
       ESP_LOGI("Display", "Can't ionit picture");
    }
    else
    {
        tft_send_picturte(W, H1, W, H0, pixels_image);
        free_image(&pixels_image, path1);
    }

    if(decode_image(&pixels_image, path2))
    {
       ESP_LOGI("Display", "Can't ionit picture");
    }
    else
    {
        tft_send_picturte(0, 0, W, H1, pixels_image);
        free_image(&pixels_image, path2);
    }

    if(decode_image(&pixels_image, path3))
    {
       ESP_LOGI("Display", "Can't ionit picture");
    }
    else
    {
        tft_send_picturte(W, 0, W, H1, pixels_image);
        free_image(&pixels_image, path3);
    }
}

void print_imTest1()
{
    stateD = stateTest1;
    prontTestIm(ImTest1_001, ImTest1_002, ImTest1_003, ImTest1_004, IM_TEST_1_W, IM_TEST_1_H, IM_TEST_3_H);
}

void print_imTest2()
{
    stateD = stateTest2;
    prontTestIm(ImTest2_001, ImTest2_002, ImTest2_003, ImTest2_004, IM_TEST_1_W, IM_TEST_1_H, IM_TEST_3_H);
}

void print_imTest3()
{
    stateD = stateTest3;
    prontTestIm(ImTest3_001, ImTest3_002, ImTest3_003, ImTest3_004, IM_TEST_1_W, IM_TEST_1_H, IM_TEST_3_H);
}

void print_imTest4()
{
    stateD = stateTest4;
    prontTestIm(ImTest4_001, ImTest4_002, ImTest4_003, ImTest4_004, IM_TEST_1_W, IM_TEST_1_H, IM_TEST_3_H);
}

void print_imTestTiger()
{
    stateD = stateTestTiger;
    prontTestIm(ImTestTiger_001, ImTestTiger_002, ImTestTiger_003, ImTestTiger_004, IM_TEST_TIGER_W, IM_TEST_TIGER_H, IM_TEST_TIGER_H);
}

void print_imTestNy0()
{
    stateD = stateTestNu0;
    prontTestIm(ImTestNy0_001, ImTestNy0_002, ImTestNy0_003, ImTestNy0_004, IM_TEST_NU0_W, IM_TEST_NU0_H, IM_TEST_NU0_H);
}

void print_imTestNy1()
{
    stateD = stateTestNu1;
    prontTestIm(ImTestNy1_001, ImTestNy1_002, ImTestNy1_003, ImTestNy1_004, IM_TEST_NU1_W, IM_TEST_NU1_H, IM_TEST_NU1_H);
}

void print_imTestNy2()
{
    stateD = stateTestNu2;
    prontTestIm(ImTestNy2_001, ImTestNy2_002, ImTestNy2_003, ImTestNy2_004, IM_TEST_NU2_W, IM_TEST_NU2_12_H, IM_TEST_NU2_34_H);
}

void print_imTestNy3()
{
    stateD = stateTestNu3;
    prontTestIm(ImTestNy3_001, ImTestNy3_002, ImTestNy3_003, ImTestNy3_004, IM_TEST_NU3_W, IM_TEST_NU3_H, IM_TEST_NU3_H);
}

