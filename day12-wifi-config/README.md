# Day 12 — 串口配网（2026-11-06）

## 目标
WiFi 账号密码不再写死在代码里：串口敲 `wifi <ssid> <pass>` 配网，存 NVS，
重启生效。`wificlr` 清除后回到 wifi_secrets.h 的默认网。换网络不用重新编译烧录。

## 现象
- `wifi` → `wifi now: geely (RSSI -61 dBm)`
- `wifi myhot abc12345` → `wifi saved: myhot, type reboot to apply`
- `reboot` → `connecting to myhot` → 新 IP，手机连同一热点能打开面板

## 学到什么
1. `prefs.putString / getString("ssid", 默认值)`，和昨天存阈值同一套 API。
2. 编译进固件的 wifi_secrets.h 变成"出厂默认"，NVS 是"用户配置"——出厂默认+用户覆盖，
   是所有路由器、智能音箱的标准做法。
3. 命令解析进阶：indexOf 找空格切 token，一条命令带两个参数。
4. 密码最少 8 位是 WiFi 标准要求，顺手做了参数校验。
