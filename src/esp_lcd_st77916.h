/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>

#include "esp_lcd_panel_vendor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_LCD_ST77916_VER_MAJOR    (0)
#define ESP_LCD_ST77916_VER_MINOR    (0)
#define ESP_LCD_ST77916_VER_PATCH    (2)
/**
 * @brief LCD panel initialization commands.
 *
 */
typedef struct {
    int cmd;                /*<! The specific LCD command */
    const void *data;       /*<! Buffer that holds the command specific data */
    size_t data_bytes;      /*<! Size of `data` in memory, in bytes */
    unsigned int delay_ms;  /*<! Delay in milliseconds after this command */
} esp_lcd_panel_vendor_init_cmd_t;

/**
 * @brief LCD panel vendor configuration.
 *
 * @note  This structure needs to be passed to the `vendor_config` field in `esp_lcd_panel_dev_config_t`.
 *
 */
typedef struct {
    const esp_lcd_panel_vendor_init_cmd_t *init_cmds;   /*!< Pointer to initialization commands array. Set to NULL if using default commands.
                                                         *   The array should be declared as `static const` and positioned outside the function.
                                                         *   Please refer to `vendor_specific_init_default` in source file.
                                                         */
    unsigned int init_cmds_size;                        /*<! Number of commands in above array */

#if SOC_LCD_RGB_SUPPORTED
    const esp_lcd_rgb_panel_config_t *rgb_config;   /*!< RGB panel configuration. */
#endif

    struct {
        unsigned int mirror_by_cmd: 1;              /*<! The `mirror()` function will be implemented by LCD command if set to 1.
                                                     *   Otherwise, the function will be implemented by software.
                                                     */
        unsigned int auto_del_panel_io: 1;          /*<! Delete the panel IO instance automatically if set to 1. All `*_by_cmd` flags will be invalid.
                                                     *   If the panel IO pins are sharing other pins of the RGB interface to save GPIOs,
                                                     *   Please set it to 1 to release the panel IO and its pins (except CS signal).
                                                     */
        /* Only available for QSPI interface */
        unsigned int use_qspi_interface: 1;         /*<! Set to 1 if use QSPI interface, default is SPI interface */
    } flags;
} esp_lcd_panel_vendor_config_t;
/**
 * @brief Create LCD panel for model ST77916
 *
 * @param[in]  io LCD panel IO handle
 * @param[in]  panel_dev_config General panel device configuration (Use `vendor_config` to select QSPI interface or override default initialization commands)
 * @param[out] ret_panel Returned LCD panel handle
 * @return
 *      - ESP_OK: Success
 *      - Otherwise: Fail
 */
esp_err_t esp_lcd_new_panel_st77916(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel);

/**
 * @brief LCD panel bus configuration structure
 *
 */
#define ST77916_PANEL_BUS_SPI_CONFIG(sclk, mosi, max_trans_sz)  \
    {                                                           \
        .sclk_io_num = sclk,                                    \
        .mosi_io_num = mosi,                                    \
        .miso_io_num = -1,                                      \
        .quadhd_io_num = -1,                                    \
        .quadwp_io_num = -1,                                    \
        .max_transfer_sz = max_trans_sz,                        \
    }
#define ST77916_PANEL_BUS_QSPI_CONFIG(sclk, d0, d1, d2, d3, max_trans_sz)\
    {                                                           \
        .sclk_io_num = sclk,                                    \
        .data0_io_num = d0,                                     \
        .data1_io_num = d1,                                     \
        .data2_io_num = d2,                                     \
        .data3_io_num = d3,                                     \
        .max_transfer_sz = max_trans_sz,                        \
    }

/**
 * @brief LCD panel IO configuration structure
 *
 */
#define ST77916_PANEL_IO_SPI_CONFIG(cs, dc, cb, cb_ctx)         \
    {                                                           \
        .cs_gpio_num = cs,                                      \
        .dc_gpio_num = dc,                                      \
        .spi_mode = 0,                                          \
        .pclk_hz = 40 * 1000 * 1000,                            \
        .trans_queue_depth = 10,                                \
        .on_color_trans_done = cb,                              \
        .user_ctx = cb_ctx,                                     \
        .lcd_cmd_bits = 8,                                      \
        .lcd_param_bits = 8,                                    \
    }
#define ST77916_PANEL_IO_QSPI_CONFIG(cs, cb, cb_ctx)            \
    {                                                           \
        .cs_gpio_num = cs,                                      \
        .dc_gpio_num = -1,                                      \
        .spi_mode = 0,                                          \
        .pclk_hz = 40 * 1000 * 1000,                            \
        .trans_queue_depth = 10,                                \
        .on_color_trans_done = cb,                              \
        .user_ctx = cb_ctx,                                     \
        .lcd_cmd_bits = 32,                                     \
        .lcd_param_bits = 8,                                    \
        .flags = {                                              \
            .quad_mode = true,                                  \
        },                                                      \
    }

#ifdef __cplusplus
}
#endif
