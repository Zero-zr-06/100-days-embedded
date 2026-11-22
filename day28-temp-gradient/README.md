# Day 28 — 温度渐变灯（连续变量 → 颜色配比）

## 目标

Day 27 的灯是**三档离散**（WiFi 断红 / 告警黄 / 正常绿），非黑即白。
今天让它**跟着温度连续变色**：凉 → 蓝，热 → 红，中间平滑过渡（紫/粉）。

这是 Day 26「比例换算 → 物理输出」思路的第二次练习：
上次把 `elapsed / idleMs` 的比例变成**屏幕进度条长度**，
这次把温度比例变成**红蓝两个颜色通道的亮度配比**。

## 今天只写 3 行填空

都在 `src/main.cpp` 的 `showTempGradient()` 函数里（Day 28 区）。

已经给你的：
- `t` = 当前温度（`temperatureRead()`）
- `ratio` = 0~1 的比例（35°C → 0，45°C → 1，越界已用 if 钳制好）

你要填的 3 行：
1. 红通道 `r`：`ratio` 越大越亮 → `r = ratio * MAX_LEVEL`
2. 蓝通道 `b`：`ratio` 越大越暗 → `b = (1 - ratio) * MAX_LEVEL`
3. 画出来：`neopixelWrite(RGB_PIN, r, 0, b)`（绿通道给 0）

⚠️ 关键坑（Day 26 踩过）：`ratio * MAX_LEVEL` 算出来是**小数**，
但 `neopixelWrite` 要的是**整数**。记得转成 `int`（`(int)(...)`）。

## 我替你搭好的前提（别改）

- 版本标记 `Day 28: temp gradient light`
- 全局 `bool rgbGradMode = false`（渐变模式开关）
- 串口命令 `rgb grad`（进渐变）、`rgb on`（回昨天的状态灯）、`rgb`（查当前模式和温度）
- loop 里灯模式优先级：渐变 > 状态灯 > 告警快闪 > 呼吸
- 两种模式互斥：进一个就自动关掉另一个

## 编译

在 `D:\100-days-embedded\day28-temp-gradient` 目录下：

```
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
```

编译卡超过两分钟（PlatformIO 联网检查）就先设这两个环境变量再编：

```powershell
$env:PLATFORMIO_SETTING_CHECK_PLATFORMIO_INTERVAL="999999"
$env:PLATFORMIO_SETTING_ENABLE_TELEMETRY="No"
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
```

## 烧录 + 验收

先 `Ctrl+C` 退出串口监视器，再烧录：

```
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
```

等 `Hash of data verified` + `[SUCCESS]`，**先开监视器、按 RST**，然后按顺序看：

1. 版本标记 `Day 28: temp gradient light`
2. 敲 `rgb` 查询 → 显示 `rgb mode: BREATH | temp xx.x C`，记住这个温度数
3. 敲 `rgb grad` → 回显 `rgb grad: ON (xx.x C)`，灯按当前温度变色
4. **用手指捏住 ESP32 芯片（黑色方块）几秒** → 温度上升，灯**慢慢从蓝变红/紫**
5. 松手 → 温度回落，灯慢慢变回蓝 ← 今天的核心：连续变量 → 连续颜色
6. 敲 `rgb on` → 回昨天的三档状态灯（绿常亮）
7. 敲 `rgb off` → 回呼吸灯

> 温度读数说明：ESP32 内部温度传感器读数偏高且不精确（基线可能 30~40°C 随板子不同）。
> 如果敲 `rgb grad` 后灯已经是红的（温度 >45），说明你这块的基线偏高，
> 把 `showTempGradient()` 里的 `35.0` 和 `45.0` 两个数字往上挪（比如 40 和 50）再重烧，
> 让常温落在蓝色端、手捏才变红 —— 这样现象最明显。这正是「改一个数字看现象变」。

## 踩坑预告

- 温度是**连续**量，`if/else if/else` 三档切不出渐变；要用**比例**算颜色通道
- 红通道和蓝通道是**此消彼长**（一个随 ratio 增，一个随 ratio 减），这就是"混色"
- `ratio` 越界必须钳制到 0~1，否则温度过高/过低时颜色值会溢出
- 小数 → 整数要强转，`neopixelWrite` 第一个参数永远是 `RGB_PIN`（Day 27 错 4 版的地方）

## 学到什么
一个连续的数字（温度）用比例算出两个颜色通道，就能从蓝平滑变红


## 复现命令

```
cd D:\100-days-embedded\day28-temp-gradient
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```

串口命令：`rgb` 查询 / `rgb grad` 渐变 / `rgb on` 状态灯 / `rgb off` 呼吸
