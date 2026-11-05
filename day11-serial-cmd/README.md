# Day 10 — 阈值掉电保存（NVS / Preferences）（2026-11-04）

## 目标
Day 9 网页改的阈值只活在内存里，一复位就丢。今天用 ESP32 自带的 NVS
（一块 Flash 里的存储区）把它存下来：改一次，永久生效。

## 现象
- 串口：`Day 10: ...` 之后多一行 `threshold restored: 40.0 C`
- 网页把 Threshold 改成 40 → 按板上 RST 复位 → 阈值还是 40（昨天会变回 43）

## 学到什么
1. `Preferences` 库三件套：`begin("cfg")` 开区、`putFloat("th", v)` 写、`getFloat("th", 默认值)` 读。
2. key 不存在时 `getFloat` 返回第二个参数当默认值——这就是"第一次开机"的处理，不用写 if。
3. Flash 写入有寿命（约 10 万次），所以只在用户点 Set 时写，不能每 2 秒存一次温度。
4. WiFi 密码这类配置以后也能这么存，换环境不用重新编译烧录。

## 复现
cd D:\100-days-embedded\day10-save-config 后照 day09 的 README 跑（烧录命令一样）。
