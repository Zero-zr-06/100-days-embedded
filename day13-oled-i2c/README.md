# Day 13 — I2C 扫描 + OLED 点亮（2026-11-07）

## 目标

第一次接外部器件：0.96 寸 OLED（SSD1306，I2C 接口）。
先扫总线确认设备在线，再用 Adafruit 库在屏幕上显示文字和芯片温度。

## 接线（这块屏的针脚顺序是反的！）

OLED 模块背面丝印从左到右是 **SDA SCL GND VCC**，和市面上常见的 `GND VCC SCL SDA` 完全相反，
必须照丝印接：

| OLED 丝印 | 接板子 |
|---|---|
| SDA | GPIO8（上排，丝印 `8`） |
| SCL | GPIO9（上排，丝印 `9`） |
| GND | GND（下排最左） |
| VCC | 3V3（上排最右） |

## 现象

- 串口：`Day 13: OLED (SSD1306)` → `OLED ready`
- 屏幕：黑底白字，第一行 `ESP32-S3 OK`，第二行运行秒数，第三行芯片温度，底部 `Day 13 - 100 days`

## 踩坑记录（今天踩了两个，都值得记）

1. **第一次接线全错，屏幕全黑。** 四根线当时分散插在不同排针上，黑色（GND）那根差点落到 5V。
   `oled.begin()` 仍然返回 true 但屏不亮 —— 因为它只判断"0x3C 有没有应答"，地址对就返回成功，
   跟线的极性无关。**先跑 I2C 扫描程序，扫到地址再谈显示。**
2. **误判成 SH1106。** 黑屏后换成 SH1106 驱动，串口照样打印 `OLED ready`（地址一样），屏照样黑。
   换回 SSD1306、把接线改对之后立刻就亮了。教训：**黑屏先查线，再查驱动**，别急着换库。

## 学到什么

1. I2C 只用两根线（SCL 时钟 + SDA 数据），设备靠 7 位地址区分，一条总线上可以挂很多器件。
2. 扫描器原理：`Wire.beginTransmission(addr)` + `endTransmission() == 0` 就是"这个地址有人应答吗"，
   不依赖任何驱动库，是最可靠的硬件自检手段。
3. OLED 不用自己画点阵，Adafruit_GFX 提供 `setCursor / print / printf`，手感和串口打印一样。
   画完必须 `display()` 才真正刷到屏上。
4. `SSD1306_SWITCHCAPVCC` 表示屏内部自升压供电，3.3V 就能点亮，不用外接高压。

## 复现

```powershell
cd D:\100-days-embedded\day13-oled-i2c
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```
