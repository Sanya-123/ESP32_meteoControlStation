#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "ascii_font.h"
#include "gpioDEF.h"

#define OFFSET_X    /*0x02*/0
#define OFFSET_Y    /*0x01*/0

#define CONFIG_ST7735_HOST_VSPI
//#define CONFIG_USE_COLOR_RBG565

#ifdef CONFIG_ST7735_HOST_VSPI
#define PIN_NUM_MISO GPIO_SPI_MISO
#define PIN_NUM_MOSI GPIO_SPI_MOSI  // SDA
#define PIN_NUM_CLK GPIO_SPI_CLK
#define PIN_NUM_CS GPIO_CS_LDC

#define PIN_NUM_DC GPIO_LCD_DC
#define PIN_NUM_RST GPIO_LCD_RST
#define PIN_NUM_LED GPIO_LCD_LED
#elif CONFIG_ST7735_HOST_HSPI
#define PIN_NUM_MISO -1
#define PIN_NUM_MOSI 13  // SDA
#define PIN_NUM_CLK 14
#define PIN_NUM_CS 15

#define PIN_NUM_DC 4
#define PIN_NUM_RST 2
#endif

// LCD backlight contorl
#define PIN_NUM_BCKL /*CONFIG_ST7735_BL_PIN*/-1

#ifdef CONFIG_USE_COLOR_RBG565 // R-B-G 5-6-5
// Some ready-made 16-bit (RBG-565) color settings:
#define	COLOR_BLACK      0x0000
#define COLOR_WHITE      0xFFFF
#define	COLOR_RED        0xF800
#define	COLOR_GREEN      0x001F
#define	COLOR_BLUE       0x07E0
#define COLOR_CYAN       0x07FF
#define COLOR_MAGENTA    0xFFE0
#define COLOR_YELLOW     0xF81F
#define	COLOR_GRAY       0x8410
#define	COLOR_OLIVE      0x8011
#else // R-G-B 5-6-5
// Some ready-made 16-bit (RGB-565) color settings:
#define	COLOR_BLACK      0x0000
#define COLOR_WHITE      0xFFFF
#define	COLOR_RED        0xF800
#define	COLOR_GREEN      0x001F
#define	COLOR_BLUE       0x07E0
#define COLOR_CYAN       0x07FF
#define COLOR_MAGENTA    0xFFE0
#define COLOR_YELLOW     0xF81F
#define	COLOR_GRAY       0x8410
#define	COLOR_OLIVE      0x8011
//#define	COLOR_OLIVE      0x8400
#endif

#define ST7735_COLOR565(r, g, b) (((r & 0x3F) << 5) | ((g & 0x1F) << 0) | ((b & 0x1F) << 11))
#define ILI9341_COLOR565(r, g, b) ((((r >> 2) & 0x3F) << 5) | (((g >> 3) & 0x1F) << 0) | (((b >> 3) & 0x1F) << 11))

void tft_init();
void tft_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void tft_invert_color(int i);
void tft_fill_screen(uint16_t color);
uint32_t tft_draw_string(uint16_t x, uint16_t y, const char *pt, int16_t color, int16_t bg_color, uint8_t size);
void tft_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color, uint8_t size);
void send_picturte(int xpos, int ypos, int W, int H, uint16_t **picture);
void read_picturte(int xpos, int ypos, int W, int H, uint16_t **picture);

//void ST7735_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
