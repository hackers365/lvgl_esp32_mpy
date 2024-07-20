//
// Created by feiyu on 2024/7/19.
//

#ifndef __LVGL_ESP32__TOUCH_H
#define __LVGL_ESP32__TOUCH_H
#include "py/obj.h"
#include "driver/gpio.h"
#include "esp_lcd_types.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_cst816s.h"
typedef struct lvgl_esp32_Touch_obj_t
{
    mp_obj_base_t base;
    uint8_t scl;
    uint8_t sda;
    uint8_t inter;
    uint8_t rst;
    uint8_t ic2_num;
    uint16_t width;
    uint16_t height;
    bool swap_xy;
    bool mirror_x;
    bool mirror_y;
    esp_lcd_touch_handle_t tp ;
    esp_lcd_panel_io_handle_t tp_io_handle ;
} lvgl_esp32_Touch_obj_t;

extern const mp_obj_type_t lvgl_esp32_Touch_type;
#endif //__LVGL_ESP32__TOUCH_H
