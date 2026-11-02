
---

## `day07-dashboard\README.md` 的内容（复制这段）

```markdown
# Day 07 — 网页实时数据面板（2026-11-01）

## 目标

把 Day 6 的"手机控制灯"反过来：**板子把数据推到手机上看**。
网页每 2 秒自动刷新，显示运行时间、剩余内存、WiFi 信号强度、芯片温度。

## 现象

- 串口：`Day 7: WiFi data dashboard` + `>>> open: http://x.x.x.x`
- 网页上方是颜色按钮，下方 **Live Data** 区四行数字
- **`Uptime` 那一行会自己往上跳** —— 看到它动就是成功

## 学到什么

1. **前后端分离**：`/api` 只吐 JSON 数据不给页面，`/` 只给页面。
   以后做手机 App、小程序，都是调这个 `/api` 拿数据。
2. **轮询**：`setInterval(upd, 2000)` + `fetch('/api')`，客户端定时问服务器要最新数据。
3. **mDNS**：`http://esp32-s3.local` 不用记 IP。缺点是安卓支持不稳定，所以 IP 和域名两个都留。

## 复现

```powershell
cd D:\100-days-embedded\day07-dashboard
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor
