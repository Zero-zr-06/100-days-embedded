# Day 01 — 点灯 + 串口打印（2026-10-27）

## 目标

把程序烧进 ESP32-S3，并且证明它真的在跑。

## 做了什么

1. 用 PlatformIO 新建工程，Board 选 `ESP32-S3-DevKitC-1`，Framework 选 Arduino
2. 配置 N16R8 模组：16MB Flash + OPI PSRAM + 写死 COM6
3. `✓ Build` → `→ Upload` → `🔌 Serial Monitor`，首次烧录成功
4. `main.cpp` 里加 `Serial.begin(115200)` 和每秒一次的 `Serial.printf`

## 学到的

- **烧录成功的标志**是终端里出现 `Hash of data verified.` 和 `Hard resetting via RTS pin...`，不是灯闪
- **板子只在上电/复位那一瞬间吐启动日志**，所以打开串口监视器后要按一下 RST 才看得到东西
- **`boot:0x8 (SPI_FAST_FLASH_BOOT)`** = 芯片从 Flash 启动成功，这是"程序在跑"的硬证据
- 串口监视器和烧录**不能同时开**，否则烧录报 `port is busy`
- 走 CH343 转接芯片时必须手动写 `upload_port`，PlatformIO 自动检测（默认找原生 USB 的 303A:1001）会失败

## 复现

```bash
platformio run                 # 编译
platformio run -t upload       # 烧录
platformio device monitor      # 串口监视，按 RST 看日志
```

预期串口输出：

```
hello from ESP32-S3
serial is alive, COM6 @ 115200
tick 0, uptime 1507 ms
tick 1, uptime 2507 ms
```
