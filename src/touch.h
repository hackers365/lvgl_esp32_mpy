//
// Created by feiyu on 2024/7/19.
//

#ifndef __LVGL_ESP32__TOUCH_H
#define __LVGL_ESP32__TOUCH_H
#include "py/obj.h"
typedef struct lvgl_esp32_Touch_obj_t
{
    mp_obj_base_t base;
    uint8_t scl;
    uint8_t sda;
    uint8_t inter;
    uint8_t rst;
} lvgl_esp32_Touch_obj_t;

extern const mp_obj_type_t lvgl_esp32_Touch_type;
#endif //__LVGL_ESP32__TOUCH_H
