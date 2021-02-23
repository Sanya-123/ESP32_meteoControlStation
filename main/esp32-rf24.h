#ifndef INC__ESP32_RF24__H
#define INC__ESP32_RF24__H

#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

/**
 * Constants
 */

#define ESP_ERR_RF24_BASE 0x2F000

/**
 * Structures
 */

typedef struct {
    spi_host_device_t spi_host;  ///< SPI host to use.
    bool init_host;              /** Initialize SPI bus before adding device.
        Set to ``true`` if the current device is the only device on this SPI bus.
        Set to ``false`` if the SPI host has already been initialized for sharing with other devices.
        */
    gpio_num_t mosi_io_num;      ///< GPIO pin for Master Out Slave In (=spi_d) signal, or -1 if not used.
    gpio_num_t miso_io_num;      ///< GPIO pin for Master In Slave Out (=spi_q) signal, or -1 if not used.
    gpio_num_t sclk_io_num;      ///< GPIO pin for Spi CLocK signal, or -1 if not used.
    gpio_num_t cs_io_num;        ///< GPIO pin for CS.
    gpio_num_t ce_io_num;        ///< GPIO pin for CE.
} rf24_bus_cfg_t;

typedef struct rf24_dev_t *rf24_dev_handle_t;

typedef struct {
    unsigned int rx_data_ready: 1;
    unsigned int tx_data_sent: 1;
    unsigned int tx_max_retried: 1;
    unsigned int rx_pipe_no: 3;  // 0-5, or ``RF24_RX_FIFO_EMPTY``
    unsigned int tx_full: 1;
} rf24_status;

#define RF24_RX_FIFO_EMPTY 7
/**
 * Functions
 */

/**
 * @brief Initialize a RF24 device connected to a SPI bus as a slave device.
 *
 * @param bus_cfg SPI bus config.
 * @param handle Pointer to variable to hold the RF24 device handle.
 * @return
 *         - ESP_ERR_INVALID_ARG   if parameter is invalid
 *         - ESP_ERR_NO_MEM        if out of memory
 *         - ESP_OK                on success
 */
esp_err_t rf24_init(rf24_bus_cfg_t *bus_cfg, rf24_dev_handle_t *handle);

/**
 * @brief Remove a RF24 device and free the resources.
 *
 * @param handle RF24 device handle to free.
 * @return
 *         - ESP_ERR_INVALID_ARG   if parameter is invalid
 *         - ESP_OK                on success
 */
esp_err_t rf24_free(rf24_dev_handle_t handle);

esp_err_t rf24_get_status(rf24_dev_handle_t handle, rf24_status *status);

esp_err_t rf24_is_chip_connected(rf24_dev_handle_t handle, bool *connected);

#endif  /* INC__ESP32_RF24__H */
