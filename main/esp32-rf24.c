#include <string.h>

#include "esp32-rf24.h"

#include "esp_log.h"

static const char *TAG = "[RF24]";

/**
 * RF24 commands
 */
#define RF24CMD_R_REGISTER          0x00
#define RF24CMD_W_REGISTER          0x20
#define RF24CMD_R_RX_PAYLOAD        0x61
#define RF24CMD_W_TX_PAYLOAD        0xA0
#define RF24CMD_FLUSH_TX            0xE1
#define RF24CMD_FLUSH_RX            0xE2
#define RF24CMD_REUSE_TX_PL         0xE3
#define RF24CMD_ACTIVATE            0x50
#define RF24CMD_R_RX_PL_WID         0x60
#define RF24CMD_W_ACK_PAYLOAD       0xA8
#define RF24CMD_W_TX_PAYLOAD_NOACK  0xB0
#define RF24CMD_NOP                 0xFF

#define RF24_REG_MASK 0x1F

/**
 * RF24 registers
 */
#define RF24REG_CONFIG      0x00
#define RF24REG_EN_AA       0x01
#define RF24REG_EN_RXADDR   0x02
#define RF24REG_SETUP_AW    0x03
#define RF24REG_SETUP_RETR  0x04
#define RF24REG_RF_CH       0x05
#define RF24REG_RF_SETUP    0x06
#define RF24REG_STATUS      0x07
#define RF24REG_OBSERVE_TX  0x08
#define RF24REG_CD          0x09
#define RF24REG_RX_ADDR_P0  0x0A
#define RF24REG_RX_ADDR_P1  0x0B
#define RF24REG_RX_ADDR_P2  0x0C
#define RF24REG_RX_ADDR_P3  0x0D
#define RF24REG_RX_ADDR_P4  0x0E
#define RF24REG_RX_ADDR_P5  0x0F
#define RF24REG_TX_ADDR     0x10
#define RF24REG_RX_PW_P0    0x11
#define RF24REG_RX_PW_P1    0x12
#define RF24REG_RX_PW_P2    0x13
#define RF24REG_RX_PW_P3    0x14
#define RF24REG_RX_PW_P4    0x15
#define RF24REG_RX_PW_P5    0x16
#define RF24REG_FIFO_STATUS 0x17
#define RF24REG_DYNPD       0x1C
#define RF24REG_FEATURE     0x1D

/* RF24REG_CONFIG */
#define RF24BIT_MASK_RX_DR  0x40
#define RF24BIT_MASK_TX_DS  0x20
#define RF24BIT_MASK_MAX_RT 0x10
#define RF24BIT_EN_CRC      0x08
#define RF24BIT_CRCO        0x04
#define RF24BIT_PWR_UP      0x02
#define RF24BIT_PRIM_RX     0x01

/* RF24REG_EN_AA */
#define RF24BIT_ENAA_P5     0x20
#define RF24BIT_ENAA_P4     0x10
#define RF24BIT_ENAA_P3     0x08
#define RF24BIT_ENAA_P2     0x04
#define RF24BIT_ENAA_P1     0x02
#define RF24BIT_ENAA_P0     0x01

/* RF24REG_EN_RXADDR */
#define RF24BIT_ERX_P5      0x20
#define RF24BIT_ERX_P4      0x10
#define RF24BIT_ERX_P3      0x08
#define RF24BIT_ERX_P2      0x04
#define RF24BIT_ERX_P1      0x02
#define RF24BIT_ERX_P0      0x01

/* RF24REG_SETUP_AW */
#define RF24BIT_AW          /*0x03*/0

/* RF24REG_SETUP_RETR */
#define RF24BIT_ARD         /*0xF0*/4
#define RF24BIT_ARC         /*0x0F*/0

/* RF24REG_RF_CH */
#define RF24BIT_RF_CH       0x7F

/* RF24REG_RF_SETUP */
#define RF24BIT_PLL_LOCK    /*0x10*/4
#define RF24BIT_RF_DR       /*0x08*/3
#define RF24BIT_RF_PWR      0x06
#define RF24BIT_LNA_HCURR   0x01

/* RF24REG_STATUS */
#define RF24BIT_RX_DR       0x40
#define RF24BIT_TX_DS       0x20
#define RF24BIT_MAX_RT      0x10
#define RF24BIT_RX_P_NO     0x0E
#define RF24BIT_TX_FULL     0x0

/* RF24REG_OBSERVE_TX */
#define RF24BIT_PLOS_CNT    /*0xF0*/4
#define RF24BIT_ARC_CNT     /*0x0F*/0

/* RF24REG_CD */
#define RF24BIT_CD          0x01

/* RF24REG_RX_PW_P0 ~ RF24REG_RX_PW_P5 */
#define RF24BIT_RX_PW       0x3F

/* RF24REG_FIFO_STATUS */
#define RF24BIT_FIFO_TX_REUSE    0x40
#define RF24BIT_FIFO_TX_FULL     0x20
#define RF24BIT_FIFO_TX_EMPTY    0x10
#define RF24BIT_FIFO_RX_FULL     0x02
#define RF24BIT_FIFO_RX_EMPTY    0x01

/* RF24REG_DYNPD */
#define RF24BIT_DPL_P5      0x20
#define RF24BIT_DPL_P4      0x10
#define RF24BIT_DPL_P3      0x08
#define RF24BIT_DPL_P2      0x04
#define RF24BIT_DPL_P1      0x02
#define RF24BIT_DPL_P0      0x01

/* RF24REG_FEATURE */
#define RF24BIT_EN_DPL      0x04
#define RF24BIT_EN_ACK_PAY  0x02
#define RF24BIT_EN_DYN_ACK  0x01

struct rf24_dev_t {
    rf24_bus_cfg_t bus_cfg;
    bool bus_initialized;
    spi_device_handle_t spi_dev;
};

#define RF24_CHECK(a, str, ret_val, ...)                                      \
    if (!(a)) {                                                               \
        ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        return (ret_val);                                                     \
    }

#define RF24_BIT_TEST(data, mask) (((data) & (mask)) == (mask))
#define RF24_BIT_TEST_10(data, mask) (RF24_BIT_TEST((data), (mask)) ? 1 : 0)

esp_err_t rf24_cmd(rf24_dev_handle_t handle, uint8_t cmd, uint8_t *res) {
    esp_err_t ret;
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
        .length = 8,
        .tx_buffer = &cmd,
        .rx_buffer = res
    };
    t.tx_data[0] = cmd;

//    ret = spi_device_polling_transmit(handle->spi_dev, &t);
    ret = spi_device_transmit(handle->spi_dev, &t);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "rf24_cmd: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return ret;
    }

    *res = t.rx_data[0];
    return ESP_OK;
}

esp_err_t rf24_read_reg(rf24_dev_handle_t handle, uint8_t reg, uint8_t *data) {
    esp_err_t ret;
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
        .length = 16,
    };
    t.tx_data[0] = RF24CMD_R_REGISTER | reg;
    t.tx_data[1] = RF24CMD_NOP;

//    ret = spi_device_polling_transmit(handle->spi_dev, &t);
    ret = spi_device_transmit(handle->spi_dev, &t);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "rf24_read_reg: spi_device_polling_transmit failed: %s", esp_err_to_name(ret));
        return ret;
    }

    *data = t.rx_data[1];
    return ESP_OK;
}

/**
 * Public functions.
 */

esp_err_t rf24_init(rf24_bus_cfg_t *bus_cfg, rf24_dev_handle_t *handle) {
    ESP_LOGV(TAG, "rf24_init");

    RF24_CHECK(bus_cfg != NULL, "invalid bus config", ESP_ERR_INVALID_ARG);
    RF24_CHECK(GPIO_IS_VALID_OUTPUT_GPIO(bus_cfg->ce_io_num), "invalid CE pin", ESP_ERR_INVALID_ARG);
    RF24_CHECK(handle != NULL, "invalid output handle", ESP_ERR_INVALID_ARG);

    esp_err_t ret = ESP_OK;

    struct rf24_dev_t *rf_dev = malloc(sizeof(struct rf24_dev_t));
    if (rf_dev == NULL) {
        ESP_LOGW(TAG, "rf24_init: malloc failed");
        ret = ESP_ERR_NO_MEM;
        goto cleanup;
    }

    memset(rf_dev, 0, sizeof(struct rf24_dev_t));
    rf_dev->bus_cfg = *bus_cfg;

    // Initialize bus if needed.
    if (bus_cfg->init_host) {
        spi_bus_config_t spi_buscfg = {
            .mosi_io_num = bus_cfg->mosi_io_num,
            .miso_io_num = bus_cfg->miso_io_num,
            .sclk_io_num = bus_cfg->sclk_io_num,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1
        };

        ret = spi_bus_initialize(bus_cfg->spi_host, &spi_buscfg, 1);

        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "rf24_init: spi_bus_initialize failed: %s", esp_err_to_name(ret));
            goto cleanup;
        }
        rf_dev->bus_initialized = true;
    }

    // Initialize device.
    spi_device_interface_config_t spi_devcfg={
        .clock_speed_hz = 8000000,
        .mode = 0,
        .spics_io_num = bus_cfg->cs_io_num,
        .queue_size = 7
    };

    ESP_LOGV(TAG, "rf24_init: spi_bus_add_device");
    spi_device_handle_t spi_dev;
    ret = spi_bus_add_device(bus_cfg->spi_host, &spi_devcfg, &spi_dev);

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "rf24_init: spi_bus_add_device failed: %s", esp_err_to_name(ret));
        goto cleanup;
    }
    rf_dev->spi_dev = spi_dev;

    // Initialize GPIO for CE pin.
    gpio_set_direction(bus_cfg->ce_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(bus_cfg->ce_io_num, 0);

    *handle = rf_dev;
    return ret;

cleanup:
    ESP_LOGV(TAG, "rf24_init: cleanup");

    if (rf_dev) {
        if (rf_dev->bus_initialized) {
            spi_bus_free(bus_cfg->spi_host);
        }
        if (rf_dev->spi_dev) {
            spi_bus_remove_device(rf_dev->spi_dev);
        }
        free(rf_dev);
    }
    return ret;
}

esp_err_t rf24_free(rf24_dev_handle_t handle)
{
    ESP_LOGV(TAG, "rf24_free");

    RF24_CHECK(handle != NULL, "invalid handle", ESP_ERR_INVALID_ARG);

    // Free SPI device and bus.
    ESP_LOGV(TAG, "rf24_free: spi_bus_remove_device");
    spi_bus_remove_device(handle->spi_dev);

    if (handle->bus_initialized) {
        ESP_LOGV(TAG, "rf24_free: spi_bus_free");
        spi_bus_free(handle->bus_cfg.spi_host);
    }

    // Free GPIO for CE pin.
    gpio_reset_pin(handle->bus_cfg.ce_io_num);

    free(handle);
    return ESP_OK;
}

esp_err_t rf24_get_status(rf24_dev_handle_t handle, rf24_status *status) {
    ESP_LOGV(TAG, "rf24_get_status");

    uint8_t res;
    esp_err_t ret = rf24_cmd(handle, RF24CMD_NOP, &res);
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "rf24_get_status: send cmd failed");
        return ret;
    }

    rf24_status s = {
        .rx_data_ready = RF24_BIT_TEST_10(res, RF24BIT_RX_DR),
        .tx_data_sent = RF24_BIT_TEST_10(res, RF24BIT_TX_DS),
        .tx_max_retried = RF24_BIT_TEST_10(res, RF24BIT_MAX_RT),
        .rx_pipe_no = ((res & RF24BIT_RX_P_NO) >> 1),
        .tx_full = RF24_BIT_TEST_10(res, RF24BIT_TX_FULL)
    };

    *status = s;
    return ESP_OK;
}

esp_err_t rf24_is_chip_connected(rf24_dev_handle_t handle, bool *connected) {
    ESP_LOGV(TAG, "rf24_is_chip_connected");

    uint8_t data;
    esp_err_t ret = rf24_read_reg(handle, RF24REG_SETUP_AW, &data);

    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "rf24_get_status: read register failed");
        return ret;
    }

    *connected = data >= 1 && data <= 3;
    return ESP_OK;
}
