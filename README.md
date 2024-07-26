# LVGL on ESP32 with MicroPython (as a USER_C_MODULE)

![硬件链接](https://gw.alicdn.com/imgextra/i2/2212878633507/O1CN01Kesn2E1bmFyNplRBT_!!2212878633507.png_.webp)
## 支持硬件
- ESP32 S3 R16N8
- QSI ST77916屏幕
- CST816S 触摸驱动

## 安装编译环境
需要用**VirtualBox**下载安装[ubuntu-22.04.4-desktop-amd64.iso](https://releases.ubuntu.com/22.04/ubuntu-22.04.4-desktop-amd64.iso)
## 软件版本

- LVGL: 9.1
- MicroPython: 1.23.0
- ESP-IDF: 5.2.2

## 编译安装
```shell
git clone -b v5.2.2 --recursive https://github.com/espressif/esp-idf.git

cd esp-idf
./install.sh       
source export.sh

git clone --recurse-submodules https://github.com/apexracing/micropython.git

cd mpy-cross
make

cd ports/esp32
make submodules
idf.py set-target esp32s3

git clone --recurse-submodules https://github.com/apexracing/lvgl_esp32_mpy.git

make BOARD=ESP32_GENERIC_S3 BOARD_VARIANT=SPIRAM_OCT USER_C_MODULES=<DIR>/lvgl_esp32_mpy/micropython.cmake

```

## 固件下载
>- 目录[dist](./dist)下已经存放编译好的固件,16位，限制30FPS，支持png,SD卡,用[flash_download_tool_3.9.5](https://www.espressif.com/sites/default/files/tools/flash_download_tool_3.9.5.zip)烧录固件
>- 使用Thonny上传[example](./examples)文件运行示例代码。
