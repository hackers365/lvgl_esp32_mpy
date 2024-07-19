//
// Created by apexracing on 2024/7/19.
//
#include "touch.h"

#include "py/runtime.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/i2c.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_touch_cst816s.h"
static const char *TAG = "lvgl_esp32_touch";


// get_finger_position 方法
static mp_obj_t  lvgl_esp32_Touch_read_data(mp_obj_t self_ptr) {
    lvgl_esp32_Touch_obj_t *self = MP_OBJ_TO_PTR(self_ptr);
    uint16_t touch_x[1];
    uint16_t touch_y[1];
    uint16_t touch_strength[1];
    uint8_t touch_cnt = 0;
    ESP_ERROR_CHECK(esp_lcd_touch_get_coordinates(self->tp, touch_x, touch_y, touch_strength, &touch_cnt, CONFIG_ESP_LCD_TOUCH_MAX_POINTS));
    mp_obj_t tuple[3];
    tuple[0] = mp_obj_new_int(touch_cnt);
    tuple[1] = mp_obj_new_int(touch_x[0]);
    tuple[2] = mp_obj_new_int(touch_y[0]);
    return mp_obj_new_tuple(3, tuple);
}

static MP_DEFINE_CONST_FUN_OBJ_1(lvgl_esp32_Touch_read_data_obj, lvgl_esp32_Touch_read_data);

static mp_obj_t lvgl_esp32_Touch_init(mp_obj_t self_ptr)
{
    lvgl_esp32_Touch_obj_t *self = MP_OBJ_TO_PTR(self_ptr);

    ESP_LOGI(TAG,"Initializing I2C for display touch");
    const i2c_config_t i2c_conf = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = self->sda,
            .scl_io_num = self->scl,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = 400000,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, i2c_conf.mode, 0, 0, 0));
    ESP_LOGI(TAG,"Initializing IO for display touch");
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)self->ic2_num, &tp_io_config, &self->tp_io_handle));
    esp_lcd_touch_config_t tp_cfg= {
        .x_max = self->width,
        .y_max = self->height,
        .rst_gpio_num = (gpio_num_t)self->rst,
        .int_gpio_num = (gpio_num_t)self->inte,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = self->swap_xy,
            .mirror_x = self->mirror_x,
            .mirror_y = self->mirror_y,
        },
        .process_coordinates = NULL,
        .interrupt_callback = NULL,
        .user_data = NULL,
        .driver_data = NULL,
    };
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(self->tp_io_handle, &tp_cfg, &self->tp));
    ESP_LOGI(TAG,"Initializing display touch Finish");
    return mp_obj_new_int_from_uint(0);
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvgl_esp32_Touch_init_obj, lvgl_esp32_Touch_init);

static mp_obj_t lvgl_esp32_Touch_deinit(mp_obj_t self_ptr)
{
    lvgl_esp32_Touch_obj_t *self = MP_OBJ_TO_PTR(self_ptr);
    if(self->tp != NULL)
    {
        ESP_LOGI(TAG, "Deinitializing Touch driver");
        ESP_ERROR_CHECK(del(self->tp));
        self->tp = NULL;
    }

    if(self->tp_io_handle != NULL)
    {
        ESP_LOGI(TAG, "Deinitializing panel IO");
        ESP_ERROR_CHECK(esp_lcd_panel_io_del(self->tp_io_handle));
        self->tp_io_handle = NULL;
    }
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
        ARG_width,          // width of the display
        ARG_height,         // height of the display
        ARG_swap_xy,        // swap X and Y axis
        ARG_mirror_x,       // mirror on X axis
        ARG_mirror_y,       // mirror on Y axis
        ARG_scl,          // width of the Touch
        ARG_sda,         // height of the Touch
        ARG_rst,            // configured SPI instance
        ARG_inter,          // RESET pin number
        ARG_i2c_num,

    };

    static const mp_arg_t allowed_args[] = {
            { MP_QSTR_sl, MP_ARG_INT | MP_ARG_REQUIRED },
            { MP_QSTR_sda, MP_ARG_INT | MP_ARG_REQUIRED },
            { MP_QSTR_rst, MP_ARG_INT | MP_ARG_REQUIRED },
            { MP_QSTR_width, MP_ARG_INT | MP_ARG_REQUIRED },
            { MP_QSTR_height, MP_ARG_INT | MP_ARG_REQUIRED },
            { MP_QSTR_swap_xy, MP_ARG_BOOL | MP_ARG_KW_ONLY, { .u_bool = false }},
            { MP_QSTR_mirror_x, MP_ARG_BOOL | MP_ARG_KW_ONLY, { .u_bool = false }},
            { MP_QSTR_mirror_y, MP_ARG_BOOL | MP_ARG_KW_ONLY, { .u_bool = false }},
            { MP_QSTR_inter, MP_ARG_INT | MP_ARG_KW_ONLY, { .u_int = GPIO_NUM_NC}},
            { MP_QSTR_i2c_num, MP_ARG_INT | MP_ARG_KW_ONLY, { .u_int = I2C_NUM_0}},

    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    lvgl_esp32_Touch_obj_t *self = mp_obj_malloc_with_finaliser(lvgl_esp32_Touch_obj_t, &lvgl_esp32_Touch_type);

    self->scl = args[ARG_scl].u_int;
    self->sda = args[ARG_sda].u_int;
    self->rst = args[ARG_rst].u_int;
    self->width = args[ARG_width].u_int;
    self->height = args[ARG_height].u_int;
    self->swap_xy = args[ARG_swap_xy].u_bool;
    self->mirror_x = args[ARG_mirror_x].u_bool;
    self->mirror_y = args[ARG_mirror_y].u_bool;
    self->inter = args[ARG_inter].u_int;
    self->ic2_num=args[ARG_i2c_num].u_int;
    self->tp=NULL;
    self->tp_io_handle=NULL;
    return MP_OBJ_FROM_PTR(self);
}

static const mp_rom_map_elem_t lvgl_esp32_Touch_locals_table[] = {
        { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&lvgl_esp32_Touch_init_obj) },
        { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&lvgl_esp32_Touch_deinit_obj) },
        { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&lvgl_esp32_Touch_deinit_obj) },
        { MP_ROM_QSTR(MP_QSTR_read_data), MP_ROM_PTR(&lvgl_esp32_Touch_read_data_obj) },
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