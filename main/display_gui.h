/**
 * @file display_gui.h
 *
 */

#ifndef DISPLAY_GUI_H
#define DISPLAY_GUI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/
#define DISPLAY_SIZE            5 /*size data displays*/
#define MIN_TIME_ON_DISPLAY     1
#define MAX_TIME_ON_DISPLAY     200
#define SIZE_EXT_DATA           4/*size ext data on 1 display*/
#define USE_LOGO                0

/**********************
 *      TYPEDEFS
 **********************/
typedef enum{
    StateMainDisplay = 0,
    StateMainDisplay2 = 1,
    StateMainDisplay3 = 2,
    StateExtDisplay = 3,
    StateWheather = 4,
    StateSetings = 40,
    StateQRcode = 41,
    StateURL = 42,
}DisplayState;

typedef enum{
    ButtonPrev = 0,
    ButtonSettings,
    ButtonAction,
    ButtonNext,
    NoneButton = 255
}Button;

typedef enum{
    ButtonRealesed = 0,
    ButtonPresed = 1,
}ButtonState;

typedef struct
{
    uint32_t timeOnDisplay[DISPLAY_SIZE];//time stay on display (if autoChangeDisplay == true);
    bool autoChangeDisplay;//if true switch display on timer
    bool usemmHg;//false - hPa;true - mmHg
    uint8_t style;//0-6 theme color
    bool darkTheme;
}SettignDisplay;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void drowDisplayLVGL(void);
void meteoButtonClicked(Button button, ButtonState state);
void loadConfig(SettignDisplay setings);
//set event save display config
void setSaveConfig(void (*saveConf)(SettignDisplay setings));
//set event add external device
void setAddExternal(void (*addExternal)(void));
//set data
void setCO2(int val);
void setHumiditi(int val);
void setTemperature(int val);
void setPressure(int val);
void setTime(int h, int m);
void setDate(int d, int m, int y);
void setWifiConnect(bool ok);
void setEnableQrPage(bool en);
void setCurentAPQrData(char *data, int len);
void setDPPQrData(char *data, int len);
void setIPdev(char *data);
void seturlQrData(char *data, int len);
//ex sensors data
void setExtName(int numSensor, char *name);
void setExtTemp(int numSensor, int data);
void setExtHumm(int numSensor, int data);
void setExtBat(int numSensor, int data);


/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DISPLAY_GUI_H*/
