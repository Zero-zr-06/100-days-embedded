# Day 29 — BOOT 键切换灯的三种模式

## 目标

前两天灯模式只能靠串口敲命令（`rgb on` / `rgb grad`）切换，得连着电脑。
今天让 **BOOT 短按** 就能循环切换三种模式，脱离电脑也能玩：

```
0 呼吸灯 → 1 状态灯(红/黄/绿) → 2 温度渐变(蓝→红) → 0 …
```

核心练习：**取模 `%` 做循环** —— 跟你 Day 17 写的 `oledPage = (oledPage + 1) % 3` 一模一样的招，
只是这次作用在灯模式编号上。

## 今天只写 2 行填空

在 `src/main.cpp` 的 `loop()` 里，短按块（`// Day 29 填空` 标注处）：

1. `rgbMode = (rgbMode + 1) % 3;` —— 照上面 `oledPage` 那行抄，编号在 0/1/2 之间循环
2. `applyRgbMode();` —— 把编号翻译成三个开关（函数我写好了，别自己改 bool）

就这两行。短按一次 = OLED 翻一页 + 灯换一种模式，两个动作互不干扰。

## 我替你搭好的前提（别改）

- 版本标记 `Day 29: BOOT switches light mode`
- 全局 `int rgbMode = 0`（模式编号，0/1/2）
- `applyRgbMode()` 函数：编号 → 三个 bool，并串口打印当前模式
- 串口命令 `rgb on/off/grad/rgb` 全部改成走 `rgbMode` 同一个变量 —— 这样串口和按键
  读写的是同一个状态，不会出现「串口说状态灯、按键以为在呼吸」的错乱
- 单一数据源：`rgbMode` 是唯一真相，三个 bool 由它派生

## 编译

在 `D:\100-days-embedded\day29-boot-mode` 目录下：

```
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
```

卡超过两分钟（PlatformIO 联网检查）就先设环境变量再编：

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

等 `Hash of data verified` + `[SUCCESS]`，**先开监视器、按 RST**，然后：

1. 版本标记 `Day 29: BOOT switches light mode`
2. 开机默认 `light mode: 0 (BREATH)`，灯在呼吸
3. **短按一下 BOOT** → 串口 `light mode: 1 (STATUS)`，灯变绿常亮（状态灯）
4. **再短按一下** → `light mode: 2 (GRAD)`，灯按温度变色
5. **再短按一下** → `light mode: 0 (BREATH)`，回到呼吸 ← 取模循环闭环
6. 敲 `rgb` 查询 → 显示的模式编号和灯当前状态一致（单一数据源生效）
7. 长按 BOOT 1 秒 → 还是回首页（Day 18 的功能没被破坏）

> 短按同时会翻 OLED 页，这是正常的 —— 今天灯和屏幕各切各的，互不干扰。

## 踩坑预告

- 取模 `% 3` 让编号到 2 之后自动回 0，不用写 `if (rgbMode > 2) rgbMode = 0` 这种啰嗦判断
- 别绕开 `applyRgbMode()` 直接改 `rgbGradMode` 那些 bool —— 那样 `rgbMode` 和 bool 会不同步，
  下次按键从错误的编号继续循环
- 这也是 Day 24 `idleEnabled` 那套「一个变量管状态」的思路，今天多了个「循环」

## 学到什么

学会利用%取模来循环

## 复现命令

```
cd D:\100-days-embedded\day29-boot-mode
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```

串口命令：`rgb` 查询 / `rgb on` 状态灯 / `rgb grad` 渐变 / `rgb off` 呼吸；或直接短按 BOOT 循环切换
