# Day 16 — OTA 无线烧录（2026-11-10，未验完，次日补）

## 目标
给 Day 15 的完整工程加 ArduinoOTA：板子连着 WiFi 就能更新固件，不用插 USB 烧录线。
第一版固件必须先用线烧进去（鸡生蛋），之后才能隔空上传。

## 今天做到哪一步

| 项目 | 状态 |
|---|---|
| `#include <ArduinoOTA.h>`、setup 里初始化、loop 里 `ArduinoOTA.handle()` | ✅ 已接入 |
| 崩溃循环（Backtrace → Rebooting → 再崩，一直刷屏） | ✅ 已修复，串口不再出现 Backtrace |
| 第一次 USB 烧录（COM6） | ✅ 成功，程序正常启动 |
| WiFi 连上、拿到 IP | ❌ geely 和 Redmi 热点都没连上 |
| `OTA ready` / 无线上传验证 | ❌ 未完成，明天补 |

**工程日志允许写「没验完」，不允许假装验完了。** 代码部分今天已经做完，卡住的纯粹是网络环境。

## 实际串口

```
Day 16: OTA enabled
threshold restored: 38.0 C
connecting to geely          （或 Redmi，取决于当时的配置）
........................................
WiFi FAILED - will keep retrying in loop
HTTP server started
OTA skipped (no WiFi)
E (43602) wifi:sta is connecting, return error
```

## 踩坑 / 学到什么

1. **`ArduinoOTA.begin()` 必须在 WiFi 连上之后。** 第一次把它贴在 setup 开头（`Serial.begin()` 后面），
   网络栈还没起来，底层直接 abort → Backtrace → 复位 → 再崩，无限刷屏。
   挪到 `server.begin()` 之后、并包一层 `if (WiFi.status() == WL_CONNECTED)`，崩溃立刻消失。
2. **mDNS 会抢实例。** ArduinoOTA 内部用 mDNS，原来 setup 里还有一句 `MDNS.begin("esp32-s3")`，
   两个不同 hostname 抢同一个实例。已把原 MDNS 那行注释掉（一直是用 IP 开网页，零损失）。
3. **`onProgress` 有除零风险。** `p / (t / 100)` 在 total < 100 时会除零，改成 `p * 100 / t`。
4. **`E (...) wifi:sta is connecting` 不是崩溃**，是 loop 里每 10 秒的 `WiFi.reconnect()`
   和 setup 的 `WiFi.begin()` 撞车。因为一直没连上，这段会空转刷警告，属已知噪音；
   以后可加 `everConnected` 门控（只在「曾经连上过、后来掉了」时才重连）。
5. **Windows 上 mDNS 不可靠，认 IP**；电脑和板子必须在同一个局域网，否则 espota 找不到设备。

## 当前工程的临时状态（重要，验证完要恢复）

`src/main.cpp` 第 219-222 行为了排查网络，**临时强制读 `wifi_secrets.h` 的 geely，
绕过了 Day 12 的 NVS 配网**：

```cpp
String ssid = WIFI_SSID_NAME;
String pass = WIFI_PASSWORD;
```

OTA 验证通过后要恢复成：

```cpp
String ssid = prefs.getString("ssid", WIFI_SSID_NAME);
String pass = prefs.getString("pass", WIFI_PASSWORD);
```

`platformio.ini` 现在保持 USB 烧录（`upload_port = COM6`），**不要提前改成 espota**，
否则明天连第一次烧录都做不了。

## 明天（Day 17）第一步

1. **在电脑上 `ping` 板子的 IP —— ping 不通，OTA 永远不可能成功**，先解决网络，其他都是白忙。
2. 确认电脑和板子连的是同一个 WiFi（电脑当前连哪个：`netsh wlan show interfaces`）。
3. 串口看到 `>>> open: http://x.x.x.x` 和 `OTA ready` 之后，再把 ini 改成：
   ```ini
   upload_protocol = espota
   upload_port = <板子的 IP>
   monitor_port = COM6
   ```
   （`upload_port = COM6` / `upload_speed` 那两行删掉或加 `;` 注释）
4. 改屏幕上一行字，`platformio.exe run -t upload` 无线推送；板子自动重启、字变了 = 验收完成。
5. 恢复上面那段 NVS 配网逻辑，README 补上验收结果。

USB 线别拔，板子靠它供电；想演示「真无线」就把线换到另一个 USB-C 口（OTG）供电。

## 复现（当前状态，走 USB）

```powershell
cd D:\100-days-embedded\day16-ota
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```
