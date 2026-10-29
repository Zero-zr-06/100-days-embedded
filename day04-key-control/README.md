# Day 04 — BOOT 按键控制彩灯（2026-10-29）

## 目标

用板子上那颗 `BOOT` 键控制彩灯：

- **短按**（小于 600ms）→ 换下一种颜色（红 / 绿 / 蓝 / 黄 / 青 / 紫）
- **长按**（大于等于 600ms）→ 关灯
- 灯灭着的时候短按 → 重新点亮并换色
- 串口同步打印，双击确认现象

不需要外接任何元件。

## 硬件事实

- `BOOT` 键接在 **GPIO0**，另一头接地，所以**按下读到低电平（LOW）**
- 必须开内部上拉 `pinMode(0, INPUT_PULLUP)`，否则引脚悬空，读数乱跳
- ⚠️ GPIO0 是 strapping 引脚：**开机/复位时别按住 BOOT**，否则芯片会进下载模式，程序不跑

## 三个知识点（今天真正学的）

1. **`INPUT_PULLUP` + 低电平有效**——新手最容易把 `LOW`/`HIGH` 判断写反
2. **`millis()` 而不是 `delay()`**——`delay(600)` 会把整个程序卡死 600ms，期间按键全丢；用时间差判断，程序一直在转
3. **`static` 变量**——`loop()` 每轮都重新进入，`static` 才能"记住"上一次的按键状态和按下时刻

## 状态机

```
空闲 ──按下(LOW)──> 计时开始
     ├─ 600ms 内松手 ──> 短按：换色
     └─ 一直按住到 600ms ──> 长按：关灯（标记 handled，松手不再触发短按）
```

`handled` 这个标志是关键：它防止"长按松手时又被当成一次短按"。

## 结果

串口输出示例：

```
Day 4: press BOOT to change color, hold to turn off
short press -> color 1 (green)
short press -> color 2 (blue)
long press -> LED off
```

## 复现

```bash
platformio run -t upload
platformio device monitor     # 115200
```

烧完点一下 BOOT → 灯变色；按住一秒 → 灯灭。

## 明天

OLED 到货就上 **I2C**（SDA→GPIO4，SCL→GPIO5，VCC→3V3），先让 SSD1306 显示一行字。
