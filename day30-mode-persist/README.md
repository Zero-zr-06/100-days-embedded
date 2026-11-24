# Day 30 — 灯模式掉电不丢（NVS 存 int）

## 目标

Day 29 能用 BOOT 短按切三种灯模式了，但**一断电就回到呼吸灯** —— 因为 `rgbMode` 只是内存里的变量。
今天把它存进 NVS（Flash），重启后灯还是上次那个模式。

这是 Day 23（`idleMs` 存 ULong）/ Day 24（`idleEnabled` 存 Bool）那套的第三次迁移：
同一个套路换个类型（这次是 **int**），换个存入时机（放在 `applyRgbMode()` 里，一条路盖住所有入口）。

## 今天只写 3 行填空

### 填空 A（1 行）— 存

在 `applyRgbMode()` 尾部 `// Day 30 填空 A` 处：

- 照 Day 23 的 `prefs.putULong("idle", idleMs);` 写
- 动作换成 `putInt`，key 用 `"rgbMode"`，值就是 `rgbMode`

为什么放在这个函数里：串口 `rgb on/off/grad` 和 BOOT 短按**都调用 `applyRgbMode()`**，
在这儿存一次，两条路全覆盖，不用每个入口各写一遍。

### 填空 B（2 行）— 读 + 生效

在 `setup()` 的 `// Day 30 填空 B` 处（在 `prefs.begin()` 之后）：

1. `rgbMode = prefs.getInt("rgbMode", 0);` 的形状 —— 动作 `get` + 类型 `Int`，默认值 `0`
2. 调 `applyRgbMode()` —— **光读到编号没用**，得让它翻译成三个开关，灯才会按恢复的模式亮

顺序不能反：先读编号，再翻译。

## 我替你搭好的前提（别改）

- 版本标记 `Day 30: RGB mode survives reboot`
- `Preferences prefs;` 从原来的位置**挪到了文件前面**（`applyRgbMode()` 今天要用它，
  C++ 要求变量先声明后使用，不挪会编译报 `not declared in this scope`）
- 原来的声明处留了注释占位，没有重复声明
- Day 29 的所有功能原样保留（BOOT 切换、串口命令、单一数据源 `rgbMode`）

## 编译

在 `D:\100-days-embedded\day30-mode-persist` 目录下：

```
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
```

卡超过两分钟先设环境变量：

```powershell
$env:PLATFORMIO_SETTING_CHECK_PLATFORMIO_INTERVAL="999999"
$env:PLATFORMIO_SETTING_ENABLE_TELEMETRY="No"
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
```

## 烧录 + 验收

先 `Ctrl+C` 退出串口监视器，再：

```
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
```

等 `Hash of data verified` + `[SUCCESS]`，**先开监视器、按 RST**，然后：

1. 版本标记 `Day 30: RGB mode survives reboot`
2. 短按 BOOT 两下 → 串口 `light mode: 2 (GRAD)`，灯按温度变色
3. 敲 `reboot`（或按 RST 硬重启）
4. 开机串口**直接打印 `light mode: 2 (GRAD)`**，灯立刻是渐变 —— **不用再按键，模式记住了** ← 今天的核心验收
5. 换成敲串口 `rgb on` → `light mode: 1 (STATUS)`，再 reboot → 开机还是 `light mode: 1 (STATUS)`（证明串口那条路也存进去了）

## 踩坑预告

- 类型要配套：存用 `putInt`、读用 `getInt`，别一个写 `putULong` 一个写 `getInt`
- `prefs` 必须先用 `prefs.begin()` 打开（setup 里已经有了，在你填空之前）；
  `applyRgbMode()` 在 setup 里第一次被调用时，`begin()` 早已执行过，放心
- 开机时 `getInt("rgbMode", 0)` 的 `0` 是「从没存过」的默认值 = 呼吸灯，符合首次开机预期
- 填空 A 别写进 `loop()` —— 那会每轮都写 Flash，既慢又伤寿命；放在状态**变化**时才调用的函数里才对

## 学到什么

复习函数就是动作加类型，学习存NVS

## 复现命令

```
cd D:\100-days-embedded\day30-mode-persist
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```

串口命令：`rgb` 查询 / `rgb on` 状态灯 / `rgb grad` 渐变 / `rgb off` 呼吸；或直接短按 BOOT 循环切换；`reboot` 验证模式保持。
