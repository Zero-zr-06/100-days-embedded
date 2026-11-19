# Day 25：把空闲设置显示在 OLED 系统页

## 今天做什么

前四天调的空闲设置（`idleMs` 时间、`idleEnabled` 开关）只能在串口敲 `idle` 才看得到。
今天把它们搬到 OLED 系统页（3/3），**不看电脑，光看屏幕就知道当前设置**。

这是 `millis`/NVS 主线的收尾：设置能被"看"见，才算真正可用。

## 已写好的部分

- 版本标记：`Day 25: settings on screen`
- 系统页原来那行 `pressCount`（Day19 的）已注释掉，腾出两行位置
- 全局变量 `idleMs`（unsigned long）、`idleEnabled`（bool）都在文件顶部
- 串口命令 `idle` / `idle 1000` / `idle off` / `idle on` 都还是好的

## 今天写两处填空（共 2 行，都在 `refreshOled()` 的系统页分支）

- **填空 A**：`oled.printf(...)` 打印 `idleMs`，格式符 `%lu`（unsigned long）
- **填空 B**：`oled.printf(...)` 打印开关，`idleEnabled ? "ON" : "OFF"` 配 `%s`

两处都照着串口里现成的 `Serial.printf(...)` 那行抄，只是把 `Serial` 换成 `oled`。
提示在代码注释框里，不给答案。

## 编译

在 `D:\100-days-embedded\day25-settings-screen` 目录下：

```
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
```

如果编译卡超过两分钟（PlatformIO 联网检查，跟 GitHub 网络有关），改用这条跳过联网：

```
$env:PLATFORMIO_SETTING_CHECK_PLATFORMIO_INTERVAL="999999"
$env:PLATFORMIO_SETTING_ENABLE_TELEMETRY="No"
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
```

## 烧录 + 验收

先 `Ctrl+C` 退出串口监视器，再烧录：

```
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
```

等 `Hash of data verified` + `[SUCCESS]`，再**先开监视器、按 RST**：

1. 版本标记 `Day 25: settings on screen`
2. 短按两下 BOOT 到 3/3 系统页 → 屏幕多出两行 idle 时间和开关
3. 敲 `idle 1000` 回车 → **屏幕那行时间立刻变成 1000**（不用重启、不用重烧）
4. 敲 `idle off` 回车 → **屏幕开关那行变成 OFF**
5. 翻到 3/3 放手 6 秒 → 开关 OFF 时不回首页（Day24 的功能没被破坏）
6. 敲 `idle on` + `idle 5000` 恢复默认值（NVS 掉电不丢，别污染明天）

第 3、4 条是今天的核心：**串口改值 → 屏幕跟着变**，证明屏幕读的是同一个变量，不是写死的数字。

## 学到什么

<!-- 这里自己写一句：今天真正弄明白的一件事 -->

## 踩坑预告

- OLED 一屏只有 8 行（128×64，字号 1 每行 8px），加行要先删行，不然最后一行被裁掉
- `oled.printf` 格式符跟 `Serial.printf` 一样：`%lu` 配 unsigned long，`%s` 配字符串
- bool 不能直接打印成 ON/OFF，要用三元表达式 `flag ? "ON" : "OFF"` 变成字符串

## 复现命令

```
cd D:\100-days-embedded\day25-settings-screen
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```
