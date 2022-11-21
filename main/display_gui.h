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

/*********************
 *      DEFINES
 *********************/
#define MIN_TIME_ON_DISPLAY     1
#define MAX_TIME_ON_DISPLAY     200

/**********************
 *      TYPEDEFS
 **********************/
typedef enum{
    StateMainDisplay = 0,
    StateExternalData = 1,
    StateWheather = 2,
    StateSetings = 40,
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

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void drowDisplayLVGL(void);
void meteoButtonClicked(Button button, ButtonState state);
void setCO2(int val);
void setHumiditi(int val);
void setTemperature(int val);
void setPressure(int val);
void setTime(int h, int m);
void setDate(int d, int m, int y);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DISPLAY_GUI_H*/
