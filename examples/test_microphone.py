import uasyncio as asyncio
from lvgl_esp32 import FFT

#异步任务框架
loop = asyncio.get_event_loop()

#读取麦克风示例
from machine import I2S
from machine import Pin
import ustruct
FFT_N=const(256)
FFT_R=const(8000)
FFT_T=const(1)
BYTELEN=ustruct.calcsize('h')
sck_pin = Pin(42)   # Serial clock output
ws_pin = Pin(45)    # Word clock output
sd_pin = Pin(46)    # Serial data output
audio_in = I2S(1,
               sck=sck_pin, ws=ws_pin, sd=sd_pin,
               mode=I2S.RX,
               bits=16,
               format=I2S.MONO,
               rate=FFT_R,
               ibuf=FFT_R*BYTELEN)
buf=bytearray(FFT_N*BYTELEN)
async def main():
    fft_plan=FFT(FFT_N,FFT.REAL,FFT.FORWARD)
    sreader = asyncio.StreamReader(audio_in)
    while True:
        num_read = await sreader.readinto(buf)
        if num_read>0:
            samples=ustruct.unpack('<'+'h'*(num_read//2),buf[:num_read])
            print(len(samples))
            #print(samples)
            fft_plan.execute(samples)
            await asyncio.sleep_ms(FFT_R//FFT_N)
    #释放内存
    del fft_plan


while True:
    asyncio.create_task(main())
    loop.run_forever()
