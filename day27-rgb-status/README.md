# Day 27：RGB 灯显示系统状态

## 今天做什么

Day 26 把 `elapsed` 数字变成了屏幕上的长度。今天换个输出设备——**把系统状态变成灯的颜色**：

- WiFi 没连上 → **红灯**
- 温度告警 → **黄灯**
- 一切正常 → **绿灯**

敲 `rgb on` 进入状态灯模式（灯不呼吸了，改按状态变色），`rgb off` 回到原来的呼吸灯。
这是「变量→物理现象」第二次练：昨天输出到屏幕，今天输出到灯。

## 已写好的部分

- 版本标记：`Day 27: RGB status light`
- 全局 `bool rgbStatusMode`（false=呼吸灯，true=状态灯）
- `showRgbStatus()` 函数骨架，注释里写清了三种状态的颜色
- 串口命令 `rgb on` / `rgb off` / `rgb`（查询）
- loop 里已经接好：状态灯模式优先，关掉时走原来的告警/呼吸逻辑

## 今天写一处填空（3~4 行，在 `showRgbStatus()` 里）

用 `if / else if / else` 判断三种情况，每种调一次 `neopixelWrite(RGB_PIN, r, g, b)`：

1. WiFi **没连上** → 红 `(MAX_LEVEL, 0, 0)`
2. 温度告警 `alert` → 黄 `(MAX_LEVEL, MAX_LEVEL, 0)`
3. 正常 → 绿 `(0, MAX_LEVEL, 0)`

提示在代码注释框里，不给答案。注意「没连上」是 `WiFi.status() != WL_CONNECTED`。

## 编译

在 `D:\100-days-embedded\day27-rgb-status` 目录下：

```
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
```

卡超过两分钟用跳过联网的两条环境变量（见 day25 README）。

## 烧录 + 验收

先退出监视器再烧：

```
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
```

等 `Hash of data verified` + `[SUCCESS]`，再先开监视器、按 RST：

1. 版本标记 `Day 27: RGB status light`
2. 默认是呼吸灯（灯在渐亮渐暗、换色）——原功能没被破坏
3. 敲 `rgb` 查询 → `rgb mode: BREATH | wifi ON | alert NO`
4. 敲 `rgb on` → 灯变成**绿色常亮**（WiFi 连着 + 没告警 = 正常）
5. 敲 `rgb off` → 回到呼吸灯
6. 制造告警：敲 `th 30`（阈值调太低，手捂芯片或等温度超过 30）→ 再敲 `rgb on` → 灯**黄色**
7. 断 WiFi：把热点关掉或改错 SSID 重启 → `rgb on` → 灯**红色**（优先级最高）

第 4 条是今天的核心：**一个 `if` 判断 + 一次 `neopixelWrite`，变量就变成看得见的颜色**。

## 学到什么
学会 if / else if / else 判断三种情况


## 踩坑预告

- RGB 是三个数 `(r, g, b)`，哪个位置给 `MAX_LEVEL` 就是哪个颜色；
  黄=红+绿都给满，青=绿+蓝都给满——颜色是「加出来」的
- 判断顺序（优先级）：WiFi 没连要排最前，因为断网时更该先看到红色
- `rgbStatusMode` 是个开关：它为真时灯归状态逻辑管，为假时归原来的呼吸逻辑管——
  跟 Day24 `idleEnabled` 一个套路，一个 bool 决定哪段代码执行
- 别用 `digitalWrite` 点 RGB：GPIO48 是 WS2812，必须 `neopixelWrite`（Day2 踩过）

## 复现命令

```
cd D:\100-days-embedded\day27-rgb-status
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```
