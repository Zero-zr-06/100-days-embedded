# Day 05 — 呼吸灯 + 按键共存（2026-10-30）

## 目标

把 Day 3 的呼吸灯和 Day 4 的按键合并进同一个 `loop()`：

- 灯持续呼吸，每一轮呼吸到最暗时自动换下一种颜色（红→绿→蓝→黄→青→紫）
- **短按 BOOT** → 立刻切到下一种呼吸色，从头开始渐变
- **长按 BOOT**（≥600ms）→ 呼吸暂停、灯灭；再长按 → 从当前颜色恢复
- 串口同步打印 `color -> N` / `paused` / `resumed`

## 学到什么

- **`delay()` 的致命缺陷**：呼吸如果用 `delay(15)` 慢慢加亮度，整个程序会卡住，按键完全失灵。
- **非阻塞的写法**：不等待，改成"看表"——`millis() - lastBreath >= 15` 才走一格，`loop()` 剩下的时间全部留给按键扫描。两个任务轮流插空执行，互不阻塞。
- 这是"环境检测仪"（采集 + 显示 + 上传并行）的核心前置技能。

## 复现

```powershell
cd D:\100-days-embedded\day05-breathing-button
pio run -t upload        # 烧录（COM6 已写死在 platformio.ini）
pio device monitor       # 115200，按 RST 看启动行
```

## 预期输出

```
ESP-ROM:esp32s3-20210327
rst:0x1 (POWERON),boot:0x8 (SPI_FAST_FLASH_BOOT)
...
Day 5: short press = next color, long press = pause/resume
color -> 1        ← 短按
paused            ← 长按一秒
resumed           ← 再长按
```

## 坑

- VS Code 里 PlatformIO 没认出新工程（状态栏不出 `esp32-s3-devkitc-1`）时，命令行 `pio run -t upload` 完全不受影响——**界面不认不影响编译烧录**。
- 复制工程文件夹时别嵌套：`platformio.ini` 必须在工程根目录，VS Code / pio 才认。
