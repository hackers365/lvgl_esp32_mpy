# Make sure LVGL gets built
add_compile_options("-Wno-unused-value")
add_compile_options("-Wno-error")

include(${CMAKE_CURRENT_LIST_DIR}/binding/binding.cmake)

add_library(usermod_lvgl_esp32 INTERFACE)

target_sources(usermod_lvgl_esp32 INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/esp_lcd_touch.c
        ${CMAKE_CURRENT_LIST_DIR}/src/esp_lcd_touch_cst816s.c
        ${CMAKE_CURRENT_LIST_DIR}/src/esp_lcd_st77916.c
        ${CMAKE_CURRENT_LIST_DIR}/src/qspi.c
        ${CMAKE_CURRENT_LIST_DIR}/src/spi.c
        ${CMAKE_CURRENT_LIST_DIR}/src/display.c
        ${CMAKE_CURRENT_LIST_DIR}/src/touch.c
        ${CMAKE_CURRENT_LIST_DIR}/src/wrapper.c
        ${CMAKE_CURRENT_LIST_DIR}/src/module.c
)
message("IDF路径:${IDF_PATH}")
message("compoonents路径:${USER_COMPONENT_DIR}")
target_include_directories(usermod_lvgl_esp32 INTERFACE
        ${IDF_PATH}/components/esp_lcd/include/
        ${IDF_PATH}/components/esp_lcd/include/
        ${IDF_PATH}/components/esp_lcd/interface/
        ${CMAKE_CURRENT_LIST_DIR}/binding/lvgl
        ${CMAKE_CURRENT_LIST_DIR}/binding/lvgl/src
)
target_link_libraries(usermod_lvgl_esp32 INTERFACE lvgl_interface)
target_link_libraries(usermod INTERFACE usermod_lvgl_esp32)

