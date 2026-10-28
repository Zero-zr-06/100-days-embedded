# Day 02 — WS2812 呼吸灯（2026-10-28）

## 目标

点亮板载 RGB 彩灯，并让它做「呼吸 + 变色」。不需要外接任何元件。

## 硬件事实

- 板上那颗方形小黑点在 GPIO48，型号 **WS2812**（智能 RGB LED）
- 它不吃高低电平，要的是 **800kHz 单线归零码**时序 —— 所以昨天 `digitalWrite` 死活点不亮
- 驱动它用 **`neopixelWrite(pin, r, g, b)`**，Arduino-ESP32 内核自带，**不用装任何库**（别去装 Adafruit_NeoPixel，那条路要走 PlatformIO 国外源，很慢）

## 做了什么

- `setWheelColor(pos, brightness)`：把 0-255 的位置换算成一种彩虹色，再按 brightness 缩放
- `loop()` 里让 brightness 从 0 涨到 40 再落回 0，就是呼吸；每呼吸一轮把颜色位置 `+= 24` 换色
- `MAX_LEVEL = 40`：这颗灯直接给 255 能晃瞎眼，压到 40 足够亮

## 结果

```
RAM:   6.9%   (22576 / 327680 bytes)
Flash: 4.9%   (323148 / 6553600 bytes)
[SUCCESS] Build   25.77s
[SUCCESS] Upload   9.68s    Hash of data verified.
```

串口确认：

```
Day02: WS2812 breathing light on GPIO48
breath done, color=24
```

## 复现

```bash
platformio run -t upload
platformio device monitor     # 波特率 115200
```

按 RST 会看到上面两行；板载彩灯应该在做缓慢变色呼吸。

## 明天

OLED 到货后走 **I2C**（SDA→GPIO4，SCL→GPIO5，VCC→3V3），先把 SSD1306 点亮。
