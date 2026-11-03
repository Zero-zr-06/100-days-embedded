# Day 09 — 阈值告警 + 掉线自动重连（2026-11-03）

## 目标

在 Day 8 的温度曲线上加一层判断：温度超过阈值就报警（彩灯红闪 + 网页红字），
阈值能在网页上改；同时给 WiFi 加一个掉线自动重连的保险。

## 现象

- 串口：`Day 9: threshold alert + auto reconnect`，之后每 2 秒一行
  `temp 37.2 C | th 38.0 | OK`
- 超过阈值的那一秒会**多打一行** `ALERT ON : 38.4 >= 38.0`，之后不再重复刷
- 彩灯不再呼吸，变成**红灯快闪**（每 300ms 翻转一次）
- 网页顶部横幅变红 `⚠ ALERT! Temp too high`，松手降温后自动变回绿色 `OK`
- 网页 `Threshold` 输入框填 38 点 Set，立刻生效

## 学到什么

1. **阈值要按实测基线定，不能拍脑袋。**&#8203; 芯片温度传感器在屏蔽罩下面，读数 = 环境温度 + 自发热，
   平时基线 34~36 ℃，捂住只能抬 3~4 度。第一天我设 43 ℃，怎么捂都不报警；
   改成 38 ℃ 立刻有现象。以后接 AHT20 测空气温度才是真环境温度。
2. **滞回（hysteresis）防抖。**&#8203; 只写 `t > threshold` 的话，温度卡在阈值上下 0.1 度时会一秒开一秒关，
   屏幕疯狂闪。加 `const float HYST = 1.0;` 后：≥阈值 才开，≤阈值-1 才关，中间是死区。
   这是所有告警系统的标配（空调、电池保护板都用）。
3. **状态机只在“变化的那一瞬间”打印。**&#8203; `if (!alert && t >= threshold)` 这个条件保证
   `ALERT ON` 只打一次，而不是每 2 秒刷一遍。日志要干净，才能一眼看出事件。
4. **网页改的参数只活在内存里。**&#8203; `/set?th=38` 只是给变量赋值，重启回默认值。
   想掉电保存要写 NVS（`Preferences` 库），那是后面的事。
5. **`WiFi.reconnect()` 是最便宜的高可用。**&#8203; 每 10 秒查一次 `WiFi.status()`，掉线就重连，
   不需要断开重连整个 `begin()`。还是用 `millis()`，不阻塞呼吸灯和网页。

## 复现

```powershell
cd D:\100-days-embedded\day09-alert
copy include\wifi_secrets.example.h include\wifi_secrets.h
# 编辑 include\wifi_secrets.h，填你自己的 SSID / 密码（这个文件已被 .gitignore 忽略，不会传上去）
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor
```

浏览器打开串口打印的 `http://192.168.x.x`（或 `http://esp32-s3.local`），
手掌捂住金属屏蔽罩 10 秒即可触发。

## 关键代码位置

| 功能 | 文件 | 行 |
|---|---|---|
| 默认阈值 / 滞回值 | `src/main.cpp` | 36-38 |
| 告警状态机 | `src/main.cpp` | 197-205 |
| 红灯快闪 | `src/main.cpp` | 208-210 |
| 网页改阈值 `/set` | `src/main.cpp` | 148-157 |
| WiFi 自动重连 | `src/main.cpp` | 179-186 |
