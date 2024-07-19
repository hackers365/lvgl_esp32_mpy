#ifndef __LVGL_ESP32_DISPLAY_H__
#define __LVGL_ESP32_DISPLAY_H__

#include "qspi.h"
#include "esp_lcd_st77916.h"
#include "esp_lcd_types.h"
#include "py/obj.h"
#include "driver/ledc.h"
typedef void (*lvgl_esp32_transfer_done_cb_t)(void *);

typedef struct lvgl_esp32_Display_obj_t
{
    mp_obj_base_t base;

    uint16_t width;
    uint16_t height;

    lvgl_esp32_QSPI_obj_t *spi;
    uint8_t reset;
    uint8_t cs;
    uint8_t blk;
    uint32_t pixel_clock;

    bool swap_xy;
    bool mirror_x;
    bool mirror_y;
    bool invert;
    bool bgr;

    lvgl_esp32_transfer_done_cb_t transfer_done_cb;
    void *transfer_done_user_data;

    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_io_handle_t io_handle;
    ledc_channel_config_t ledc_channel;
    ledc_timer_config_t ledc_timer;
} lvgl_esp32_Display_obj_t;

extern const mp_obj_type_t lvgl_esp32_Display_type;

void lvgl_esp32_Display_draw_bitmap(
    lvgl_esp32_Display_obj_t *self,
    int x_start,
    int y_start,
    int x_end,
    int y_end,
    const void *data
);

#endif /* __LVGL_ESP32_DISPLAY_H__ */
