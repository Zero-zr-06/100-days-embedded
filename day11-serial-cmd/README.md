# Day 11 — 串口命令行（2026-11-05）

## 目标
不开网页，在串口监视器里敲命令控制板子：`th <num>` 改阈值（存 NVS）、`show` 看状态、`reboot` 重启。

## 现象
- 敲 `show` → `temp 35.2 C | th 38.0 | OK | heap ...`
- 敲 `th 36` → `threshold -> 36.0 (saved)`
- 敲 `reboot` → 重启后 `threshold restored: 36.0`（Day 10 的 NVS 还在）

## 学到什么
1. `while (Serial.available())` 非阻塞读串口，一次读完缓冲区，和 millis() 轮询同一个思路。
2. 按行拼包：字符一个个进 `cmdBuf`，碰到 `\n` 才处理，`\r` 扔掉（回车是 \r\n 两个字符）。
3. `String::startsWith / substring / toFloat` 拆命令，嵌入式 shell 的最小实现。
4. 命令改的参数走同一套 `prefs.putFloat`——入口可以有很多个，存储只有一份。
