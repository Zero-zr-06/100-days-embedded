# Day 08 — 网页温度曲线图（2026-11-02）

## 目标

在 Day 7 的数据面板上加一条**会滚动的温度曲线**：板子每 2 秒采一次芯片温度，
存最近 60 个点（约 2 分钟），网页用 Canvas 画成折线，每 2 秒自动刷新。

## 现象

- 串口：第一行 `Day 8: temperature chart`，之后每 2 秒打一行 `temp = 53.2`
- 网页：上半部分是颜色按钮，下半部分是红色折线
- **用手按住芯片的金属屏蔽罩 10 秒，曲线会明显往上爬** —— 这是最直观的验证

## 学到什么

1. **环形数组**：60 个格子写满后回到开头覆盖旧数据，`histPos = (histPos + 1) % HIST_N` 一行搞定。
   内存只有几百 KB 的设备不能无限追加，都是这么存历史的。
2. **采样也不能用 `delay()`**：还是 Day 5 那套 `millis()`，三件事（接待网页 / 采温度 / 呼吸灯）
   在同一个 `loop()` 里轮流插空跑，谁也不挡谁。
3. **Canvas 画图**：`moveTo / lineTo` 把数组映射成坐标。以后把 `temperatureRead()`
   换成 `aht20.readTemperature()`，这段代码就是期末项目的成品。

## 复现

```powershell
cd D:\100-days-embedded\day08-temp-chart
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor
