//
// Created by apexracing on 2024/7/19.
//
#include "touch.h"

#include "py/runtime.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "lvgl_esp32_touch";
static mp_obj_t lvgl_esp32_Touch_init(mp_obj_t self_ptr)
{
    lvgl_esp32_Touch_obj_t *self = MP_OBJ_TO_PTR(self_ptr);

    ESP_LOGI(TAG, "Setting up panel IO");

    return mp_obj_new_int_from_uint(0);
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvgl_esp32_Touch_init_obj, lvgl_esp32_Touch_init);

static mp_obj_t lvgl_esp32_Touch_deinit(mp_obj_t self_ptr)
{
    lvgl_esp32_Touch_obj_t *self = MP_OBJ_TO_PTR(self_ptr);



    return mp_obj_new_int_from_uint(0);
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvgl_esp32_Touch_deinit_obj, lvgl_esp32_Touch_deinit);

static mp_obj_t lvgl_esp32_Touch_make_new(
        const mp_obj_type_t *type,
        size_t n_args,
        size_t n_kw,
        const mp_obj_t *all_args
)
{
    enum
    {
        ARG_scl,          // width of the Touch
        ARG_sda,         // height of the Touch
        ARG_rst,            // configured SPI instance
        ARG_inter,          // RESET pin number
    };

    static const mp_arg_t allowed_args[] = {
            { MP_QSTR_sl, MP_ARG_INT | MP_ARG_REQUIRED },
            { MP_QSTR_sda, MP_ARG_INT | MP_ARG_REQUIRED },
            { MP_QSTR_rst, MP_ARG_INT | MP_ARG_REQUIRED },
            { MP_QSTR_inter, MP_ARG_INT | MP_ARG_KW_ONLY, { .u_int = 0}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    lvgl_esp32_Touch_obj_t *self = mp_obj_malloc_with_finaliser(lvgl_esp32_Touch_obj_t, &lvgl_esp32_Touch_type);

    self->scl = args[ARG_scl].u_int;
    self->ARG_sda = args[ARG_sda].u_int;

    self->rst = args[ARG_rst].u_int;
    self->inter = args[ARG_inter].u_int;

    return MP_OBJ_FROM_PTR(self);
}

static const mp_rom_map_elem_t lvgl_esp32_Touch_locals_table[] = {
        { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&lvgl_esp32_Touch_init_obj) },
        { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&lvgl_esp32_Touch_deinit_obj) },
        { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&lvgl_esp32_Touch_deinit_obj) },
};

static MP_DEFINE_CONST_DICT(lvgl_esp32_Touch_locals, lvgl_esp32_Touch_locals_table);

MP_DEFINE_CONST_OBJ_TYPE(
        lvgl_esp32_Touch_type,
        MP_QSTR_Touch,
        MP_TYPE_FLAG_NONE,
        make_new,
        lvgl_esp32_Touch_make_new,
        locals_dict,
&lvgl_esp32_Touch_locals
);