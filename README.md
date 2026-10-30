# 100-days-embedded

二本大三，基础一般，自律偏弱。每天 30 分钟，连续 100 天，把一块 ESP32-S3 玩明白。

规则：每天必须有一条提交。哪怕只改一行字、拍一张照片，也算完成。

## 硬件

- ESP32-S3-N16R8 开发板（16MB Flash + 8MB OPI PSRAM，双 USB-C，带摄像头座）
- 串口：CH343 转接口，COM6，115200
- 按键：左边 RST（复位），右边 BOOT（进下载模式）
- 注意：GPIO48 那颗是 WS2812 彩灯，必须用 `neopixelWrite(48, r, g, b)` 驱动，`digitalWrite` 点不亮它

## 环境

VS Code + PlatformIO（board: `esp32-s3-devkitc-1`，flash 16MB，`memory_type = qio_opi`）

## 进度

| 天数 | 日期 | 今天做了什么 | 现象 |
|---|---|---|---|
| Day 0 | 2026-10-24 | 买板子，建仓库 | — |
| Day 1 | 2026-10-25 | 装好 VS Code + PlatformIO，搭出第一个点灯工程 | 待烧录 |
| Day 2 | 2026-10-27 | 首次烧录成功，串口打印跑通 | `boot:0x8`，烧录成功 + tick 刷屏 |
| Day 3 | 2026-10-28 | WS2812 RGB 呼吸灯点亮（`neopixelWrite` GPIO48） | 彩灯变色呼吸 |
| Day 4 | 2026-10-29 | BOOT 按键控制彩灯：短按换色、长按关灯 | 串口打印 short/long press，颜色跟着变 |
| Day 5 | 2026-10-30 | 呼吸灯 + 按键共存（`millis()` 非阻塞替代 `delay()`） | 呼吸自动换色，短按切色、长按暂停 |

> 目录里的 `01-GPIO点灯/` 是 Day 1 最早搭的骨架，代码没改动，留着当备份；真正烧过的版本在 `day01-blink-serial/`。
> 每个 dayXX 目录里都有单独的 README，写当天做了什么、怎么复现。

## 目标清单

- [x] 环境搭好，点亮第一个 LED
- [x] 串口打印
- [x] 按键控制
- [ ] I2C 读温湿度传感器
- [ ] OLED 屏显示数据
- [ ] WiFi 连上，数据发到手机

## 路线

点灯 → 串口 → I2C → OLED → 期末做出「桌面环境检测仪」→ 暑假 FreeRTOS + 物联网数据采集终端

