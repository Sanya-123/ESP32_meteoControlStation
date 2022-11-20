/**
 * @file lv_demo_meteo.c
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

//TODO
#undef LV_HOR_RES
#define LV_HOR_RES 320

#undef LV_VER_RES
#define LV_VER_RES 240

/*********************
 *      DEFINES
 *********************/
#define BUTTON_HEIGHT       30
#define BORDER_BUTTONS      2

//size rectangle for mersurment data
#define SMALL_RECT_HEIGHT   70
#define SMALL_RECT_WIDTH    110
#define BIG_RECT_HEIGHT     140
#define BIG_RECT_WIDTH      210

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
static void readKeyButton(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static void btnSide_event_cb(lv_event_t * e);//button left and right
static void btnCtrl_event_cb(lv_event_t * e);//button conf
static void btnAction_event_cb(lv_event_t * e);//button action
static void timer_event_cb(lv_timer_t * timer);
static void mainPage_create(lv_obj_t * parent);

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

static DisplayState stateDisplay = StateMainDisplay;
uint32_t timeOnDisplay[3] = {3000, 1000, 1000};//time state on display

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
//static lv_style_t styleTime, styleDate;

//sitings display
static lv_obj_t * displaySettings;
static lv_obj_t * switchTimerEn;

static int indexSelectedSettings = 0;
static lv_obj_t * selectedSetings;//curent select sitings in listSetings
static lv_obj_t *listSetings[1] = {/*switchTimerEn*/};


/**********************
 *      MACROS
 **********************/
#define CREATE_BUTTON(btn, label, x) do{ btn = lv_btn_create(lv_scr_act());   \
                                        label = lv_label_create(btn);   \
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

    fontMain = LV_FONT_DEFAULT;
    fontData = LV_FONT_DEFAULT;

#if LV_FONT_MONTSERRAT_14
        fontMain    =  &lv_font_montserrat_14;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_16 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.")
#endif

#if LV_FONT_MONTSERRAT_16
        fontData    =  &lv_font_montserrat_16;
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
    lv_style_set_text_color(&styleButtons, (lv_color_t)LV_COLOR_MAKE(0xFF, 0xFF, 0xFF));
    lv_style_set_text_font(&styleButtons, fontMain);
    lv_style_set_text_opa(&styleButtons, LV_OPA_100);
    //radius
    lv_style_set_radius(&styleButtons, 0);
    //border style
    lv_style_set_border_width(&styleButtons, BORDER_BUTTONS);
    lv_style_set_border_side(&styleButtons, LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_RIGHT);

//    lv_style_init(&styleButtonRight);
//    lv_style_set_text_color(&styleButtonRight, (lv_color_t)LV_COLOR_MAKE(0xFF, 0xFF, 0xFF));
//    lv_style_set_radius(&styleButtonRight, 0);
//    lv_style_set_border_width(&styleButtonRight, BORDER_BUTTONS);
//    lv_style_set_border_side(&styleButtonRight, LV_BORDER_SIDE_TOP);

    // lv_style_init(&style_bullet);
    // lv_style_set_border_width(&style_bullet, 0);
    // lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

    //remove sliders
    lv_obj_remove_style(lv_scr_act(), NULL, LV_PART_SCROLLBAR /*| LV_STATE_ANY*/);

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
    lv_label_set_text(labelAction, "act");

    static uint8_t typeLeft = 1, typeRight = 0;

    lv_obj_add_event_cb(buttonLeft, btnSide_event_cb, LV_EVENT_ALL, &typeLeft);
    lv_obj_add_event_cb(buttonRight, btnSide_event_cb, LV_EVENT_ALL, &typeRight);
    lv_obj_add_event_cb(buttonSettings, btnCtrl_event_cb, LV_EVENT_ALL, &typeRight);

    //windows
    /*Tile1: just a label*/
    displayMain = lv_tileview_add_tile(tl, StateMainDisplay, 0, LV_DIR_RIGHT);
    mainPage_create(displayMain);


    //setings page
    displaySettings = lv_tileview_add_tile(tl, 0, 1, LV_DIR_NONE);
    settingPage_create(displaySettings);
    /*Tile1: just a label*/
    lv_obj_t * tile2 = lv_tileview_add_tile(tl, StateExternalData, 0, LV_DIR_HOR);
    lv_obj_t * label2 = lv_label_create(tile2);
    lv_label_set_text(label2, "Scroll RIGHT | LEFT");
    lv_obj_center(label2);

    /*Tile1: just a label*/
    lv_obj_t * tile3 = lv_tileview_add_tile(tl, StateWheather, 0, LV_DIR_LEFT);
    lv_obj_t * label3 = lv_label_create(tile3);
    lv_label_set_text(label3, "Scroll LEFT");
    lv_obj_center(label3);

    

    // if(disp_size == DISP_LARGE) {
    //     lv_obj_t * tab_btns = lv_tabview_get_tab_btns(tv);
    //     lv_obj_set_style_pad_left(tab_btns, LV_HOR_RES / 2, 0);
    //     lv_obj_t * logo = lv_img_create(tab_btns);
    //     LV_IMG_DECLARE(img_lvgl_logo);
    //     lv_img_set_src(logo, &img_lvgl_logo);
    //     lv_obj_align(logo, LV_ALIGN_LEFT_MID, -LV_HOR_RES / 2 + 25, 0);

    //     lv_obj_t * label = lv_label_create(tab_btns);
    //     lv_obj_add_style(label, &style_title, 0);
    //     lv_label_set_text(label, "LVGL v8");
    //     lv_obj_align_to(label, logo, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);

    //     label = lv_label_create(tab_btns);
    //     lv_label_set_text(label, "Widgets demo");
    //     lv_obj_add_style(label, &style_text_muted, 0);
    //     lv_obj_align_to(label, logo, LV_ALIGN_OUT_RIGHT_BOTTOM, 10, 0);
    // }

    // lv_obj_t * t1 = lv_tabview_add_tab(tv, "Profile");
    // lv_obj_t * t2 = lv_tabview_add_tab(tv, "Analytics");
    // lv_obj_t * t3 = lv_tabview_add_tab(tv, "Shop");
    // profile_create(t1);
    // analytics_create(t2);
    // shop_create(t3);

    // color_changer_create(tv);

    //create timer for change display
    timerNextDisplay = lv_timer_create(timer_event_cb, timeOnDisplay[0], NULL);
    lv_timer_pause(timerNextDisplay);//TODO temp
}

void meteoButtonClicked(Button button, ButtonState state)
{
    buttonChange = button;
    buttonState = state;
}
//TODO check values
void setCO2(int val)
{
    lv_label_set_text_fmt(labelCo2, "%d ppm", val);
}

void setHumiditi(int val)
{
    lv_label_set_text_fmt(labelHumidity, "%d%%", val);
}

void setTemperature(int val)
{
    lv_label_set_text_fmt(labelTemperatre, "%d C", val);
}

void setPressure(int val)
{
    lv_label_set_text_fmt(labelPressure, "%d hPa", val);
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

//andler analize input buttons
static void readKeyButton(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void)indev_drv;
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

            lv_timer_set_period(timerNextDisplay, timeOnDisplay[stateDisplay]);//set period display
            lv_timer_reset(timerNextDisplay);//reset count timer
            lv_obj_set_tile_id(tl, stateDisplay, 0, LV_ANIM_ON);//set display
        }
    }
    else
    {

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
        }
    }
    else
    {
        if(code == LV_EVENT_CLICKED)
        {
            stateDisplay = oldState;

            //set privius state and continiu timer
            lv_timer_resume(timerNextDisplay);
            lv_timer_reset(timerNextDisplay);

            lv_timer_set_period(timerNextDisplay, timeOnDisplay[stateDisplay]);//set period display
            lv_obj_set_tile_id(tl, stateDisplay, 0, LV_ANIM_ON);//set display
        }

    }
}

static void btnAction_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
}

static void timer_event_cb(lv_timer_t * timer)
{
    (void)timer;
//    static int time = 0;
//    time++;
//    if(time == 5001)
//        time = 700;
//    lv_label_set_text_fmt(labelCo2, "%d ppm", time);

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

static void mainPage_create(lv_obj_t * parent)
{
//    lv_obj_remove_style(parent, NULL, LV_PART_SCROLLBAR);
//    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    lv_style_init(&styleDisplayMain);
    lv_style_set_text_color(&styleDisplayMain, (lv_color_t)LV_COLOR_MAKE(0x00, 0x00, 0x00));
    lv_style_set_text_font(&styleDisplayMain, fontData);
    lv_style_set_radius(&styleDisplayMain, 0);
    lv_style_set_border_width(&styleDisplayMain, 1);
    lv_style_set_border_color(&styleDisplayMain, (lv_color_t)LV_COLOR_MAKE(0x00, 0x00, 0x00));

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

    lv_obj_center(labelCo2);
    lv_obj_center(labelHumidity);
    lv_obj_center(labelTemperatre);
    lv_obj_center(labelPressure);

    labelTime = lv_label_create(parent);
    lv_obj_set_style_text_font(labelTime, &lv_font_montserrat_40, 0);
    lv_obj_set_pos(labelTime, 2, 15);
    lv_label_set_text(labelTime, "20:47");

    labelDate = lv_label_create(parent);
    lv_obj_set_style_text_font(labelDate, &lv_font_montserrat_18, 0);
    lv_obj_set_pos(labelDate, 115, 40);
    lv_label_set_text(labelDate, "23.03.2022");

    lv_label_set_text(labelCo2, "5000 ppm");
    lv_label_set_text(labelHumidity, "100%");
    lv_label_set_text(labelTemperatre, "45 C");
    lv_label_set_text(labelPressure, "1045 hPa");
}

static void settingPage_create(lv_obj_t * parent)
{
    color_changer_create(parent);

    //crate checkbox eneble timer switch
    switchTimerEn = lv_switch_create(parent);
    lv_obj_set_pos(switchTimerEn, 15, 10);
    lv_obj_add_state(switchTimerEn, LV_STATE_CHECKED);
    lv_obj_add_event_cb(switchTimerEn, switchTimer_event_cb, LV_EVENT_VALUE_CHANGED, NULL);


//    switchTimerEn
    lv_obj_t * label = lv_label_create(parent);
    lv_obj_set_pos(label, 2, 40);
    lv_label_set_text(label, "Auto switch");
//    lv_obj_align_to();

    //swelect object TODO check onouther way
    lv_obj_set_style_outline_width(switchTimerEn, 2, 0);
    lv_obj_set_style_outline_color(switchTimerEn, lv_theme_get_color_primary(parent), 0);
    lv_obj_set_style_outline_pad(switchTimerEn, 2, 0);
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

    if(code == LV_EVENT_FOCUSED) {
        lv_obj_t * color_cont = lv_obj_get_parent(obj);
        if(lv_obj_get_width(color_cont) < LV_HOR_RES / 2) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, color_cont);
            lv_anim_set_exec_cb(&a, color_changer_anim_cb);
            lv_anim_set_values(&a, 0, 256);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
        }
    }
    else if(code == LV_EVENT_CLICKED) {
        lv_palette_t * palette_primary = lv_event_get_user_data(e);
        lv_palette_t palette_secondary = (*palette_primary) + 3; /*Use another palette as secondary*/
        if(palette_secondary >= _LV_PALETTE_LAST) palette_secondary = 0;
#if LV_USE_THEME_DEFAULT
        lv_theme_default_init(NULL, lv_palette_main(*palette_primary), lv_palette_main(palette_secondary),
                              LV_THEME_DEFAULT_DARK, fontMain);
#endif
//        lv_color_t color = lv_palette_main(*palette_primary);
//        lv_style_set_text_color(&style_icon, color);
//        lv_chart_set_series_color(chart1, ser1, color);
//        lv_chart_set_series_color(chart2, ser3, color);
    }
}

static void color_changer_create(lv_obj_t * parent)
{
    static lv_palette_t palette[] = {
        LV_PALETTE_BLUE, LV_PALETTE_GREEN, LV_PALETTE_BLUE_GREY,  LV_PALETTE_ORANGE,
        LV_PALETTE_RED, LV_PALETTE_PURPLE, LV_PALETTE_TEAL, _LV_PALETTE_LAST
    };

    lv_obj_t * color_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(color_cont);
    lv_obj_set_flex_flow(color_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(color_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(color_cont, LV_OBJ_FLAG_FLOATING);

    lv_obj_set_style_bg_color(color_cont, lv_color_white(), 0);
    lv_obj_set_style_pad_right(color_cont, LV_DPX(47), 0);
    lv_obj_set_style_bg_opa(color_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(color_cont, LV_RADIUS_CIRCLE, 0);

    lv_obj_set_size(color_cont, LV_DPX(52), LV_DPX(52));

    lv_obj_align(color_cont, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));

    uint32_t i;
    for(i = 0; palette[i] != _LV_PALETTE_LAST; i++) {
        lv_obj_t * c = lv_btn_create(color_cont);
        lv_obj_set_style_bg_color(c, lv_palette_main(palette[i]), 0);
        lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_opa(c, LV_OPA_TRANSP, 0);
        lv_obj_set_size(c, 20, 20);
        lv_obj_add_event_cb(c, color_event_cb, LV_EVENT_ALL, &palette[i]);
        lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }

    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_FLOATING | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(btn, lv_color_white(), LV_STATE_CHECKED);
    lv_obj_set_style_pad_all(btn, 10, 0);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_event_cb(btn, color_changer_event_cb, LV_EVENT_ALL, color_cont);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_TINT, 0);

    lv_obj_set_size(btn, LV_DPX(42), LV_DPX(42));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(15), -LV_DPX(15));
}
