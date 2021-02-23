
#include "ST7735.h"

/*
 The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct.
*/
/*typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes;  // No of data in data, 0xFF = end of cmds
} lcd_init_cmd_t;*/

static spi_device_handle_t spi_dev;

#define ST7735_TFTWIDTH_128 128   // for 1.44 and mini
#define ST7735_TFTWIDTH_80 80     // for mini
#define ST7735_TFTHEIGHT_128 128  // for 1.44" display
#define ST7735_TFTHEIGHT_160 160  // for 1.8" and mini display

#define LCD_WIDTH 320
#define LCD_HEIGHT 240

#define ILI9488_WIDTH  320	//y
#define ILI9488_HEIGHT 480	//x

// ST77XX commands
#define ST_CMD_DELAY 0x80  // special signifier for command lists

#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

// ST7735 commands
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

//ILI command
#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13

#define ILI9341_RDMODE  0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0D
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_GAMMASET 0x26

#define ILI9341_PIXFMT  0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDID4   0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1

#define ILI9341_POWERA	0xCB
#define ILI9341_POWERB	0xCF
#define ILI9341_POWER_SEQ       0xED
#define ILI9341_DTCA	0xE8
#define ILI9341_DTCB	0xEA
#define ILI9341_PRC	0xF7
#define ILI9341_3GAMMA_EN	0xF2

//TFT
#define TFT_INVOFF     0x20
#define TFT_INVONN     0x21
#define TFT_DISPOFF    0x28
#define TFT_DISPON     0x29
#define TFT_CASET      0x2A
#define TFT_PASET      0x2B
#define TFT_RAMWR      0x2C
#define TFT_RAMRD      0x2E
#define TFT_MADCTL	   0x36
#define TFT_PTLAR 	   0x30
#define TFT_ENTRYM 	   0xB7
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

#define PARARELL_LINE       (LCD_HEIGHT / 4)

// RGB-565 16bit, 128*160;
static uint8_t display_buff[LCD_WIDTH * PARARELL_LINE * 2];

//DRAM_ATTR static const lcd_init_cmd_t st7735_init_cmds[] = {
//    // software reset with delay
//    {ST77XX_SWRESET, {0}, ST_CMD_DELAY},
//    // Out of sleep mode with delay
//    {ST77XX_SLPOUT, {0}, ST_CMD_DELAY},
//    // Framerate ctrl - normal mode. Rate = fosc/(1x2+40) * (LINE+2C+2D)
//    {ST7735_FRMCTR1, {0x01, 0x2C, 0x2D}, 3},
//    // Framerate ctrl - idle mode.  Rate = fosc/(1x2+40) * (LINE+2C+2D)
//    {ST7735_FRMCTR2, {0x01, 0x2C, 0x2D}, 3},
//    // Framerate - partial mode. Dot/Line inversion mode
//    {ST7735_FRMCTR3, {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D}, 6},
//    // Display inversion ctrl: No inversion
//    {ST7735_INVCTR, {0x07}, 1},
//    // Power control1 set GVDD: -4.6V, AUTO mode.
//    {ST7735_PWCTR1, {0xA2, 0x02, 0x84}, 3},
//    // Power control2 set VGH/VGL: VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
//    {ST7735_PWCTR2, {0xC5}, 1},
//    // Power control3 normal mode(Full color): Op-amp current small, booster voltage
//    {ST7735_PWCTR3, {0x0A, 0x00}, 2},
//    // Power control4 idle mode(8-colors): Op-amp current small & medium low
//    {ST7735_PWCTR4, {0x8A, 0x2A}, 2},
//    // Power control5 partial mode + full colors
//    {ST7735_PWCTR5, {0x8A, 0xEE}, 2},
//    // VCOMH VoltageVCOM control 1: VCOMH=0x0E=2.850
//    {ST7735_VMCTR1, {0x0E}, 1},
//    // Display Inversion Off
//    {ST77XX_INVOFF, {0}, 0},
//    // Memory Data Access Control: top-bottom/left-right refresh
//    {ST77XX_MADCTL, {0xC8}, 1},
//    // Color mode, Interface Pixel Format: RGB-565, 16-bit/pixel
//    {ST77XX_COLMOD, {0x05}, 1},

//    // Column Address Set: 2, 127+2
//    {ST77XX_CASET, {0x00, 0x02, 0x00, 0x7F + 0x02}, 4},
//    // Row Address Set: 1,159+1
//    {ST77XX_RASET, {0x00, 0x01, 0x00, 0x9F + 0x01}, 4},

//    // Gamma Adjustments (pos. polarity). Not entirely necessary, but provides accurate colors.
//    {ST7735_GMCTRP1,
//     {0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10},
//     16},
//    // Gamma Adjustments (neg. polarity). Not entirely necessary, but provides accurate colors.
//    {ST7735_GMCTRN1,
//     {0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10},
//     16},
//    // Normal Display Mode On
//    {ST77XX_NORON, {0}, ST_CMD_DELAY},
//    // Display On
//    {ST77XX_DISPON, {0}, ST_CMD_DELAY},
//    {0, {0}, 0xFF},
//};

 /*DRAM_ATTR static const lcd_init_cmd_t st7735_init_cmds[] = {
    {ILI9341_SWRESET, {0}, ST_CMD_DELAY},
    {ILI9341_POWERA, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    {ILI9341_POWERB, {0x00, 0XC1, 0X30}, 3},
    {0xEF, {0x03, 0x80, 0x02}, 3},
    {ILI9341_DTCA, {0x85, 0x00, 0x78}, 3},
    {ILI9341_DTCB, {0x00, 0x00}, 2},
    {ILI9341_POWER_SEQ, { 0x64, 0x03, 0X12, 0X81}, 4},
    {ILI9341_PRC, {0x20}, 1},
    {ILI9341_PWCTR1, {0x23}, 1},
    {ILI9341_PWCTR2, {0x10}, 1},
    {ILI9341_VMCTR1, {0x3E, 0x28}, 2},
    {ILI9341_VMCTR2, {0x86}, 1},
    {TFT_MADCTL, {MADCTL_MV | MADCTL_BGR}, 1},
    {ILI9341_PIXFMT, {0x55}, 1},
    {ILI9341_FRMCTR1, {0x00, 0x18}, 2},
    {ILI9341_DFUNCTR, {0x08, 0x82, 0x27}, 3},
    {TFT_PTLAR, {0x00, 0x00, 0x01, 0x3F}, 4},
    {ILI9341_3GAMMA_EN, {0x00}, 1},
    {ILI9341_GAMMASET, {0x01}, 1},
    {ILI9341_GMCTRP1, {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                       0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15},
    {ILI9341_GMCTRN1, {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                       0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15},
    {ILI9341_SLPOUT, {0}, ST_CMD_DELAY},
    {TFT_DISPON, {0}, 0}
};*/

//To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many. More means more memory use,
//but less overhead for setting up / finishing transfers. Make sure 240 is dividable by this.
#define PARALLEL_LINES 16

/*
 The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct.
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

typedef enum {
    LCD_TYPE_ILI = 1,
    LCD_TYPE_ST,
    LCD_TYPE_MAX,
} type_lcd_t;

//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
//DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[]={
//    /* Memory Data Access Control, MX=MV=1, MY=ML=MH=0, RGB=0 */
//    {0x36, {(1<<5)|(1<<6)}, 1},
//    /* Interface Pixel Format, 16bits/pixel for RGB/MCU interface */
//    {0x3A, {0x55}, 1},
//    /* Porch Setting */
//    {0xB2, {0x0c, 0x0c, 0x00, 0x33, 0x33}, 5},
//    /* Gate Control, Vgh=13.65V, Vgl=-10.43V */
//    {0xB7, {0x45}, 1},
//    /* VCOM Setting, VCOM=1.175V */
//    {0xBB, {0x2B}, 1},
//    /* LCM Control, XOR: BGR, MX, MH */
//    {0xC0, {0x2C}, 1},
//    /* VDV and VRH Command Enable, enable=1 */
//    {0xC2, {0x01, 0xff}, 2},
//    /* VRH Set, Vap=4.4+... */
//    {0xC3, {0x11}, 1},
//    /* VDV Set, VDV=0 */
//    {0xC4, {0x20}, 1},
//    /* Frame Rate Control, 60Hz, inversion=0 */
//    {0xC6, {0x0f}, 1},
//    /* Power Control 1, AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V */
//    {0xD0, {0xA4, 0xA1}, 1},
//    /* Positive Voltage Gamma Control */
//    {0xE0, {0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19}, 14},
//    /* Negative Voltage Gamma Control */
//    {0xE1, {0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19}, 14},
//    /* Sleep Out */
//    {0x11, {0}, 0x80},
//    /* Display On */
//    {0x29, {0}, 0x80},
//    {0, {0}, 0xff}
//};

DRAM_ATTR static const lcd_init_cmd_t ili_init_cmds[]={
    /* Power contorl B, power control = 0, DC_ENA = 1 */
    {0xCF, {0x00, 0x83, 0X30}, 3},
    /* Power on sequence control,
     * cp1 keeps 1 frame, 1st frame enable
     * vcl = 0, ddvdh=3, vgh=1, vgl=2
     * DDVDH_ENH=1
     */
    {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
    /* Driver timing control A,
     * non-overlap=default +1
     * EQ=default - 1, CR=default
     * pre-charge=default - 1
     */
    {0xE8, {0x85, 0x01, 0x79}, 3},
    /* Power control A, Vcore=1.6V, DDVDH=5.6V */
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    /* Pump ratio control, DDVDH=2xVCl */
    {0xF7, {0x20}, 1},
    /* Driver timing control, all=0 unit */
    {0xEA, {0x00, 0x00}, 2},
    /* Power control 1, GVDD=4.75V */
    {0xC0, {0x26}, 1},
    /* Power control 2, DDVDH=VCl*2, VGH=VCl*7, VGL=-VCl*3 */
    {0xC1, {0x11}, 1},
    /* VCOM control 1, VCOMH=4.025V, VCOML=-0.950V */
    {0xC5, {0x35, 0x3E}, 2},
    /* VCOM control 2, VCOMH=VMH-2, VCOML=VML-2 */
    {0xC7, {0xBE}, 1},
    /* Memory access contorl, MX=MY=0, MV=1, ML=0, BGR=1, MH=0 */
    {0x36, {0x28}, 1},
    /* Pixel format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x55}, 1},
    /* Frame rate control, f=fosc, 70Hz fps */
    {0xB1, {0x00, 0x1B}, 2},
    /* Enable 3G, disabled */
    {0xF2, {0x08}, 1},
    /* Gamma set, curve 1 */
    {0x26, {0x01}, 1},
    /* Positive gamma correction */
    {0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
    /* Negative gamma correction */
    {0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
    /* Column address set, SC=0, EC=0xEF */
    {0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
    /* Page address set, SP=0, EP=0x013F */
    {0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
    /* Memory write */
    {0x2C, {0}, 0},
    /* Entry mode set, Low vol detect disabled, normal display */
    {0xB7, {0x07}, 1},
    /* Display function control */
    {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
    /* Sleep out */
    {0x11, {0}, 0x80},
    /* Display on */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff},
};

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    //ret=spi_device_queue_trans(spi, &t, portMAX_DELAY);
    assert(ret==ESP_OK);            //Should have had no issues.
}


void send_line_finish()
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
    //Wait for all 6 transactions to be done and get back the results.
    for (int x=0; x<6; x++) {
        ret=spi_device_get_trans_result(spi_dev, &rtrans, portMAX_DELAY);
        assert(ret==ESP_OK);
        //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
    }
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    //ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    ret=spi_device_queue_trans(spi, &t, portMAX_DELAY);
    assert(ret==ESP_OK);            //Should have had no issues.
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

uint32_t lcd_get_id(spi_device_handle_t spi)
{
    //get_id cmd
    lcd_cmd(spi, 0x04);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=8*3;
    t.flags = SPI_TRANS_USE_RXDATA;
    t.user = (void*)1;

    esp_err_t ret = spi_device_polling_transmit(spi, &t);
    assert( ret == ESP_OK );

    return *(uint32_t*)t.rx_data;
}

//Initialize the display
void lcd_init(spi_device_handle_t spi)
{
    int cmd=0;
    const lcd_init_cmd_t* lcd_init_cmds;

    //Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);

    //detect LCD type
    uint32_t lcd_id = lcd_get_id(spi);
    int lcd_detected_type = 0;
    int lcd_type;

    printf("LCD ID: %08X\n", lcd_id);
    if ( lcd_id == 0 ) {
        //zero, ili
        lcd_detected_type = LCD_TYPE_ILI;
        printf("ILI9341 detected.\n");
    } else {
        // none-zero, ST
        lcd_detected_type = LCD_TYPE_ST;
        printf("ST7789V detected.\n");
    }

/*#ifdef CONFIG_LCD_TYPE_AUTO
    lcd_type = lcd_detected_type;
#elif defined( CONFIG_LCD_TYPE_ST7789V )
    printf("kconfig: force CONFIG_LCD_TYPE_ST7789V.\n");
    lcd_type = LCD_TYPE_ST;
#elif defined( CONFIG_LCD_TYPE_ILI9341 )
    printf("kconfig: force CONFIG_LCD_TYPE_ILI9341.\n");
    lcd_type = LCD_TYPE_ILI;
#endif
    if ( lcd_type == LCD_TYPE_ST ) {
        printf("LCD ST7789V initialization.\n");
        lcd_init_cmds = st_init_cmds;
    } else {
        printf("LCD ILI9341 initialization.\n");
        lcd_init_cmds = ili_init_cmds;
    }*/
    
    lcd_init_cmds = ili_init_cmds;

    //Send all the commands
    while (lcd_init_cmds[cmd].databytes!=0xff) {
        lcd_cmd(spi, lcd_init_cmds[cmd].cmd);
        lcd_data(spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes&0x1F);
        if (lcd_init_cmds[cmd].databytes&0x80) {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }

    ///Enable backlight
    gpio_set_level(PIN_NUM_BCKL, 0);
}

static void tft_set_address_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4];
    lcd_cmd(spi_dev, ST77XX_CASET);
    data[0] = (x0 >> 8);
    data[1] = x0 + OFFSET_X;
    data[2] = (x1 >> 8);
    data[3] = x1 + OFFSET_X;
    lcd_data(spi_dev, data, 4);

    lcd_cmd(spi_dev, ST77XX_RASET);
    data[0] = (y0 >> 8);
    data[1] = y0 + OFFSET_Y;
    data[2] = (y1 >> 8);
    data[3] = y1 + OFFSET_Y;
    lcd_data(spi_dev, data, 4);

    // memory write
    lcd_cmd(spi_dev, ST77XX_RAMWR);
}

void send_picturte_line(int xpos, int ypos, int W, int H, uint16_t *linedata)
{
    esp_err_t ret;
    int x;
    tft_set_address_window(xpos, ypos, xpos+W, ypos+H);
    //Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
    //function is finished because the SPI driver needs access to it even while we're already calculating the next line.
    static spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.tx_buffer=linedata;        //finally send the line data
    trans.length=W*H*2*8;          //Data length, in bits
    trans.flags=0; //undo SPI_TRANS_USE_TXDATA flag
    trans.user=(void*)1;

    ret=spi_device_queue_trans(spi_dev, &trans, portMAX_DELAY);
    assert(ret==ESP_OK);
}

void send_picturte(int xpos, int ypos, int W, int H, uint16_t **picture)
{
    for(int i = 0; i < H; i++)
    {
    
	send_picturte_line(xpos, ypos + H - 1 - i, W, 1, picture[i]);
	//send_line_finish();
    }
}

void read_picturte_line(int xpos, int ypos, int W, int H, uint16_t *linedata)
{
    esp_err_t ret;
    int x;
    //Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
    //function is finished because the SPI driver needs access to it even while we're already calculating the next line.
    static spi_transaction_t trans[6];

    //In theory, it's better to initialize trans and data only once and hang on to the initialized
    //variables. We allocate them on the stack, so we need to re-init them each call.
    for (x=0; x<6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x&1)==0) {
            //Even transfers are commands
            trans[x].length=8;
            trans[x].user=(void*)0;
        } else {
            //Odd transfers are data
            trans[x].length=8*4;
            trans[x].user=(void*)1;
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0]=TFT_CASET;           //Column Address Set
    trans[1].tx_data[0]=xpos>>8;        //Start Col High
    trans[1].tx_data[1]=xpos&0xff;      //Start Col Low
    trans[1].tx_data[2]=(xpos+W)>>8;       //End Col High
    trans[1].tx_data[3]=(xpos+W)&0xff;     //End Col Low
    trans[2].tx_data[0]=TFT_PASET;           //Page address set
    trans[3].tx_data[0]=ypos>>8;        //Start page high
    trans[3].tx_data[1]=ypos&0xff;      //start page low
    trans[3].tx_data[2]=(ypos+H)>>8;    //end page high
    trans[3].tx_data[3]=(ypos+H)&0xff;  //end page low
    trans[4].tx_data[0]=TFT_RAMRD;           //memory write

    trans[5].rx_buffer=linedata;        //finally send the line data
    trans[5].tx_buffer=NULL;        //finally send the line data
    trans[5].length=W*H*2*8;          //Data length, in bits
    trans[5].flags=0; //undo SPI_TRANS_USE_TXDATA flag

    //Queue all transactions.
    for (x=0; x<6; x++) {
        ret=spi_device_queue_trans(spi_dev, &trans[x], portMAX_DELAY);
        assert(ret==ESP_OK);
    }

    //When we are here, the SPI driver is busy (in the background) getting the transactions sent. That happens
    //mostly using DMA, so the CPU doesn't have much to do here. We're not going to wait for the transaction to
    //finish because we may as well spend the time calculating the next line. When that is done, we can call
    //send_line_finish, which will wait for the transfers to be done and check their status.

    //send_line_finish(spi_dev);
}

void read_picturte(int xpos, int ypos, int W, int H, uint16_t **picture)
{
    for(int i = 0; i < H; i++)
    {

        read_picturte_line(xpos, ypos + H - 1 - i, W, 1, picture[i]);
        //send_line_finish();
    }
}

void tft_fill_screen(uint16_t color) {
    for (int i = 0; i < (LCD_WIDTH * PARARELL_LINE * 2); i = i + 2) {
        display_buff[i] = color & 0xFF;
        display_buff[i + 1] = color >> 8;
    }

    for(int i = 0; i < LCD_HEIGHT; i += PARARELL_LINE)
    {
        tft_set_address_window(0, i, LCD_WIDTH - 1, PARARELL_LINE + i - 1);
        lcd_data(spi_dev, display_buff, LCD_WIDTH * PARARELL_LINE * 2);
    }
//    tft_set_address_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT/2 - 1);
//    lcd_data(spi_dev, display_buff, LCD_WIDTH * LCD_HEIGHT);
//    tft_set_address_window(0, LCD_HEIGHT/2, LCD_WIDTH - 1, LCD_HEIGHT - 1);
//    lcd_data(spi_dev, display_buff, LCD_WIDTH * LCD_HEIGHT);
}

//TODO razdelit na pararel lines
void tft_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // rudimentary clipping (drawChar w/big text requires this)
    if ((x > LCD_WIDTH) || (y > LCD_HEIGHT)) return;
    if ((x + w - 1) >= LCD_WIDTH) w = LCD_WIDTH - x;
    if ((y + h - 1) >= LCD_HEIGHT) h = LCD_HEIGHT - y;

    tft_set_address_window(x, y, x + w - 1, y + h - 1);

    //lcd_cmd(spi_dev, ST77XX_RAMWR);

    for (int i = 0; i < (w * h * 2); i+=2) {
        display_buff[i] = color & 0xFF;
        display_buff[i + 1] = (color >> 8);
    }
    lcd_data(spi_dev, display_buff, w * h * 2);
}

void tft_invert_color(int i) {
    if (i) {
        lcd_cmd(spi_dev, ST77XX_INVON);
    } else {
        lcd_cmd(spi_dev, ST77XX_INVOFF);
    }
}

void tft_init() {
	esp_err_t ret;
    //spi_device_handle_t spi;
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=ILI9488_HEIGHT*ILI9488_WIDTH*2/2+8
    };
    spi_device_interface_config_t devcfg={
#ifdef CONFIG_LCD_OVERCLOCK
        .clock_speed_hz=26*1000*1000,           //Clock out at 26 MHz
#else
        .clock_speed_hz=10*1000*1000,           //Clock out at 10 MHz
#endif
        .mode=0,                                //SPI mode 0
        .spics_io_num=PIN_NUM_CS,               //CS pin
        .queue_size=7,                          //We want to be able to queue 7 transactions at a time
        .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(VSPI_HOST, &buscfg, 2);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(VSPI_HOST, &devcfg, &spi_dev);
    ESP_ERROR_CHECK(ret);
    //Initialize the LCD
    lcd_init(spi_dev);
}

void tft_draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= LCD_HEIGHT) || (y < 0) || (y >= LCD_WIDTH)) return;

    display_buff[0] = color & 0xFF;
    display_buff[1] = color >> 8;
    tft_set_address_window(x, y, 1, 1);
    lcd_data(spi_dev, display_buff, 2);
}

//i <-> j потомучто дисплей повернут на 90 градусов
void tft_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color, uint8_t size) {
    if ((x >= LCD_WIDTH) ||          // Clip right
            (y >= LCD_HEIGHT) ||         // Clip bottom
            ((x + 6 * size - 1) < 0) ||  // Clip left
            ((y + 8 * size - 1) < 0))    // Clip top
        return;

    for (int8_t i = 0; i < 5; i++) {  // Char bitmap = 5 columns
        uint8_t line = std_font[c * 5 + i];
        for (int8_t j = 0; j < 8; j++, line <<= 1) {
            if (line & 0x80) {
                tft_rect(x - (i * size), y + (j * size), size, size, color);
            } else if (bg_color != color) {
                tft_rect(x - (i * size), y + (j * size), size, size, bg_color);
            }
        }
    }
}

uint32_t tft_draw_string(uint16_t x, uint16_t y, const char *pt, int16_t color, int16_t bg_color, uint8_t size) {
    // check row and colume
    uint32_t /*x_offset = 7,*/ x_offset = 5 + 1;  // font size 5x7.

    uint32_t count = 0;
    y = y;
    //  if (y > 15) return 0;
    while (*pt) {
        tft_draw_char(x/* * x_offset*/, y/* * y_offset*/, *pt, color, bg_color, size);
        pt++;
        //    x = x + size*x_offset;
        if (x < (x_offset*size)) return count;  // number of characters printed
        x = x - x_offset*size;
        count++;
    }
    return count;  // number of characters printed
}
