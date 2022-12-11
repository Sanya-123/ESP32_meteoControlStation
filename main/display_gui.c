/**
 * @file display_gui.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "display_gui.h"
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "../lvgl/lvgl.h"
#endif

//TODO use my
#include "images.h"


/*********************
 *      DEFINES
 *********************/
#undef LV_HOR_RES
#define LV_HOR_RES 320

#undef LV_VER_RES
#define LV_VER_RES 240

#define BUTTON_HEIGHT       30
#define BORDER_BUTTONS      2

//size rectangle for mersurment data
#define SMALL_RECT_HEIGHT   70
#define SMALL_RECT_WIDTH    110
#define BIG_RECT_HEIGHT     140
#define BIG_RECT_WIDTH      210

#define SITING_LIST_SIZE    10

#define MAX_CO2             5000
#define MIN_TEMP            -50
#define MAX_TEMP            +50

//range data color
#define TEMP_COMFORT_MIN    16
#define TEMP_COMFORT_MAX    26

#define HUM_COMFORT_MIN     40
#define HUM_COMFORT_MAX     70

#define D2_COLOR_BLUE       0xD5E6FB
#define D2_COLOR_GREEN      0xD3EADD
#define D2_COLOR_YELLOW     0xFBF3D3
#define D2_COLOR_ORNAGE     0xFCEBDB
#define D2_COLOR_RED        0xFBDDDD


#define STYLE_SIZE          7

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
static void zoomLogoAnimation(void * obj, int32_t zoom);
static void readKeyButton(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static void btnSide_event_cb(lv_event_t * e);//button left and right
static void btnCtrl_event_cb(lv_event_t * e);//button conf
static void btnAction_event_cb(lv_event_t * e);//button action
static void saveSettings();
static void timer_event_cb(lv_timer_t * timer);
static void mainPage_create(lv_obj_t * parent);
static void mainPage2_create(lv_obj_t * parent);
static void mainPage3_create(lv_obj_t * parent);
static void mainPage4_create(lv_obj_t * parent);
static void extDataPage_create(lv_obj_t * parent, lv_obj_t **_tabel);
static void weatherPage_create(lv_obj_t * parent);

static void qrCodePage_create(lv_obj_t * parent);
static void urlPage_create(lv_obj_t * parent);
static void settingPage_create(lv_obj_t * parent);
static void color_changer_create(lv_obj_t * parent);

/**********************
 *  STATIC VARIABLES
 **********************/
//butons
static lv_indev_t *indev;
static lv_indev_drv_t indev_drv;
static Button buttonChange = NoneButton;
static ButtonState buttonState = ButtonRealesed;
//static ButtonState buttonState[ButtonNext + 1] = {ButtonRealesed};

static DisplayState stateDisplay = StateMainDisplay;
static uint32_t timeOnDisplay[DISPLAY_SIZE] = {3, 3, 3, 3, 3, 3};//time state on display [s]

static lv_obj_t * buttonLeft, * buttonRight, * buttonSettings, * buttonAction;
static lv_obj_t * labelLeft, * labelRight, * labelSettings, * labelAction;

static lv_style_t styleButtons/*, styleButtonRight*/;

static lv_font_t * fontMain;
static lv_font_t * fontData;

static lv_obj_t * tl;

static lv_timer_t * timerNextDisplay;//display timer

//mainDisplay
static lv_obj_t * displayMain;
static lv_obj_t * co2Obj, * humidityObj, * temperatureObj, * pressureObj;
static lv_obj_t * labelCo2, * labelHumidity, * labelTemperatre, * labelPressure;
static lv_style_t styleDisplayMain;
static lv_obj_t * labelTime, * labelDate;
static lv_obj_t * ledWifi;
//static lv_style_t styleTime, styleDate;

//second display
static lv_obj_t * displayMain2;
static lv_obj_t * co2Obj2, * humidityObj2, * temperatureObj2, * pressureObj2, * timeDateObj2;
static lv_obj_t * labelCo22, * labelHumidity2, * labelTemperatre2, * labelPressure2;
static lv_obj_t * labelCo2ppm2/*, * labelCo2co22*/;
static lv_style_t styleDisplayMain2;
static lv_obj_t * labelTime2, * labelDate2;
static lv_obj_t * ledWifi2;

//therd display
static lv_obj_t * displayMain3;
static lv_obj_t * meterCo23, * meterHumidity3, * meterTemperature3;
static lv_meter_indicator_t * indicCo23, * indicHumidity3, * indicTemperature3;
static lv_obj_t * labelCo23, * labelHumidity3, * labelTemperatre3/*, * labelPressure3*/;

//4 display
static lv_obj_t * displayMain4;
static lv_obj_t * meterData4;
static lv_meter_indicator_t * indicCo24, * indicHumidity4, * indicTemperature4;
static lv_obj_t * labelCo24, * labelHumidity4, * labelTemperatre4, * labelPressure4;
static lv_obj_t * labelTime4, * labelDate4;

//display extData
static lv_obj_t * displayExtData;
static lv_obj_t * tableExtData;

//weather display
static lv_obj_t * displayWeather;
static lv_obj_t * wMainImage;
static lv_obj_t * wMainTemperature, * wFellTemperature;
static lv_obj_t * wHummidity;
static lv_obj_t * wTime;
static lv_obj_t * wLastUpdate;
static lv_obj_t * wForcastHTemp[SIZE_FORCAST] = {NULL};
static lv_obj_t * wForcastHImage[SIZE_FORCAST] = {NULL};
static lv_obj_t * wForcastDTemp[SIZE_FORCAST] = {NULL};
static lv_obj_t * wForcastDImage[SIZE_FORCAST] = {NULL};

//sitings display
static lv_obj_t * displaySettings;
static lv_obj_t * switchTimerEn;
static lv_obj_t * switchPressure;//hPa <-> mmHg
static lv_obj_t * switchTheme;
static lv_group_t * settingFocuse;
static lv_obj_t * btnColorSelect;
static lv_obj_t * colorCont;
bool stateConfigValue = false;//state load int value of settings
bool selectColorState = false;
static lv_obj_t * labelTimeDisplay[DISPLAY_SIZE] = {NULL};
int currentLabelTimeDisplay = -1;
void (*saveConfFunction)(SettignDisplay setings) = NULL;//
void (*addExternalFunction)(void) = NULL;//event add new external device
//bool pressurePring = false;//0 - hPa;1 - mmHg

static lv_group_t * colorFocuse;

//static int indexSelectedSettings = 0;
//static lv_obj_t * selectedSetings;//curent select sitings in listSetings
//static lv_obj_t * listSetings[SITING_LIST_SIZE] = {NULL/*switchTimerEn*/};
static lv_style_t styleListSettings;

//display qrcode
static lv_obj_t * displayQRcode;
static lv_obj_t * qrCodeThisAP, * qrCodeConnectToAp;
static bool enableShowQr = true;//show wifi qr on action button clicck

//display url
static lv_obj_t * displayURL;
static lv_obj_t * labelDeviceIP;
static lv_obj_t * qrCodeURL;


/**********************
 *      MACROS
 **********************/
#define CREATE_BUTTON(btn, label, x) do{ btn = lv_obj_create(lv_scr_act());   \
                                        label = lv_label_create(btn);   \
                                        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE); \
                                        lv_obj_center(label);   \
                                        lv_obj_add_style(btn, &styleButtons, 0); \
                                        lv_obj_set_size(btn, LV_HOR_RES/4 /*+ border_buttons*/, BUTTON_HEIGHT);  \
                                        lv_obj_set_pos(btn, LV_HOR_RES/4*x/* + ((x-2)*border_buttons/2)*/, LV_VER_RES - BUTTON_HEIGHT /*+ border_buttons*/);}while(0);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void drowDisplayLVGL(void)
{
    //add device for read buttons
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = readKeyButton;
    indev_drv.type = LV_INDEV_TYPE_BUTTON;
    indev = lv_indev_drv_register(&indev_drv);
    //registarete input device

    //set points on display for buttons device in the middle of buttons on display
    static lv_point_t points_array[] = { { LV_HOR_RES/8, LV_VER_RES - BUTTON_HEIGHT/2 },
                                       { LV_HOR_RES/8*3, LV_VER_RES - BUTTON_HEIGHT/2 },
                                       { LV_HOR_RES/8*5, LV_VER_RES - BUTTON_HEIGHT/2 },
                                       { LV_HOR_RES/8*7, LV_VER_RES - BUTTON_HEIGHT/2 }};
    lv_indev_set_button_points(indev, points_array);

    fontMain = (lv_font_t *)LV_FONT_DEFAULT;
    fontData = (lv_font_t *)LV_FONT_DEFAULT;

#if LV_FONT_MONTSERRAT_14
        fontMain    =  (lv_font_t *)&lv_font_montserrat_14;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_16 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.")
#endif

#if LV_FONT_MONTSERRAT_16
        fontData    =  (lv_font_t *)&lv_font_montserrat_16;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_16 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.")
#endif

#if LV_USE_THEME_DEFAULT
    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, fontMain);
#endif

    //init style
    // lv_color_t colorText = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF);
    lv_style_init(&styleButtons);
    //text
    lv_style_set_text_color(&styleButtons, lv_color_make(0xFF, 0xFF, 0xFF));
    lv_style_set_text_font(&styleButtons, fontMain);
    lv_style_set_text_opa(&styleButtons, LV_OPA_100);
    //radius
    lv_style_set_radius(&styleButtons, 0);
    //border style
    lv_style_set_border_width(&styleButtons, BORDER_BUTTONS);
    lv_style_set_border_side(&styleButtons, LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_RIGHT);
    lv_style_set_border_color(&styleButtons, lv_color_make(0xF6, 0xF6, 0xF6));
    lv_style_set_bg_color(&styleButtons, lv_theme_get_color_primary(NULL));
//    LV_OPA_TRANSP;

//    lv_style_set_opa(&styleButtons, LV_OPA_100);
//    LV_OPA_COVER;

//    lv_style_init(&styleButtonRight);
//    lv_style_set_text_color(&styleButtonRight, lv_color_make(0xFF, 0xFF, 0xFF));
//    lv_style_set_radius(&styleButtonRight, 0);
//    lv_style_set_border_width(&styleButtonRight, BORDER_BUTTONS);
//    lv_style_set_border_side(&styleButtonRight, LV_BORDER_SIDE_TOP);

    // lv_style_init(&style_bullet);
    // lv_style_set_border_width(&style_bullet, 0);
    // lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

    //remove sliders
    lv_obj_remove_style(lv_scr_act(), NULL, LV_PART_SCROLLBAR /*| LV_STATE_ANY*/);

    //create page changer
    tl = lv_tileview_create(lv_scr_act());
    lv_obj_set_pos(tl, 0, 0);
    lv_obj_set_size(tl, LV_HOR_RES, LV_VER_RES - BUTTON_HEIGHT + 1);
    lv_obj_remove_style(tl, NULL, LV_PART_SCROLLBAR /*| LV_STATE_ANY*/);

    //init buttons
    CREATE_BUTTON(buttonLeft, labelLeft, 0);
    CREATE_BUTTON(buttonSettings, labelSettings, 1);
    CREATE_BUTTON(buttonAction, labelAction, 2);
    CREATE_BUTTON(buttonRight, labelRight, 3);

    lv_obj_set_style_border_side(buttonRight, LV_BORDER_SIDE_TOP, 0);

    lv_label_set_text(labelLeft, "<");
    lv_label_set_text(labelRight, ">");
    lv_label_set_text(labelSettings, "conf");
    lv_label_set_text(labelAction, "add");

    static uint8_t typeLeft = 1, typeRight = 0;

    lv_obj_add_event_cb(buttonLeft, btnSide_event_cb, LV_EVENT_ALL, &typeLeft);
    lv_obj_add_event_cb(buttonRight, btnSide_event_cb, LV_EVENT_ALL, &typeRight);
    lv_obj_add_event_cb(buttonSettings, btnCtrl_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(buttonAction, btnAction_event_cb, LV_EVENT_ALL, NULL);

    //add pages
    /*main display*/
//    lv_obj_set_style_bg_color(tl, lv_color_hex(0xFFFFFF), 0);

    displayMain = lv_tileview_add_tile(tl, StateMainDisplay, 0, LV_DIR_NONE);
    mainPage_create(displayMain);

    displayMain2 = lv_tileview_add_tile(tl, StateMainDisplay2, 0, LV_DIR_NONE);
    mainPage2_create(displayMain2);

    displayMain3 = lv_tileview_add_tile(tl, StateMainDisplay3, 0, LV_DIR_NONE);
    mainPage3_create(displayMain3);

    displayMain4 = lv_tileview_add_tile(tl, StateMainDisplay4, 0, LV_DIR_NONE);
    mainPage4_create(displayMain4);

    displayExtData = lv_tileview_add_tile(tl, StateExtDisplay, 0, LV_DIR_NONE);
    extDataPage_create(displayExtData, &tableExtData);

    displayWeather = lv_tileview_add_tile(tl, StateWheather, 0, LV_DIR_NONE);
    weatherPage_create(displayWeather);



    //setings page
    displaySettings = lv_tileview_add_tile(tl, 0, 1, LV_DIR_NONE);
    settingPage_create(displaySettings);

    //create timer for change display
    timerNextDisplay = lv_timer_create(timer_event_cb, timeOnDisplay[0]*1000, NULL);
    lv_timer_pause(timerNextDisplay);//TODO temp

    displayQRcode = lv_tileview_add_tile(tl, 0, 2, LV_DIR_NONE);
    qrCodePage_create(displayQRcode);

    displayURL = lv_tileview_add_tile(tl, 0, 3, LV_DIR_NONE);
    urlPage_create(displayURL);


//    lv_obj_t * mbox1 = lv_msgbox_create(lv_scr_act(), "Error", "Couldn't connect to AP", NULL, false);
//    lv_obj_center(mbox1);
////    lv_obj_fade_in(mbox1, 600, 2000);
////    lv_obj_fade_out(mbox1, 600, 4000);
//    lv_anim_t a2;
//    lv_anim_set_path_cb(&a2, lv_anim_path_ease_in);
//    lv_anim_set_var(&a2, mbox1);
//    lv_anim_set_time(&a2, 600);
//    lv_anim_set_delay(&a2, 4000);
//    lv_anim_set_values(&a2, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE*8);
//    lv_anim_set_ready_cb(&a2, lv_obj_del_anim_ready_cb);
//    lv_anim_start(&a2);

    lv_png_init();
#if USE_LOGO
    //draw logo prewie

    LV_IMG_DECLARE(logo);
//    extern lv_img_dsc_t logo;
    lv_obj_t * logoImage = lv_img_create(lv_scr_act());
    lv_img_set_src(logoImage, &logo);

//    lv_img_decoder_create()

//    lv_obj_move_foreground(logoImage);
    lv_obj_move_background(logoImage);
    lv_obj_center(logoImage);

    lv_img_set_zoom(logoImage, LV_IMG_ZOOM_NONE/4);
//    lv_obj_set_style_img_opa(logoImage, LV_OPA_50, 0);

    lv_obj_fade_in(tl, 600, 1000);
    lv_obj_fade_in(buttonLeft, 600, 1000);
    lv_obj_fade_in(buttonRight, 600, 1000);
    lv_obj_fade_in(buttonSettings, 600, 1000);
    lv_obj_fade_in(buttonAction, 600, 1000);

    //startr zoom animation
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_var(&a, logoImage);
    lv_anim_set_time(&a, 400);
    lv_anim_set_delay(&a, 1000);
    lv_anim_set_values(&a, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE*8);
    lv_anim_set_exec_cb(&a, zoomLogoAnimation);
//    lv_anim_set_exec_cb(&a, lv_img_set_zoom);//thats work but warning
    lv_anim_set_ready_cb(&a, lv_obj_del_anim_ready_cb);
    lv_anim_start(&a);
#endif


    //test data
    setCO2(2700);
    setHumiditi(50);
    setTemperature(14);
    setPressure(1015);
    setExtName(3, "Ground");
    setExtTemp(0, 35);
    setExtTemp(2, -15);
    setExtHumm(3, 65);
    setExtBat(1, 99);
    setWifiConnect(true);
}

void meteoButtonClicked(Button button, ButtonState state)
{
//    if(button <= ButtonNext)
//        buttonState[button] = state;
    buttonChange = button;
    buttonState = state;
}

void loadConfig(SettignDisplay setings)
{
    for(uint32_t i = 0; i < DISPLAY_SIZE; i++)
    {
        timeOnDisplay[i] = setings.timeOnDisplay[i];
        if(timeOnDisplay[i] > MAX_TIME_ON_DISPLAY)
            timeOnDisplay[i] = MAX_TIME_ON_DISPLAY;
        if(labelTimeDisplay[i] != NULL)
            lv_label_set_text_fmt(labelTimeDisplay[i], "%d", timeOnDisplay[i]);
    }

    if(setings.autoChangeDisplay)
        lv_obj_add_state(switchTimerEn, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(switchTimerEn, LV_STATE_CHECKED);

    if(setings.usemmHg)
        lv_obj_add_state(switchPressure, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(switchPressure, LV_STATE_CHECKED);

    if(setings.darkTheme)
        lv_obj_add_state(switchTheme, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(switchTheme, LV_STATE_CHECKED);

    lv_event_send(switchTimerEn, LV_EVENT_VALUE_CHANGED, NULL);
    lv_event_send(switchPressure, LV_EVENT_VALUE_CHANGED, NULL);
    lv_event_send(switchTheme, LV_EVENT_VALUE_CHANGED, NULL);

    if(setings.style >= STYLE_SIZE)
        setings.style = STYLE_SIZE - 1;

    //refocs to object (or change select color)
    lv_group_focus_obj(lv_obj_get_child(colorCont, setings.style));
}

void setSaveConfig(void (*saveConf)(SettignDisplay setings))
{
    saveConfFunction = saveConf;
}

void setAddExternal(void (*addExternal)(void))
{
    addExternalFunction = addExternal;
}

void setCO2(int val)
{
    if(val > MAX_CO2)
        val = MAX_CO2;

    //display 1
    lv_label_set_text_fmt(labelCo2, "%dppm", val);
    if(val >= CO2_BEGIN_RED)
        lv_obj_set_style_bg_color(co2Obj, lv_palette_main(LV_PALETTE_DEEP_ORANGE), 0);
    else if(val >= CO2_BEGIN_ORANG)
        lv_obj_set_style_bg_color(co2Obj, lv_palette_main(LV_PALETTE_ORANGE), 0);
    else if(val >= CO2_BEGIN_YELLOW)
        lv_obj_set_style_bg_color(co2Obj, lv_palette_main(LV_PALETTE_YELLOW), 0);
    else
        lv_obj_set_style_bg_color(co2Obj, lv_palette_main(LV_PALETTE_LIGHT_GREEN), 0);

    //display 2
    lv_label_set_text_fmt(labelCo22, "%d", val);
//    lv_obj_align_to(labelCo2co22, labelCo22, LV_ALIGN_OUT_LEFT_BOTTOM, -5, -6);
    lv_obj_align_to(labelCo2ppm2, labelCo22, LV_ALIGN_OUT_RIGHT_BOTTOM, +3, -6);

    if(val >= CO2_BEGIN_RED)
        lv_obj_set_style_bg_color(co2Obj2, lv_color_hex(D2_COLOR_RED), 0);
    else if(val >= CO2_BEGIN_ORANG)
        lv_obj_set_style_bg_color(co2Obj2, lv_color_hex(D2_COLOR_ORNAGE), 0);
    else if(val >= CO2_BEGIN_YELLOW)
        lv_obj_set_style_bg_color(co2Obj2, lv_color_hex(D2_COLOR_YELLOW), 0);
    else
        lv_obj_set_style_bg_color(co2Obj2, lv_color_hex(D2_COLOR_GREEN), 0);

    //display 3
    if(val >= CO2_BEGIN_RED)
        indicCo23->type_data.arc.color = lv_palette_main(LV_PALETTE_RED);
    else if(val >= CO2_BEGIN_ORANG)
        indicCo23->type_data.arc.color = lv_palette_main(LV_PALETTE_ORANGE);
    else if(val >= CO2_BEGIN_YELLOW)
        indicCo23->type_data.arc.color = lv_palette_main(LV_PALETTE_YELLOW);
    else
        indicCo23->type_data.arc.color = lv_palette_main(LV_PALETTE_GREEN);

    lv_meter_set_indicator_end_value(meterCo23, indicCo23, val);

    lv_label_set_text_fmt(labelCo23, "%d", val);

    //display 4
    if(val >= CO2_BEGIN_RED)
        indicCo24->type_data.arc.color = lv_palette_main(LV_PALETTE_RED);
    else if(val >= CO2_BEGIN_ORANG)
        indicCo24->type_data.arc.color = lv_palette_main(LV_PALETTE_ORANGE);
    else if(val >= CO2_BEGIN_YELLOW)
        indicCo24->type_data.arc.color = lv_palette_main(LV_PALETTE_YELLOW);
    else
        indicCo24->type_data.arc.color = lv_palette_main(LV_PALETTE_GREEN);

    lv_meter_set_indicator_end_value(meterData4, indicCo24, val);

    lv_label_set_text_fmt(labelCo24, "%d", val);
}

void setHumiditi(int val)
{
    lv_label_set_text_fmt(labelHumidity, "%d%%", val);

    //display 2
    lv_label_set_text_fmt(labelHumidity2, "%3d %%", val);
    lv_label_set_text_fmt(labelTemperatre2, "%3d C", val);
    if(val < HUM_COMFORT_MIN)
        lv_obj_set_style_bg_color(humidityObj2, lv_color_hex(D2_COLOR_RED), 0);
    else if(val > HUM_COMFORT_MAX)
        lv_obj_set_style_bg_color(humidityObj2, lv_color_hex(D2_COLOR_BLUE), 0);
    else
        lv_obj_set_style_bg_color(humidityObj2, lv_color_hex(D2_COLOR_GREEN), 0);

    //display 3
    lv_meter_set_indicator_end_value(meterHumidity3, indicHumidity3, val);
    lv_label_set_text_fmt(labelHumidity3, "%d", val);

    //display 4
    lv_meter_set_indicator_end_value(meterData4, indicHumidity4, val);
    lv_label_set_text_fmt(labelHumidity4, "%d", val);
}

void setTemperature(int val)
{
    if(val < MIN_TEMP)
        val = MIN_TEMP;
    if(val > MAX_TEMP)
        val = MAX_TEMP;

    lv_label_set_text_fmt(labelTemperatre, "%3dC", val);

    //display 2
    lv_label_set_text_fmt(labelTemperatre2, "%3d C", val);
    if(val < TEMP_COMFORT_MIN)
        lv_obj_set_style_bg_color(temperatureObj2, lv_color_hex(D2_COLOR_BLUE), 0);
    else if(val > TEMP_COMFORT_MAX)
        lv_obj_set_style_bg_color(temperatureObj2, lv_color_hex(D2_COLOR_RED), 0);
    else
        lv_obj_set_style_bg_color(temperatureObj2, lv_color_hex(D2_COLOR_GREEN), 0);

    //display 3
    lv_meter_set_indicator_end_value(meterTemperature3, indicTemperature3, val);
    lv_label_set_text_fmt(labelTemperatre3, "%d", val);

    //display 3
    lv_meter_set_indicator_start_value(meterData4, indicTemperature4, val);
    lv_label_set_text_fmt(labelTemperatre4, "%d", val);
}

void setPressure(int val)
{
    //if value is -1 so i need just update hPa <-> nnHg
    static int oldVal = 1045;
    if(val == -1)
        val = oldVal;

    int printVal = lv_obj_has_state(switchPressure, LV_STATE_CHECKED) ? val*0.750062 : val;
    lv_label_set_text_fmt(labelPressure, "%d%s", printVal, lv_obj_has_state(switchPressure, LV_STATE_CHECKED) ? "mmHg" : "hPa");

    lv_label_set_text_fmt(labelPressure2, "%d %s", printVal, lv_obj_has_state(switchPressure, LV_STATE_CHECKED) ? "mmHg" : "hPa");

//    lv_label_set_text_fmt(labelPressure3, "%d %s", printVal, lv_obj_has_state(switchPressure, LV_STATE_CHECKED) ? "mmHg" : "hPa");

    lv_label_set_text_fmt(labelPressure4, "P %d %s", printVal, lv_obj_has_state(switchPressure, LV_STATE_CHECKED) ? "mmHg" : "hPa");

    oldVal = val;
}

void setTime(int h, int m)
{
    lv_label_set_text_fmt(labelTime, "%d:%d", h, m);

    lv_label_set_text_fmt(labelTime2, "%d:%d", h, m);

    lv_label_set_text_fmt(labelTime4, "%d:%d", h, m);

    lv_label_set_text_fmt(wTime, "%d:%d", h, m);
}

void setDate(int d, int m, int y)
{
    lv_label_set_text_fmt(labelDate, "%d.%d.%d", d, m, y);

    lv_label_set_text_fmt(labelDate2, "%d.%d.%d", d, m, y);

    lv_label_set_text_fmt(labelDate4, "%d.%d.%d", d, m, y);
}

void setWifiConnect(bool ok)
{
    if(ok)
        lv_led_on(ledWifi);
    else
        lv_led_off(ledWifi);

    if(ok)
        lv_led_set_color(ledWifi2, lv_palette_lighten(LV_PALETTE_LIGHT_GREEN, 2));
    else
        lv_led_set_color(ledWifi2, lv_palette_lighten(LV_PALETTE_RED, 2));

    //TODO shwo messadge
}

void setWifiLedOn(bool en)
{
    if(en)
        lv_led_on(ledWifi2);
    else
        lv_led_off(ledWifi2);
}

void setEnableQrPage(bool en)
{
    enableShowQr = en;
}

void setCurentAPQrData(char *data, int len)
{
    lv_qrcode_update(qrCodeThisAP, data, len);
}

void setDPPQrData(char *data, int len)
{
    lv_qrcode_update(qrCodeConnectToAp, data, len);
}

void setIPdev(char *data)
{
    lv_label_set_text_fmt(labelDeviceIP, "IP:%s", data);
}

void seturlQrData(char *data, int len)
{
    lv_qrcode_update(qrCodeURL, data, len);
}

void setExtData(int numSensor, int numData, void *data)
{
    if(numSensor >= SIZE_EXT_DATA)
        return;

    if(numData == 0)
        lv_table_set_cell_value(tableExtData, numSensor + 1, numData, (char*)data);
    else
        lv_table_set_cell_value_fmt(tableExtData, numSensor + 1, numData, "%d", *((int*)data));
}

void setExtName(int numSensor, char *name)
{
    setExtData(numSensor, 0, name);
}

void setExtTemp(int numSensor, int data)
{
    if(data > MAX_TEMP)
        data = MAX_TEMP;
    if(data < MIN_TEMP)
        data = MIN_TEMP;
    setExtData(numSensor, 1, &data);
}

void setExtHumm(int numSensor, int data)
{
    if(data > 100)
        data = 100;
    setExtData(numSensor, 2, &data);
}

void setExtBat(int numSensor, int data)
{
    if(data > 100)
        data = 100;
    setExtData(numSensor, 3, &data);
}

void setWeatherCurentTemp(int temp)
{
    lv_label_set_text_fmt(wMainTemperature, "%+d", temp);
}

void setWeatherCurentTempFell(int temp)
{
    lv_label_set_text_fmt(wFellTemperature, "Feel %+d", temp);
}

void setWeatherCurentHummidity(int hum)
{
    lv_label_set_text_fmt(wHummidity, "H:%3d%%", hum);
}

void setWeatherCurentPicture(int num)
{
    lv_img_set_src(wMainImage, lv_weather[num].img_dsc);
}

void setWeatherUpdateTime(int h, int m)
{
    lv_label_set_text_fmt(wLastUpdate, "update:%d:%d", h, m);
}

void setForcastHTemp(unsigned int f, int temp)
{
    if(f < SIZE_FORCAST)
    {
        lv_label_set_text_fmt(wForcastHTemp[f], "%+d(+%dh)", temp, STEP_FORCAST_HOUR*(f + 1));
    }
}

void setForcastHPicture(unsigned int f, int num)
{
    if(f < SIZE_FORCAST)
    {
        lv_img_set_src(wForcastHImage[f], lv_weather[num].img_dsc);
    }
}

void setForcastDTemp(unsigned int f, int temp)
{
    if(f < SIZE_FORCAST)
    {
        lv_label_set_text_fmt(wForcastDTemp[f], "%+d(+%dd)", temp, (f + 1));
    }
}

void setForcastDPicture(unsigned int f, int num)
{
    if(f < SIZE_FORCAST)
    {
        lv_img_set_src(wForcastDImage[f], lv_weather[num].img_dsc);
    }
}

DisplayState getDisplay(bool next)//get statte after current
{
    DisplayState new = StateMainDisplay;
    if(next)
    {
        if(stateDisplay == StateWheather)
            new = StateMainDisplay;
        else
            new = stateDisplay + 1;
    }
    else
    {
        if(stateDisplay == StateMainDisplay)
            new = StateWheather;
        else
            new = stateDisplay - 1;
    }

    return new;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void zoomLogoAnimation(void * obj, int32_t zoom)
{
    lv_img_set_zoom((lv_obj_t *)obj, (uint16_t)zoom);
}

//andler analize input buttons
static void readKeyButton(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void)indev_drv;

//    static int i = 0;

//    data->continue_reading = i < ButtonNext;
//    data->btn_id = i;
//    data->state = buttonState[i] == ButtonPresed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;


//    if(i == ButtonNext)
//        i = 0;
//    else
//        i++;


    static Button last_btn = NoneButton;     /* Store the last pressed button */
    static ButtonState lastState = ButtonRealesed;

    if(buttonChange != NoneButton)
    {
        data->btn_id = buttonChange;
        data->state = buttonState == ButtonPresed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;

        last_btn = buttonChange;
        lastState = buttonState;
        buttonChange = NoneButton;
    }
    else
    {
        data->btn_id = last_btn;
        data->state = lastState == ButtonPresed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    }

    data->continue_reading = false;

}

static void btnSide_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    // lv_obj_t * btn = lv_event_get_target(e);
    void *param = lv_event_get_user_data(e);
    uint8_t typeBytton = *((uint8_t*)param);

    if(stateDisplay != StateSetings)
    {
        if(code == LV_EVENT_CLICKED)
        {
            stateDisplay = getDisplay(typeBytton == 0);

            lv_timer_set_period(timerNextDisplay, timeOnDisplay[stateDisplay]*1000);//set period display
            lv_timer_reset(timerNextDisplay);//reset count timer
            lv_obj_set_tile_id(tl, stateDisplay, 0, LV_ANIM_ON);//set display
        }
    }
    else
    {
            if(stateConfigValue == false)
            {
                if(code == LV_EVENT_CLICKED)
                {
                    if(typeBytton == 0)
                        lv_group_focus_next(settingFocuse);
                    else
                        lv_group_focus_prev(settingFocuse);
                }
            }
            else
            {
                if(code == LV_EVENT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
                {
                //get selected object
//                lv_obj_t *curentSetting = lv_group_get_focused(settingFocuse);
                //seting value of object
                    if(selectColorState)//state chage color
                    {
                        if(typeBytton == 0)
                            lv_group_focus_next(colorFocuse);
                        else
                            lv_group_focus_prev(colorFocuse);
                    }
                    else if(currentLabelTimeDisplay >= 0)//sate change time display
                    {
                        if(typeBytton == 0)
                            timeOnDisplay[currentLabelTimeDisplay]++;
                        else
                            timeOnDisplay[currentLabelTimeDisplay]--;

                        //check value
                        if(timeOnDisplay[currentLabelTimeDisplay] < MIN_TIME_ON_DISPLAY)
                            timeOnDisplay[currentLabelTimeDisplay] = MIN_TIME_ON_DISPLAY;
                        if(timeOnDisplay[currentLabelTimeDisplay] > MAX_TIME_ON_DISPLAY)
                            timeOnDisplay[currentLabelTimeDisplay] = MAX_TIME_ON_DISPLAY;

                        lv_label_set_text_fmt(labelTimeDisplay[currentLabelTimeDisplay], "%d", timeOnDisplay[currentLabelTimeDisplay]);
                    }
                }
            }
    }
}

static void btnCtrl_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    static DisplayState oldState = StateMainDisplay;

    if(stateDisplay != StateSetings)
    {
        if(code == LV_EVENT_CLICKED)
        {
            //set seting page and stop timer
            oldState = stateDisplay;
            stateDisplay = StateSetings;
            lv_obj_set_tile_id(tl, 0, 1, LV_ANIM_ON);//set display
            lv_timer_pause(timerNextDisplay);

            lv_label_set_text(labelSettings, "disp");
            lv_label_set_text(labelAction, "act");
        }
    }
    else
    {
        if(code == LV_EVENT_CLICKED)
        {
//            if(stateConfigValue)
//            {

//            }
//            else
//            {
                stateDisplay = oldState;

                //set privius state and continiu timer
                lv_timer_resume(timerNextDisplay);
                lv_timer_reset(timerNextDisplay);

                lv_timer_set_period(timerNextDisplay, timeOnDisplay[stateDisplay]*1000);//set period display
                lv_obj_set_tile_id(tl, stateDisplay, 0, LV_ANIM_ON);//set display

                lv_label_set_text(labelSettings, "conf");
                lv_label_set_text(labelAction, "add");

                //save setings
                saveSettings();
//            }
        }
    }
}

static void btnAction_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    static DisplayState oldState = StateMainDisplay;

    if(((stateDisplay == StateQRcode) || (stateDisplay == StateURL)) && (code == LV_EVENT_CLICKED))//reset page
    {
        stateDisplay = oldState;

        lv_timer_resume(timerNextDisplay);
        lv_timer_reset(timerNextDisplay);

        lv_obj_set_tile_id(tl, stateDisplay, 0, LV_ANIM_OFF);//set display
        lv_obj_clear_state(buttonSettings, LV_STATE_DISABLED);
        lv_obj_set_style_opa(buttonSettings, LV_OPA_COVER, 0);
    }
    else if((stateDisplay != StateSetings) && (code == LV_EVENT_LONG_PRESSED))//on long presed open url page
    {
        if(stateDisplay == StateQRcode)
            return;
        oldState = stateDisplay;
        stateDisplay = StateURL;

        lv_timer_pause(timerNextDisplay);

        lv_obj_set_tile_id(tl, 0, 3, LV_ANIM_OFF);//set display
        lv_obj_add_state(buttonSettings, LV_STATE_DISABLED);//disable button
        lv_obj_set_style_opa(buttonSettings, LV_OPA_50, 0);//set opa 50 to show disable
    }
    else if((stateDisplay != StateSetings) && (code == LV_EVENT_CLICKED))
    {
        if(enableShowQr)
        {
            oldState = stateDisplay;
            stateDisplay = StateQRcode;

            lv_timer_pause(timerNextDisplay);

            lv_obj_set_tile_id(tl, 0, 2, LV_ANIM_OFF);//set display
            lv_obj_add_state(buttonSettings, LV_STATE_DISABLED);//disable button
            lv_obj_set_style_opa(buttonSettings, LV_OPA_50, 0);//set opa 50 to show disable
        }
        else if(addExternalFunction != NULL)
            addExternalFunction();
    }
    else if((stateDisplay == StateSetings) && (code == LV_EVENT_CLICKED))
    {
        //get selected object
        lv_obj_t *curentSetting = lv_group_get_focused(settingFocuse);

        if(lv_obj_check_type(curentSetting, &lv_switch_class))//change switch state
        {
            if(lv_obj_has_state(curentSetting, LV_STATE_CHECKED))
                lv_obj_clear_state(curentSetting, LV_STATE_CHECKED);
            else
                lv_obj_add_state(curentSetting, LV_STATE_CHECKED);
            lv_event_send(curentSetting, LV_EVENT_VALUE_CHANGED, NULL);
        }
        else if(lv_obj_check_type(curentSetting, &lv_label_class))//change time
        {
            stateConfigValue = !stateConfigValue;
            if(stateConfigValue)//set steta change time
            {
                for(int i = 0; i < DISPLAY_SIZE; i++)
                {
                    if(labelTimeDisplay[i] == NULL)
                        continue;
                    if(curentSetting == labelTimeDisplay[i])
                    {
                        currentLabelTimeDisplay = i;
                        break;
                    }
                }
//                //change label value
//                lv_obj_t * labelButtons;
//                labelButtons = lv_obj_get_child(buttonLeft, 0);
//                lv_label_set_text(labelButtons, "-");
                lv_label_set_text(labelLeft, "-");
                lv_label_set_text(labelRight, "+");
                lv_label_set_text(labelAction, "OK");
                lv_obj_add_state(buttonSettings, LV_STATE_DISABLED);
                lv_obj_set_style_opa(buttonSettings, LV_OPA_50, 0);
            }
            else//leve state change time
            {
                currentLabelTimeDisplay = -1;
                lv_label_set_text(labelLeft, "<");
                lv_label_set_text(labelRight, ">");
                lv_label_set_text(labelAction, "act");
                lv_obj_clear_state(buttonSettings, LV_STATE_DISABLED);
                lv_obj_set_style_opa(buttonSettings, LV_OPA_COVER, 0);
            }
        }
        else if(curentSetting == btnColorSelect)
        {
            stateConfigValue = !stateConfigValue;
            selectColorState = stateConfigValue;
            lv_event_send(btnColorSelect, LV_EVENT_CLICKED, NULL);
            if(stateConfigValue)
            {
                lv_label_set_text(labelAction, "OK");
                lv_obj_add_state(buttonSettings, LV_STATE_DISABLED);
                lv_obj_set_style_opa(buttonSettings, LV_OPA_50, 0);
            }
            else
            {
                lv_label_set_text(labelAction, "act");
                lv_obj_clear_state(buttonSettings, LV_STATE_DISABLED);
                lv_obj_set_style_opa(buttonSettings, LV_OPA_COVER, 0);
            }
        }
    }
}

static void saveSettings()
{
    if(saveConfFunction == NULL)
        return;

    SettignDisplay setings;

    for(uint32_t i = 0; i < DISPLAY_SIZE; i++)
    {
        setings.timeOnDisplay[i] = timeOnDisplay[i];
    }

    setings.autoChangeDisplay = lv_obj_has_state(switchTimerEn, LV_STATE_CHECKED);

    setings.usemmHg = lv_obj_has_state(switchPressure, LV_STATE_CHECKED);

    setings.darkTheme = lv_obj_has_state(switchTheme, LV_STATE_CHECKED);

    setings.style = lv_obj_get_child_id(lv_group_get_focused(colorFocuse));


    saveConfFunction(setings);
}

static void timer_event_cb(lv_timer_t * timer)
{
    (void)timer;
//    if(time % 2)
//    {
//        meteoButtonClicked(ButtonNext, ButtonPresed);
//    }
//    else
//    {
//        meteoButtonClicked(ButtonNext, ButtonRealesed);
//    }

    lv_event_send(buttonRight, LV_EVENT_CLICKED, NULL);
}

static void switchTimer_event_cb(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target(e);

    if(lv_obj_has_state(sw, LV_STATE_CHECKED))
    {
        lv_timer_set_cb(timerNextDisplay, timer_event_cb);
//        lv_timer_enable(true);//TODO only for 1 timer
    }
    else
    {
        lv_timer_set_cb(timerNextDisplay, NULL);
//        lv_timer_enable(false);//TODO only for 1 timer
    }
}

static void switchPressure_event_cb(lv_event_t * e)
{//TODO change curent value
//    lv_obj_t * sw = lv_event_get_target(e);

//    pressurePring = lv_obj_has_state(sw, LV_STATE_CHECKED);
    setPressure(-1);

    //realign label
    if(lv_obj_has_state(switchPressure, LV_STATE_CHECKED))
    {
        lv_obj_align(labelPressure, LV_ALIGN_CENTER, 2, 10);
        lv_obj_align(labelPressure2, LV_ALIGN_CENTER, 2, 10);
    }
    else
    {
        lv_obj_align(labelPressure, LV_ALIGN_CENTER, 8, 10);
        lv_obj_align(labelPressure2, LV_ALIGN_CENTER, 8, 10);
    }

//    if(lv_obj_has_state(sw, LV_STATE_CHECKED))
//    {
////        strlen lv_label_get_text()
//    }
//    else
//    {

//    }
}

static void switchTheme_event_cb(lv_event_t * e)
{
    //realign label
#if LV_USE_THEME_DEFAULT
        lv_theme_default_init(NULL, lv_theme_get_color_primary(NULL), lv_theme_get_color_secondary(NULL),
                              lv_obj_has_state(switchTheme, LV_STATE_CHECKED), fontMain);

        if(lv_obj_has_state(switchTheme, LV_STATE_CHECKED))
            lv_style_set_border_color(&styleButtons, lv_color_make(0x10, 0x14, 0x18));
        else
            lv_style_set_border_color(&styleButtons, lv_color_make(0xF6, 0xF6, 0xF6));
#endif
}

static void mainPage_create(lv_obj_t * parent)
{
    //remove and disable scroll
//    lv_obj_remove_style(parent, NULL, LV_PART_SCROLLBAR);
//    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
//    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    lv_style_init(&styleDisplayMain);
    lv_style_set_text_color(&styleDisplayMain, lv_color_make(0x00, 0x00, 0x00));
    lv_style_set_text_font(&styleDisplayMain, fontData);
    lv_style_set_radius(&styleDisplayMain, 0);
    lv_style_set_border_width(&styleDisplayMain, 1);
    lv_style_set_border_color(&styleDisplayMain, lv_color_make(0x00, 0x00, 0x00));

    co2Obj = lv_obj_create(parent);
    humidityObj = lv_obj_create(parent);
    temperatureObj = lv_obj_create(parent);
    pressureObj = lv_obj_create(parent);

    lv_obj_add_style(co2Obj, &styleDisplayMain, 0);
    lv_obj_add_style(humidityObj, &styleDisplayMain, 0);
    lv_obj_add_style(temperatureObj, &styleDisplayMain, 0);
    lv_obj_add_style(pressureObj, &styleDisplayMain, 0);

    labelCo2 = lv_label_create(co2Obj);
    labelHumidity = lv_label_create(humidityObj);
    labelTemperatre = lv_label_create(temperatureObj);
    labelPressure = lv_label_create(pressureObj);

    lv_obj_set_style_text_font(labelCo2, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_font(labelHumidity, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_font(labelTemperatre, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_font(labelPressure, &lv_font_montserrat_18, 0);

//    lv_palette_lighten()
    lv_obj_set_style_bg_color(co2Obj, lv_palette_main(LV_PALETTE_LIGHT_GREEN), 0);
    lv_obj_set_style_bg_color(humidityObj, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_bg_color(temperatureObj, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_obj_set_style_bg_color(pressureObj, lv_palette_lighten(LV_PALETTE_TEAL, 2), 0);

    lv_obj_set_size(co2Obj, BIG_RECT_WIDTH + 1, BIG_RECT_HEIGHT + 1);
    lv_obj_set_size(humidityObj, SMALL_RECT_WIDTH, SMALL_RECT_HEIGHT + 1);
    lv_obj_set_size(temperatureObj, SMALL_RECT_WIDTH, SMALL_RECT_HEIGHT + 1);
    lv_obj_set_size(pressureObj, SMALL_RECT_WIDTH, SMALL_RECT_HEIGHT + 1);

    lv_obj_set_pos(co2Obj, 0, SMALL_RECT_HEIGHT);
    lv_obj_set_pos(humidityObj, BIG_RECT_WIDTH, 0);
    lv_obj_set_pos(temperatureObj, BIG_RECT_WIDTH, SMALL_RECT_HEIGHT);
    lv_obj_set_pos(pressureObj, BIG_RECT_WIDTH, SMALL_RECT_HEIGHT*2);

    labelTime = lv_label_create(parent);
    lv_obj_set_style_text_font(labelTime, &lv_font_montserrat_40, 0);
    lv_obj_set_pos(labelTime, 2, 15);
    lv_label_set_text(labelTime, "20:47");

    labelDate = lv_label_create(parent);
    lv_obj_set_style_text_font(labelDate, &lv_font_montserrat_18, 0);
    lv_obj_set_pos(labelDate, 115, 40);
    lv_label_set_text(labelDate, "23.03.2022");

    lv_label_set_text(labelCo2, "5000ppm");
    lv_label_set_text(labelHumidity, "100%");
    lv_label_set_text(labelTemperatre, " 45C");
    lv_label_set_text(labelPressure, "1045hPa");

    //wifi data
    lv_obj_t * wifiImage = lv_img_create(parent);
    lv_img_set_src(wifiImage, LV_SYMBOL_WIFI);
    lv_obj_set_pos(wifiImage, 180, 15);

    ledWifi  = lv_led_create(parent);
    lv_obj_set_size(ledWifi, 20, 20);
    lv_led_set_color(ledWifi, lv_palette_main(LV_PALETTE_LIGHT_GREEN));
    lv_obj_align_to(ledWifi, wifiImage, LV_ALIGN_OUT_LEFT_MID, -6, 0);
    lv_led_off(ledWifi);

    lv_obj_remove_style(humidityObj, NULL, LV_PART_SCROLLBAR);
    lv_obj_remove_style(temperatureObj, NULL, LV_PART_SCROLLBAR);
    lv_obj_remove_style(pressureObj, NULL, LV_PART_SCROLLBAR);
    lv_obj_clear_flag(pressureObj, LV_OBJ_FLAG_SCROLLABLE);
//    lv_obj_set_scrollbar_mode(pressureObj, LV_SCROLLBAR_MODE_OFF);

    lv_obj_center(labelCo2);
    lv_obj_align(labelHumidity, LV_ALIGN_CENTER, 10, 10);
    lv_obj_align(labelTemperatre, LV_ALIGN_CENTER, 10, 10);
    lv_obj_align(labelPressure, LV_ALIGN_CENTER, 8, 10);

    //help label
    lv_obj_t * label;
    label = lv_label_create(co2Obj);
    lv_label_set_text(label, "CO2");
    lv_obj_set_pos(label, -10, -10);

    label = lv_label_create(humidityObj);
    lv_label_set_text(label, "H");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(label, -10, -10);

    label = lv_label_create(temperatureObj);
    lv_label_set_text(label, "t");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(label, -10, -10);

    label = lv_label_create(pressureObj);
    lv_label_set_text(label, "P");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(label, -10, -10);
}

static void mainPage2_create(lv_obj_t * parent)
{
    lv_style_init(&styleDisplayMain2);
    lv_style_set_text_color(&styleDisplayMain2, lv_color_make(0x00, 0x00, 0x00));
    lv_style_set_text_font(&styleDisplayMain2, fontData);
    lv_style_set_radius(&styleDisplayMain2, 0);
    lv_style_set_border_width(&styleDisplayMain2, 0);
//    lv_style_set_border_color(&styleDisplayMain2, lv_color_make(0xFF, 0xFF, 0xFF));

    co2Obj2 = lv_obj_create(parent);
    humidityObj2 = lv_obj_create(parent);
    temperatureObj2 = lv_obj_create(parent);
    pressureObj2 = lv_obj_create(parent);
    timeDateObj2 = lv_obj_create(parent);
    lv_obj_clear_flag(timeDateObj2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_style(co2Obj2, &styleDisplayMain2, 0);
    lv_obj_add_style(humidityObj2, &styleDisplayMain2, 0);
    lv_obj_add_style(temperatureObj2, &styleDisplayMain2, 0);
    lv_obj_add_style(pressureObj2, &styleDisplayMain2, 0);
//    lv_obj_add_style(timeDateObj2, &styleDisplayMain2, 0);
    lv_obj_set_style_radius(timeDateObj2, 0, 0);
    lv_obj_set_style_border_width(timeDateObj2, 0, 0);

    labelCo22 = lv_label_create(co2Obj2);
    labelHumidity2 = lv_label_create(humidityObj2);
    labelTemperatre2 = lv_label_create(temperatureObj2);
    labelPressure2 = lv_label_create(pressureObj2);

//    lv_palette_lighten()
    lv_obj_set_style_bg_color(co2Obj2, lv_color_make(0xD3, 0xEA, 0xDD), 0);
    lv_obj_set_style_bg_color(humidityObj2, lv_color_make(0xD3, 0xEA, 0xDD), 0);
    lv_obj_set_style_bg_color(temperatureObj2, lv_color_make(0xD3, 0xEA, 0xDD), 0);
    lv_obj_set_style_bg_color(pressureObj2, lv_color_make(0xEB, 0xDC, 0xF9), 0);
//    lv_obj_set_style_bg_color(timeDateObj2, lv_color_make(0xD3, 0xEA, 0xDD), 0);

//    lv_obj_set_style_border_side(co2Obj2, LV_BORDER_SIDE_FULL - LV_BORDER_SIDE_LEFT, 0);
//    lv_obj_set_style_border_side(timeDateObj2, LV_BORDER_SIDE_FULL - LV_BORDER_SIDE_LEFT, 0);
//    lv_obj_set_style_border_side(humidityObj2, LV_BORDER_SIDE_FULL - LV_BORDER_SIDE_RIGHT, 0);
//    lv_obj_set_style_border_side(temperatureObj2, LV_BORDER_SIDE_FULL - LV_BORDER_SIDE_RIGHT, 0);
//    lv_obj_set_style_border_side(pressureObj2, LV_BORDER_SIDE_FULL - LV_BORDER_SIDE_RIGHT, 0);

    lv_obj_set_size(co2Obj2, BIG_RECT_WIDTH-3, BIG_RECT_HEIGHT-6);
    lv_obj_set_size(humidityObj2, SMALL_RECT_WIDTH-3, SMALL_RECT_HEIGHT-3);
    lv_obj_set_size(temperatureObj2, SMALL_RECT_WIDTH-3, SMALL_RECT_HEIGHT-6);
    lv_obj_set_size(pressureObj2, SMALL_RECT_WIDTH-3, SMALL_RECT_HEIGHT-6);
    lv_obj_set_size(timeDateObj2, BIG_RECT_WIDTH-3, SMALL_RECT_HEIGHT-3);

    lv_obj_set_pos(co2Obj2, 0, SMALL_RECT_HEIGHT+3);
    lv_obj_set_pos(humidityObj2, BIG_RECT_WIDTH + 3, 0);
    lv_obj_set_pos(temperatureObj2, BIG_RECT_WIDTH + 3, SMALL_RECT_HEIGHT+3);
    lv_obj_set_pos(pressureObj2, BIG_RECT_WIDTH + 3, SMALL_RECT_HEIGHT*2+3);
    lv_obj_set_pos(timeDateObj2, 0, 0);

    labelTime2 = lv_label_create(timeDateObj2);
    lv_obj_set_style_text_font(labelTime2, &lv_font_montserrat_40, 0);
    lv_obj_align(labelTime2, LV_ALIGN_LEFT_MID, -8, 0);
    lv_label_set_text(labelTime2, "20:47");

    labelDate2 = lv_label_create(timeDateObj2);
    lv_obj_set_style_text_font(labelDate2, &lv_font_montserrat_18, 0);
//    lv_obj_set_pos(labelDate2, 65, 20);
    lv_obj_align_to(labelDate2, labelTime2, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 0);
    lv_label_set_text(labelDate2, "23.03.2022");

    //wifi data
    ledWifi2  = lv_led_create(timeDateObj2);
    lv_obj_set_size(ledWifi2, 21, 21);
    lv_led_set_color(ledWifi2, lv_palette_main(LV_PALETTE_LIGHT_GREEN));
    lv_obj_align_to(ledWifi2, labelDate2, LV_ALIGN_OUT_TOP_MID, 0, -4);
//    lv_led_off(ledWifi2);
    lv_led_set_color(ledWifi2, lv_palette_main(LV_PALETTE_RED));

    lv_obj_t * wifiImage = lv_img_create(ledWifi2);
    lv_img_set_src(wifiImage, LV_SYMBOL_WIFI);
    lv_obj_align(wifiImage, LV_ALIGN_CENTER, 1, 1);

    lv_label_set_text(labelCo22, "5000");
    lv_label_set_text(labelHumidity2, "100 %");
    lv_label_set_text(labelTemperatre2, " 45 C");
    lv_label_set_text(labelPressure2, "1045 hPa");

    lv_obj_set_style_text_font(labelCo22, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_font(labelHumidity2, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_font(labelTemperatre2, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_font(labelPressure2, &lv_font_montserrat_18, 0);

//    lv_obj_remove_style(pressureObj2, NULL, LV_PART_SCROLLBAR);
    lv_obj_clear_flag(humidityObj2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(temperatureObj2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(pressureObj2, LV_OBJ_FLAG_SCROLLABLE);

    //help label
    lv_obj_t * label;

    label = lv_label_create(co2Obj2);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_label_set_text(label, "CO2");
    lv_obj_set_pos(label, -10, -10);

    labelCo2ppm2 = lv_label_create(co2Obj2);
    lv_obj_set_style_text_font(labelCo2ppm2, &lv_font_montserrat_12, 0);
    lv_label_set_text(labelCo2ppm2, "ppm");

    label = lv_label_create(humidityObj2);
    lv_label_set_text(label, "H");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(label, -10, -10);

    label = lv_label_create(temperatureObj2);
    lv_label_set_text(label, "t");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(label, -10, -10);

    label = lv_label_create(pressureObj2);
    lv_label_set_text(label, "p");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(label, -10, -10);

    lv_obj_center(labelCo22);
    lv_obj_align(labelHumidity2, LV_ALIGN_CENTER, 10, 6);
    lv_obj_align(labelTemperatre2, LV_ALIGN_CENTER, 10, 6);
    lv_obj_align(labelPressure2, LV_ALIGN_CENTER, 8, 10);

//    lv_obj_align_to(labelCo2co22, labelCo22, LV_ALIGN_OUT_LEFT_BOTTOM, -5, -6);
    lv_obj_align_to(labelCo2ppm2, labelCo22, LV_ALIGN_OUT_RIGHT_BOTTOM, +3, -6);
}

static void mainPage3_create(lv_obj_t * parent)
{
    lv_meter_scale_t * scale;
    meterCo23 = lv_meter_create(parent);

    //create circle indicators
    lv_obj_remove_style(meterCo23, NULL, LV_PART_INDICATOR);
    scale = lv_meter_add_scale(meterCo23);
    lv_meter_set_scale_range(meterCo23, scale, 0, MAX_CO2, 360, 90);
    lv_meter_set_scale_ticks(meterCo23, scale, 2, 0, 1, lv_palette_main(LV_PALETTE_GREY));
    lv_obj_set_style_border_width(meterCo23, 0, 0);
    //add line(arc) indicator
    indicCo23 = lv_meter_add_arc(meterCo23, scale, 15, lv_palette_main(LV_PALETTE_RED), 10);

    meterHumidity3 = lv_meter_create(parent);
    lv_obj_remove_style(meterHumidity3, NULL, LV_PART_INDICATOR);
    scale = lv_meter_add_scale(meterHumidity3);
    lv_meter_set_scale_range(meterHumidity3, scale, 0, 100, 360, 90);
    lv_meter_set_scale_ticks(meterHumidity3, scale, 2, 0, 1, lv_palette_main(LV_PALETTE_GREY));
    lv_obj_set_style_border_width(meterHumidity3, 0, 0);
    indicHumidity3 = lv_meter_add_arc(meterHumidity3, scale, 8, lv_palette_main(LV_PALETTE_LIGHT_BLUE), 10);

    meterTemperature3 = lv_meter_create(parent);
    lv_obj_remove_style(meterTemperature3, NULL, LV_PART_INDICATOR);
    scale = lv_meter_add_scale(meterTemperature3);
    lv_meter_set_scale_range(meterTemperature3, scale, MIN_TEMP, MAX_TEMP, 360, 90);
    lv_meter_set_scale_ticks(meterTemperature3, scale, 2, 0, 1, lv_palette_main(LV_PALETTE_GREY));
    lv_obj_set_style_border_width(meterTemperature3, 0, 0);
    indicTemperature3 = lv_meter_add_arc(meterTemperature3, scale, 8, lv_palette_main(LV_PALETTE_YELLOW), 10);


    //create labels
    lv_obj_set_size(meterCo23, 200, 200);
    lv_obj_set_pos(meterCo23, 10, 5);

    lv_obj_set_size(meterHumidity3, 100, 100);
    lv_obj_set_pos(meterHumidity3, 215, 5);

    lv_obj_set_size(meterTemperature3, 100, 100);
    lv_obj_set_pos(meterTemperature3, 215, 105);

    //help label
    lv_obj_t * label;

    //co2 data
    labelCo23 = lv_label_create(meterCo23);
    lv_obj_set_style_text_font(labelCo23, &lv_font_montserrat_48, 0);
    lv_label_set_text(labelCo23, "5000");
//    lv_obj_center(labelCo23);
    lv_obj_align(labelCo23, LV_ALIGN_CENTER, 0, 0);

    label = lv_label_create(meterCo23);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_label_set_text(label, "CO2");
    lv_obj_align_to(label, labelCo23, LV_ALIGN_OUT_TOP_MID, 0, 0);

    label = lv_label_create(meterCo23);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_label_set_text(label, "ppm");
    lv_obj_align_to(label, labelCo23, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

//    //pressure data
//    labelPressure3 = lv_label_create(meterCo23);
//    lv_obj_set_style_text_font(labelPressure3, &lv_font_montserrat_22, 0);
//    lv_label_set_text(labelPressure3, "1045 hPa");
//    lv_obj_align(labelPressure3, LV_ALIGN_CENTER, 0, 40);//align with value up

    //humidity data
    labelHumidity3 = lv_label_create(meterHumidity3);
    lv_obj_set_style_text_font(labelHumidity3, &lv_font_montserrat_20, 0);
    lv_label_set_text(labelHumidity3, "100");
    lv_obj_center(labelHumidity3);

    label = lv_label_create(meterHumidity3);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_label_set_text(label, "H");
    lv_obj_align_to(label, labelHumidity3, LV_ALIGN_OUT_TOP_MID, 0, -2);

    label = lv_label_create(meterHumidity3);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_label_set_text(label, "%");
    lv_obj_align_to(label, labelHumidity3, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    //temperature data
    labelTemperatre3 = lv_label_create(meterTemperature3);
    lv_obj_set_style_text_font(labelTemperatre3, &lv_font_montserrat_20, 0);
    lv_label_set_text(labelTemperatre3, "37");
    lv_obj_center(labelTemperatre3);

    label = lv_label_create(meterTemperature3);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_label_set_text(label, "t");
    lv_obj_align_to(label, labelTemperatre3, LV_ALIGN_OUT_TOP_MID, 0, -2);

    label = lv_label_create(meterTemperature3);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_label_set_text(label, "C");
    lv_obj_align_to(label, labelTemperatre3, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    lv_meter_set_indicator_start_value(meterTemperature3, indicTemperature3, MIN_TEMP);



    indicCo23->type_data.arc.color = lv_palette_main(LV_PALETTE_GREEN);
    lv_meter_set_indicator_end_value(meterCo23, indicCo23, 2600);

    lv_meter_set_indicator_end_value(meterHumidity3, indicHumidity3, 95);
    lv_meter_set_indicator_end_value(meterTemperature3, indicTemperature3, 37);

}

static void mainPage4_create(lv_obj_t * parent)
{
    lv_meter_scale_t * scalecO2, *scalecHumidity, *scalecTemperatyre;
    meterData4 = lv_meter_create(parent);

    //create circle indicators
    lv_obj_remove_style(meterData4, NULL, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(meterData4, 0, 0);
    scalecO2 = lv_meter_add_scale(meterData4);
    scalecHumidity = lv_meter_add_scale(meterData4);
    scalecTemperatyre = lv_meter_add_scale(meterData4);


    lv_meter_set_scale_range(meterData4, scalecO2, 0, MAX_CO2, 360, 90);
    lv_meter_set_scale_ticks(meterData4, scalecO2, 2, 0, 1, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_range(meterData4, scalecHumidity, 0, 100, 180, 90);
    lv_meter_set_scale_ticks(meterData4, scalecHumidity, 2, 0, 1, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_range(meterData4, scalecTemperatyre, MAX_TEMP, MIN_TEMP, 180, 270);
    lv_meter_set_scale_ticks(meterData4, scalecTemperatyre, 2, 0, 1, lv_palette_main(LV_PALETTE_GREY));
    //add line(arc) indicator
    indicCo24 = lv_meter_add_arc(meterData4, scalecO2, 15, lv_palette_main(LV_PALETTE_RED), -1);
    indicHumidity4 = lv_meter_add_arc(meterData4, scalecHumidity, 10, lv_palette_main(LV_PALETTE_LIGHT_BLUE), 9);
    indicTemperature4 = lv_meter_add_arc(meterData4, scalecTemperatyre, 10, lv_palette_main(LV_PALETTE_YELLOW), 9);

    //create labels
    lv_obj_set_size(meterData4, 200, 200);
//    lv_obj_center(meterData4);
    lv_obj_align(meterData4, LV_ALIGN_CENTER, -20, 0);


    //help label
    lv_obj_t * label;

    //co2 data
    labelCo24 = lv_label_create(meterData4);
    lv_obj_set_style_text_font(labelCo24, &lv_font_montserrat_48, 0);
    lv_label_set_text(labelCo24, "5000");
    lv_obj_align(labelCo24, LV_ALIGN_CENTER, 0, 0);

    label = lv_label_create(meterData4);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_label_set_text(label, "CO2");
    lv_obj_align_to(label, labelCo24, LV_ALIGN_OUT_TOP_MID, 0, 0);

    label = lv_label_create(meterData4);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_label_set_text(label, "ppm");
    lv_obj_align_to(label, labelCo24, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    //humidity data
    labelHumidity4 = lv_label_create(parent);
    lv_obj_set_style_text_font(labelHumidity4, &lv_font_montserrat_20, 0);
    lv_label_set_text(labelHumidity4, "100");
    lv_obj_align_to(labelHumidity4, meterData4, LV_ALIGN_OUT_LEFT_MID, 0, 0);

    label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_label_set_text(label, "H");
    lv_obj_align_to(label, labelHumidity4, LV_ALIGN_OUT_TOP_MID, 0, -2);

    label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_label_set_text(label, "%");
    lv_obj_align_to(label, labelHumidity4, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    //temperature data
    labelTemperatre4 = lv_label_create(parent);
    lv_obj_set_style_text_font(labelTemperatre4, &lv_font_montserrat_20, 0);
    lv_label_set_text(labelTemperatre4, "37");
    lv_obj_align_to(labelTemperatre4, meterData4, LV_ALIGN_OUT_RIGHT_MID, 3, 0);

    label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_label_set_text(label, "t");
    lv_obj_align_to(label, labelTemperatre4, LV_ALIGN_OUT_TOP_MID, 0, -2);

    label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_label_set_text(label, "C");
    lv_obj_align_to(label, labelTemperatre4, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    //pressure data
    labelPressure4 = lv_label_create(parent);
    lv_obj_set_style_text_font(labelPressure4, &lv_font_montserrat_16, 0);
    lv_label_set_text(labelPressure4, "P 1045 hPa");
//    lv_label_set_text(labelPressure4, "P 748 mmHg");
    lv_obj_set_pos(labelPressure4, 205, 184);

    //time
    labelTime4 = lv_label_create(parent);
    lv_obj_set_style_text_font(labelTime4, &lv_font_montserrat_36, 0);
    lv_label_set_text(labelTime4, "20:47");
    lv_obj_set_pos(labelTime4, 220, 2);

    labelDate4 = lv_label_create(parent);
    lv_obj_set_style_text_font(labelDate4, &lv_font_montserrat_16, 0);
    lv_label_set_text(labelDate4, "23.03.2022");
//    lv_obj_set_pos(labelTime4, 220, 2);
    lv_obj_align_to(labelDate4, labelTime4, LV_ALIGN_OUT_BOTTOM_MID, 4, -4);

    lv_meter_set_indicator_end_value(meterData4, indicTemperature4, MIN_TEMP);



    indicCo24->type_data.arc.color = lv_palette_main(LV_PALETTE_GREEN);
    lv_meter_set_indicator_end_value(meterData4, indicCo24, 2600);

    lv_meter_set_indicator_end_value(meterData4, indicHumidity4, 95);

//    lv_meter_set_indicator_start_value(meterData4, indicTemperature4, (MAX_TEMP - MIN_TEMP)/2 - (-25));
//    lv_meter_set_indicator_end_value(meterData4, indicTemperature4, MAX_TEMP - MIN_TEMP);
    lv_meter_set_indicator_start_value(meterData4, indicTemperature4, 25);

}

static void draw_part_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
    /*If the cells are drawn...*/
    if(dsc->part == LV_PART_ITEMS) {
        uint32_t row = dsc->id /  lv_table_get_col_cnt(obj);
        uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);

        /*Make the texts in the first cell center aligned*/
        if(row == 0) {
            dsc->label_dsc->align = LV_TEXT_ALIGN_CENTER;
            dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_BLUE), dsc->rect_dsc->bg_color, LV_OPA_20);
            dsc->rect_dsc->bg_opa = LV_OPA_COVER;
        }
        /*In the first column align the texts to the right*/
        else if(col == 0) {
            dsc->label_dsc->align = LV_TEXT_ALIGN_RIGHT;
        }

        if((col != 0) && (row != 0))
        {
            dsc->label_dsc->font = &lv_font_montserrat_20;
        }

        /*MAke every 2nd row grayish*/
        if((row != 0 && row % 2) == 0) {
            dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), dsc->rect_dsc->bg_color, LV_OPA_10);
            dsc->rect_dsc->bg_opa = LV_OPA_COVER;
        }
    }
}

static void extDataPage_create(lv_obj_t * parent, lv_obj_t **_tabel)
{
    lv_obj_t *tabel = lv_table_create(parent);
    *_tabel = tabel;
    lv_table_set_cell_value(tabel, 0, 0, "Place");
    lv_table_set_cell_value(tabel, 0, 1, "Tem[C]");
    lv_table_set_cell_value(tabel, 0, 2, "Hum[%]");
    lv_table_set_cell_value(tabel, 0, 3, "Bat[%]");

    lv_obj_set_style_text_font(tabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_border_width(tabel, 0, 0);
    lv_obj_set_style_outline_width(tabel, 0, 0);

    lv_table_set_col_width(tabel, 0, 100);
    lv_table_set_col_width(tabel, 1, 75);
    lv_table_set_col_width(tabel, 2, 75);
    lv_table_set_col_width(tabel, 3, 70);

    lv_table_set_cell_value(tabel, 1, 0, "Outside");
    lv_table_set_cell_value(tabel, 2, 0, "Badroom");
    lv_table_set_cell_value(tabel, 3, 0, "Kitchen");
    lv_table_set_cell_value(tabel, 4, 0, "Wall");
//    lv_table_set_cell_value(tabel, 5, 0, "Badroom2");

    for(int i = 0; i < SIZE_EXT_DATA; i++)
    {
        lv_table_set_cell_value(tabel, i+1, 1, "0");
        lv_table_set_cell_value(tabel, i+1, 2, "0");
        lv_table_set_cell_value(tabel, i+1, 3, "0");
    }

    lv_obj_set_pos(tabel, 0, 0);
//    lv_obj_set_height(tabel, 200);
//    lv_obj_set_width(tabel, 200);
    lv_obj_set_size(tabel, LV_HOR_RES, LV_VER_RES - BUTTON_HEIGHT);

    /*Add an event callback to to apply some custom drawing*/
    lv_obj_add_event_cb(tabel, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
}

static void weatherPage_create(lv_obj_t * parent)
{
    //NOTE use white image
    //set a litle dark BG for contrast wheather image on white theme
    lv_obj_set_style_bg_color(parent, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_10, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);


    wMainImage = lv_img_create(parent);
    lv_obj_set_pos(wMainImage, 240, 10);
    lv_img_set_src(wMainImage, lv_weather[10].img_dsc);

    wMainTemperature = lv_label_create(parent);
    lv_label_set_text_fmt(wMainTemperature, "%+d", 25);
    lv_obj_set_style_text_font(wMainTemperature, &lv_font_montserrat_42, 0);
    lv_obj_align_to(wMainTemperature, wMainImage, LV_ALIGN_OUT_LEFT_MID, -20, -18);

    wFellTemperature = lv_label_create(parent);
    lv_label_set_text_fmt(wFellTemperature, "Feel %+d", 22);
    lv_obj_set_style_text_font(wFellTemperature, &lv_font_montserrat_16, 0);
    lv_obj_align_to(wFellTemperature, wMainTemperature, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    wHummidity = lv_label_create(parent);
    lv_label_set_text_fmt(wHummidity, "H:%3d%%", 80);
    lv_obj_set_style_text_font(wHummidity, &lv_font_montserrat_14, 0);
    lv_obj_align_to(wHummidity, wFellTemperature, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);

    wTime = lv_label_create(parent);
    lv_label_set_text(wTime, "20:47");
    lv_obj_set_style_text_font(wTime, &lv_font_montserrat_40, 0);
    lv_obj_align(wTime, LV_ALIGN_TOP_LEFT, 5, 5);

    wLastUpdate = lv_label_create(parent);
    lv_label_set_text(wLastUpdate, "update:19:30");
    lv_obj_set_style_text_font(wLastUpdate, &lv_font_montserrat_16, 0);
    lv_obj_align_to(wLastUpdate, wTime, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);


    static lv_point_t line_points[] = { {5, 0}, {310, 0}};

    lv_obj_t * line1;
    line1 = lv_line_create(parent);
    lv_line_set_points(line1, line_points, 2);     /*Set the points*/
    lv_obj_set_pos(line1, 0, 85);

    //create forcast images
//    lv_obj_t *labelTimeForcast;
    for(int i = 0; i < SIZE_FORCAST; i++)
    {
        wForcastHTemp[i] = lv_label_create(parent);
        wForcastHImage[i] = lv_img_create(parent);
        wForcastDTemp[i] = lv_label_create(parent);
        wForcastDImage[i] = lv_img_create(parent);

        //H forcast image
//        lv_obj_set_pos(wForcastHImage[i], i*80 + 10, 95);
        lv_img_set_src(wForcastHImage[i], lv_weather[10].img_dsc);
        lv_img_set_zoom(wForcastHImage[i], 128);
        lv_obj_set_pos(wForcastHImage[i], i*80 + 10, 100);

        //H forcast val
        lv_label_set_text_fmt(wForcastHTemp[i], "%+d(+%dh)", +50, STEP_FORCAST_HOUR*(i + 1));
        lv_obj_set_style_text_font(wForcastHTemp[i], &lv_font_montserrat_14, 0);
        lv_obj_align_to(wForcastHTemp[i], wForcastHImage[i], LV_ALIGN_OUT_TOP_MID, 0, 7);


        //D forcast image
        lv_img_set_src(wForcastDImage[i], lv_weather[10].img_dsc);
        lv_img_set_zoom(wForcastDImage[i], 128);
        lv_obj_set_pos(wForcastDImage[i], i*80 + 10, 160);

        //D forcast val
        lv_label_set_text_fmt(wForcastDTemp[i], "%+d(+%dd)", +50, (i + 1));
        lv_obj_set_style_text_font(wForcastDTemp[i], &lv_font_montserrat_14, 0);
        lv_obj_align_to(wForcastDTemp[i], wForcastDImage[i], LV_ALIGN_OUT_TOP_MID, 0, 7);

    }

//    lv_img_set_src(wForcastHImage[0], lv_weather[35].img_dsc);
//    lv_img_set_src(wForcastHImage[1], lv_weather[23].img_dsc);
//    lv_img_set_src(wForcastHImage[2], lv_weather[31].img_dsc);
//    lv_img_set_src(wForcastHImage[3], lv_weather[39].img_dsc);

//    lv_img_set_src(wForcastDImage[0], lv_weather[24].img_dsc);
////    lv_img_set_src(wForcastDImage[1], lv_weather[11].img_dsc);
////    lv_img_set_src(wForcastDImage[2], lv_weather[26].img_dsc);
//    lv_img_set_src(wForcastDImage[1], lv_weather[35].img_dsc);
//    lv_img_set_src(wForcastDImage[2], lv_weather[10].img_dsc);
//    lv_img_set_src(wForcastDImage[3], lv_weather[5].img_dsc);


    lv_img_set_src(wForcastHImage[0], lv_weather[32].img_dsc);
    lv_img_set_src(wForcastHImage[1], lv_weather[33].img_dsc);
    lv_img_set_src(wForcastHImage[2], lv_weather[34].img_dsc);
    lv_img_set_src(wForcastHImage[3], lv_weather[35].img_dsc);

    lv_img_set_src(wForcastDImage[0], lv_weather[36].img_dsc);
    lv_img_set_src(wForcastDImage[1], lv_weather[37].img_dsc);
    lv_img_set_src(wForcastDImage[2], lv_weather[38].img_dsc);
    lv_img_set_src(wForcastDImage[3], lv_weather[39].img_dsc);



//    LV_IMG_DECLARE(b_0_2x);
//    LV_IMG_DECLARE(b_5_2x);
//    LV_IMG_DECLARE(b_11_2x);
//    LV_IMG_DECLARE(b_99_2x);

//    LV_IMG_DECLARE(w_0_2x);
//    LV_IMG_DECLARE(w_5_2x);
//    LV_IMG_DECLARE(w_11_2x);
//    LV_IMG_DECLARE(w_99_2x);


//    wMainImage = lv_img_create(parent);
//    lv_img_set_src(wMainImage, &b_0_2x);
//    lv_obj_set_pos(wMainImage, 5, 5);

//    wMainImage = lv_img_create(parent);
//    lv_img_set_src(wMainImage, &w_0_2x);
//    lv_obj_set_pos(wMainImage, 5, 110);

//    wMainImage = lv_img_create(parent);
//    lv_img_set_src(wMainImage, &b_5_2x);
//    lv_obj_set_pos(wMainImage, 110, 5);

//    wMainImage = lv_img_create(parent);
//    lv_img_set_src(wMainImage, &w_5_2x);
//    lv_obj_set_pos(wMainImage, 110, 110);

//    wMainImage = lv_img_create(parent);
//    lv_img_set_src(wMainImage, &b_11_2x);
//    lv_obj_set_pos(wMainImage, 240, 5);

//    wMainImage = lv_img_create(parent);
//    lv_img_set_src(wMainImage, &w_11_2x);
//    lv_obj_set_pos(wMainImage, 240, 110);

//    wMainImage = lv_img_create(parent);
//    lv_img_set_src(wMainImage, &b_99_2x);
//    lv_obj_set_pos(wMainImage, 370, 5);

//    wMainImage = lv_img_create(parent);
//    lv_img_set_src(wMainImage, &w_99_2x);
//    lv_obj_set_pos(wMainImage, 370, 110);


}

static void qrCodePage_create(lv_obj_t * parent)
{
    qrCodeThisAP = lv_qrcode_create(parent, 100, lv_color_hex(0), lv_color_hex(0xFFFFFFFF));
    qrCodeConnectToAp = lv_qrcode_create(parent, 100, lv_color_hex(0), lv_color_hex(0xFFFFFFFF));

    lv_obj_set_pos(qrCodeThisAP, 35, 65);
    lv_obj_set_pos(qrCodeConnectToAp, 185, 65);

    lv_qrcode_update(qrCodeThisAP, "WIFI:T:nopass;S:SSID;P:;;", 25);
    lv_qrcode_update(qrCodeConnectToAp, "DPP:C:81/6;M:ff:ff:ff:ff:ff:ff;I:dev;K:0=;;", 43);

    lv_obj_t *label;
    label = lv_label_create(parent);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_recolor(label, true);
    lv_label_set_text(label, "Connect to device AP using #0000ff QR-code A# or connect to or"
                             "connect thit device to your net using #ff0000 QR-code B#");
    lv_obj_set_width(label, 300);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    label = lv_label_create(parent);
    lv_label_set_recolor(label, true);
    lv_label_set_text(label, "#0000ff QR-code A# \n(This AP)");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(label, qrCodeThisAP, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    label = lv_label_create(parent);
    lv_label_set_recolor(label, true);
    lv_label_set_text(label, "#ff0000 QR-code B# \n(Connect dev to net)");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(label, qrCodeConnectToAp, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
}

static void urlPage_create(lv_obj_t * parent)
{
    labelDeviceIP = lv_label_create(parent);
    lv_label_set_text(labelDeviceIP, "IP:192.168.10.1");
    lv_obj_align(labelDeviceIP, LV_ALIGN_TOP_MID, 0, 10);

    qrCodeURL = lv_qrcode_create(parent, 100, lv_color_hex(0), lv_color_hex(0xFFFFFFFF));
    lv_qrcode_update(qrCodeURL, "http://192.168.10.1", 19);

    lv_obj_align(qrCodeURL, LV_ALIGN_CENTER, 0, 10);
}

static void focusChange_event_cb(lv_event_t * e)
{//change selected settings
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
//    lv_obj_set_style_outline_color(switchTimerEn, lv_theme_get_color_primary(obj), 0);
    switch (code)
    {
    case LV_EVENT_FOCUSED:
        lv_obj_set_style_outline_width(obj, 2, 0);
        break;
    case LV_EVENT_DEFOCUSED:
        lv_obj_set_style_outline_width(obj, 0, 0);
        break;
    default:
        break;
    }
}

#define CREATE_LABEL_TIME(x,y)          do{ label = lv_label_create(parent);    \
                                            lv_obj_set_pos(label, x, y);      \
                                            /*lv_obj_set_width(label, 60);*/      \
                                            lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0); \
                                            lv_obj_set_style_text_font(label, &lv_font_montserrat_30, 0);   \
                                            lv_label_set_text_fmt(label, "%d", MAX_TIME_ON_DISPLAY);}while(0);

static void settingPage_create(lv_obj_t * parent)
{
    lv_obj_t * listSetings[SITING_LIST_SIZE] = {NULL/*switchTimerEn*/};
    //create groub settings
    settingFocuse = lv_group_create();

    //crate checkbox eneble timer switch
    switchTimerEn = lv_switch_create(parent);
    lv_obj_set_pos(switchTimerEn, 20, 10);
    lv_obj_add_state(switchTimerEn, LV_STATE_CHECKED);
    lv_obj_add_event_cb(switchTimerEn, switchTimer_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    listSetings[0] = switchTimerEn;

    switchPressure = lv_switch_create(parent);
    lv_obj_set_pos(switchPressure, 120, 10);
    lv_obj_add_event_cb(switchPressure, switchPressure_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    listSetings[1] = switchPressure;

    //create label values
    lv_obj_t * label;
    CREATE_LABEL_TIME(80, 70);
    listSetings[3] = labelTimeDisplay[0] = label;
    CREATE_LABEL_TIME(170, 70);
    listSetings[4] = labelTimeDisplay[1] = label;
    CREATE_LABEL_TIME(245, 70);
    listSetings[5] = labelTimeDisplay[2] = label;
    CREATE_LABEL_TIME(40, 140);
    listSetings[6] = labelTimeDisplay[3] = label;
    CREATE_LABEL_TIME(130, 140);
    listSetings[7] = labelTimeDisplay[4] = label;
    CREATE_LABEL_TIME(220, 140);
    listSetings[8] = labelTimeDisplay[5] = label;

//    switchTimerEn
    //help label
    label = lv_label_create(parent);
    lv_label_set_text(label, "Auto switch");
    lv_obj_align_to(label, switchTimerEn, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);//align position

    label = lv_label_create(parent);
    lv_label_set_text(label, "hPa|mmHg");
    lv_obj_align_to(label, switchPressure, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);//align position

    label = lv_label_create(parent);
    lv_label_set_text(label, "Time:");
    lv_obj_align_to(label, labelTimeDisplay[0], LV_ALIGN_OUT_LEFT_MID, -20, 0);//align position

    label = lv_label_create(parent);
    lv_label_set_text(label, "Main display");
    lv_obj_align_to(label, labelTimeDisplay[0], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);//align position

    label = lv_label_create(parent);
    lv_label_set_text(label, "Display 1");
    lv_obj_align_to(label, labelTimeDisplay[1], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);//align position

    label = lv_label_create(parent);
    lv_label_set_text(label, "Display 2");
    lv_obj_align_to(label, labelTimeDisplay[2], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);//align position

    label = lv_label_create(parent);
    lv_label_set_text(label, "Display 3");
    lv_obj_align_to(label, labelTimeDisplay[3], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);//align position

    label = lv_label_create(parent);
    lv_label_set_text(label, "Ext data");
    lv_obj_align_to(label, labelTimeDisplay[4], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);//align position

    label = lv_label_create(parent);
    lv_label_set_text(label, "Weather");
    lv_obj_align_to(label, labelTimeDisplay[5], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);//align position

    for(int i = 0 ; i < DISPLAY_SIZE; i++)
        if(labelTimeDisplay[i] != NULL)
            lv_label_set_text_fmt(labelTimeDisplay[i], "%d", timeOnDisplay[i]);

#if LV_USE_THEME_DEFAULT
    switchTheme = lv_switch_create(parent);
    lv_obj_set_pos(switchTheme, 220, 10);
    lv_obj_add_event_cb(switchTheme, switchTheme_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    listSetings[2] = switchTheme;

    //create widget select object
    color_changer_create(parent);

    listSetings[9] = btnColorSelect;

    label = lv_label_create(parent);
    lv_label_set_text(label, "dark theme");
    lv_obj_align_to(label, switchTheme, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);//align position
#endif

    lv_style_init(&styleListSettings);
    //text
    lv_style_set_outline_color(&styleListSettings, lv_theme_get_color_primary(NULL));
    lv_style_set_outline_pad(&styleListSettings, 2);


    for(int i = 0; i < SITING_LIST_SIZE; i++)
    {
        if(listSetings[i] == NULL)
            continue;
//        lv_obj_set_style_outline_color(listSetings[i], lv_theme_get_color_primary(NULL), 0);
//        lv_obj_set_style_outline_pad(listSetings[i], 2, 0);
        lv_obj_add_style(listSetings[i], &styleListSettings, 0);
        lv_obj_add_event_cb(listSetings[i], focusChange_event_cb, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(listSetings[i], focusChange_event_cb, LV_EVENT_DEFOCUSED, NULL);

        lv_group_add_obj(settingFocuse, listSetings[i]);
    }


//    //swelect object TODO check onouther way
//    lv_obj_set_style_outline_width(switchTimerEn, 2, 0);
//    lv_obj_set_style_outline_color(switchTimerEn, lv_theme_get_color_primary(parent), 0);
//    lv_obj_set_style_outline_pad(switchTimerEn, 2, 0);
}

static void color_changer_anim_cb(void * var, int32_t v)
{
    lv_obj_t * obj = var;
    lv_coord_t max_w = lv_obj_get_width(lv_obj_get_parent(obj)) - LV_DPX(20);
    lv_coord_t w;

    w = lv_map(v, 0, 256, LV_DPX(52), max_w);
    lv_obj_set_width(obj, w);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));

    if(v > LV_OPA_COVER) v = LV_OPA_COVER;

    uint32_t i;
    for(i = 0; i < lv_obj_get_child_cnt(obj); i++) {
        lv_obj_set_style_opa(lv_obj_get_child(obj, i), v, 0);
    }
}

static void color_changer_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t * color_cont = lv_event_get_user_data(e);
        if(lv_obj_get_width(color_cont) < LV_HOR_RES / 2) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, color_cont);
            lv_anim_set_exec_cb(&a, color_changer_anim_cb);
            lv_anim_set_values(&a, 0, 256);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
        }
        else {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, color_cont);
            lv_anim_set_exec_cb(&a, color_changer_anim_cb);
            lv_anim_set_values(&a, 256, 0);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
        }
    }
}

static void color_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_DEFOCUSED)
    {
        lv_obj_set_style_outline_width(obj, 0, 0);
    }
    else if(code == LV_EVENT_FOCUSED) {
        lv_obj_set_style_outline_width(obj, 2, 0);

        lv_palette_t * palette_primary = lv_event_get_user_data(e);
        lv_palette_t palette_secondary = (*palette_primary) + 3; /*Use another palette as secondary*/
        if(palette_secondary >= _LV_PALETTE_LAST) palette_secondary = 0;
#if LV_USE_THEME_DEFAULT
        lv_theme_default_init(NULL, lv_palette_main(*palette_primary), lv_palette_main(palette_secondary),
                              lv_obj_has_state(switchTheme, LV_STATE_CHECKED), fontMain);
#endif
//        for(int i = 0; i < SITING_LIST_SIZE; i++)
//        {
//            if(listSetings[i] == NULL)
//                continue;
//            lv_obj_set_style_outline_color(listSetings[i], lv_theme_get_color_primary(NULL), 0);
//        }
        lv_style_set_outline_color(&styleListSettings, lv_theme_get_color_primary(NULL));

        lv_style_set_bg_color(&styleButtons, lv_theme_get_color_primary(NULL));
    }
}

static void color_changer_create(lv_obj_t * parent)
{
    static lv_palette_t palette[] = {
        LV_PALETTE_BLUE, LV_PALETTE_GREEN, LV_PALETTE_BLUE_GREY,  LV_PALETTE_ORANGE,
        LV_PALETTE_RED, LV_PALETTE_PURPLE, LV_PALETTE_TEAL, _LV_PALETTE_LAST
    };

    //create select group
    colorFocuse = lv_group_create();

    colorCont = lv_obj_create(parent);
    lv_obj_remove_style_all(colorCont);
    lv_obj_set_flex_flow(colorCont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(colorCont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(colorCont, LV_OBJ_FLAG_FLOATING);

    lv_obj_set_style_bg_color(colorCont, lv_color_white(), 0);
    lv_obj_set_style_pad_right(colorCont, LV_DPX(47), 0);
    lv_obj_set_style_bg_opa(colorCont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(colorCont, LV_RADIUS_CIRCLE, 0);

    lv_obj_set_size(colorCont, LV_DPX(52), LV_DPX(52));

    lv_obj_align(colorCont, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));

    uint32_t i;
    for(i = 0; palette[i] != _LV_PALETTE_LAST; i++) {
        lv_obj_t * c = lv_btn_create(colorCont);
        lv_obj_set_style_bg_color(c, lv_palette_main(palette[i]), 0);
        lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_opa(c, LV_OPA_TRANSP, 0);
        lv_obj_set_size(c, 20, 20);
        lv_obj_add_event_cb(c, color_event_cb, LV_EVENT_ALL, &palette[i]);
        lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

        lv_obj_set_style_outline_color(c, lv_palette_main(palette[i]), 0);
        lv_obj_set_style_outline_pad(c, 2, 0);
        lv_group_add_obj(colorFocuse, c);
    }

    btnColorSelect = lv_btn_create(parent);
    lv_obj_t * btn = btnColorSelect;
    lv_obj_add_flag(btn, LV_OBJ_FLAG_FLOATING | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(btn, lv_color_white(), LV_STATE_CHECKED);
    lv_obj_set_style_pad_all(btn, 10, 0);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_event_cb(btn, color_changer_event_cb, LV_EVENT_ALL, colorCont);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_TINT, 0);

    lv_obj_set_size(btn, LV_DPX(42), LV_DPX(42));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(15), -LV_DPX(15));
}
