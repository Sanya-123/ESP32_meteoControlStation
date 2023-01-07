#include "display.h"
#include "decode_image.h"
#include "esp_log.h"

//LVGL
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "../lvgl/lvgl.h"
#endif
#include "lvgl_helpers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "csevent.h"

//#include "lv_demo_widgets.h"
//#include "lv_ex_get_started.h"
#include "display_gui.h"

lv_disp_t * disp;

#define LV_TICK_PERIOD_MS 10

void initDisplay();


static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xGuiSemaphore;
lv_color_t* buf1 = NULL;
lv_color_t* buf2 = NULL;
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
const esp_timer_create_args_t periodic_timer_args = {
    .callback = &lv_tick_task,
    .name = "periodic_gui"
};
esp_timer_handle_t periodic_timer;

bool takeGuiSem(uint32_t timeout)
{
    return xSemaphoreTake(xGuiSemaphore, timeout) == pdTRUE;
}

void giveGuiSem()
{
    xSemaphoreGive(xGuiSemaphore);
}

void guiTask(void *pvParameter)
{
    void (*application)(void);
    application = pvParameter;
    initDisplay();
    /* Create the demo application */
//    create_demo_application();//show log


    if(pvParameter != NULL)
        application();
//    vTaskDelay(pdMS_TO_TICKS(1000));

    lv_task_handler();//include once for drow display mau be include afre
    sendEvent(EVENT_DISPLAY_INIT_DONE);

    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if(takeGuiSem(portMAX_DELAY)) {
//        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
//            xSemaphoreGive(xGuiSemaphore);
            giveGuiSem();
       }
    }

    /* A task should NEVER return */
    free(buf1);
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
#if CONFIG_DOUBLE_DIPLAY_BUFFER
    free(buf2);
#endif
#endif
    vTaskDelete(NULL);
}

void initDisplay()
{
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();

    buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    /* Use double buffered when not working with monochrome displays */
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
#if CONFIG_DOUBLE_DIPLAY_BUFFER
    buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);
#else
    buf2 = NULL;
#endif
#else
    buf2 = NULL;
#endif

    uint32_t size_in_px = DISP_BUF_SIZE;

#if defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_IL3820         \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_JD79653A    \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_UC8151D     \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_SSD1306

    /* Actual size in pixels, not bytes. */
    size_in_px *= 8;
#endif

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = disp_driver_flush;


    /* When using a monochrome display we need to register the callbacks:
     * - rounder_cb
     * - set_px_cb */
#ifdef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    disp_drv.rounder_cb = disp_driver_rounder;
    disp_drv.set_px_cb = disp_driver_set_px;
#endif

    disp_drv.draw_buf = &disp_buf;
    ESP_LOGI("Display", "hir:%d;ver:%d\n\r", LV_HOR_RES, LV_VER_RES);
    disp = lv_disp_drv_register(&disp_drv);
    ESP_LOGI("Display", "hir:%d;ver:%d\n\r", LV_HOR_RES, LV_VER_RES);

//    lv_disp_set_rotation(disp, LV_DISP_ROT_90);//don't work with ila9341(only in config)

    /* Register an input device when enabled on the menuconfig */
#if CONFIG_LV_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#endif

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

}
