import lvgl_esp32

# Adapt these values for your own configuration
spi = lvgl_esp32.QSPI(
    2,
    baudrate=80_000_000,
    sck=9,
    data0=11,
    data1=12,
    data2=13,
    data3=14,
)
spi.init()

display = lvgl_esp32.Display(
    spi=spi,
    width=360,
    height=360,
    swap_xy=False,
    mirror_x=False,
    mirror_y=False,
    invert=True,
    bgr=True,
    reset=47,
    dc=-1,
    cs=10,
    pixel_clock=50_000_000,
)
display.init()
