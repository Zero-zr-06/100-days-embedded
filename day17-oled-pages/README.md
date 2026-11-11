# Day 17 — BOOT 按键切换 OLED 页面（2026-11-11）

## 目标
0.96 寸屏只有 128×64，一屏塞不下温度、曲线、系统状态。今天用板载 BOOT 键（GPIO0）
把显示拆成三页，按一下翻一页。**所有现象都在屏幕上，不依赖 WiFi**——Day 16 卡在连不上网，
今天先把能离线验收的部分做完，把第 17 天守住。

## 接线
不用动，沿用 Day 13：OLED GND→GND，VCC→3V3，SCL→GPIO9，SDA→GPIO8。
BOOT 键是板载 GPIO0，没有外接元件。

## 现象 / 验收标准

| 页 | 屏幕内容 |
|---|---|
| 1/3 | `== TEMP ==` + 大字温度（`setTextSize(2)`）+ 阈值 + 告警状态 + 操作提示 |
| 2/3 | `== CHART / 2min ==` + 全屏温度折线（自动缩放） |
| 3/3 | `== SYSTEM ==` + WiFi 状态 / IP / RSSI / heap / uptime |

- 右上角 `1/3`、`2/3`、`3/3` 跟着变；每按一下串口回显 `OLED page -> 2`
- 按三下循环回第一页；曲线页每 2 秒多一个点（共用 Day 8 的 `hist` 数组），网页曲线照常同步
- 采样、呼吸灯、网页、串口命令同时活着，按键不阻塞任何一路

## 学到什么

1. **上拉输入**：`pinMode(0, INPUT_PULLUP)`，没按时引脚被内部电阻拉高（HIGH），按下接地变 LOW。
   所以判断条件是「变低」，不是「变高」。
2. **边沿检测**：`lastBtn == HIGH && btn == LOW` 只在「刚按下」那一瞬间成立一次。
   只写 `btn == LOW` 的话，按住不放会每轮 loop 都翻页，屏幕疯狂闪。
3. **200ms 软件消抖**：机械按键触点会抖几毫秒，用 `millis()` 记上次触发时间，200ms 内不重复响应。
   依旧不用 `delay()`，和 Day 5 的多任务思路一脉相承。
4. **状态机让显示和数据解耦**：`oledPage` 只决定「画什么」，采样、历史数组、阈值、告警全在
   `loop()` 里照常跑。切页不丢数据、不打断采样——一个数据源，多个视图（Day 14 的结论再用一次）。
5. 小屏产品的通用做法：信息分组 + 单键翻页，比把所有字挤成蚂蚁清楚得多。

## 踩坑

- **中文全角标点不能进代码**：把 `pinMode(0, INPUT_PULLUP);（BOOT 键）` 整行复制进 main.cpp，
  编译器报一串 `stray '\xxx' in program`——全角括号是非 ASCII 字符，C++ 词法直接不认。
  说明文字要么删掉、要么放到 `//` 后面。改完 `platformio.exe run` 29 秒编译通过。
  **注释里的中文没问题，代码区一个中文标点都不能有。**
- 顺手还了 Day 16 的债：setup 里那个「强制读 `wifi_secrets.h` 的 geely」的临时改动，
  已恢复成 `prefs.getString("ssid"/"pass", 默认值)`，Day 12 的 NVS 配网重新生效。
- **板子里烧的是 Day 4 固件（今天最耗时的一坑）**：按 RST 只蹦出
  `Day 4: press BOOT to change color...`，所以 WiFi、OLED 三页、串口命令全都不存在。
  根因是「VS Code 当前打开哪个文件夹，点上传就烧哪个工程」，和 Day 5 那次一样。
  用命令行 `cd day17-oled-pages && platformio.exe run -t upload` 才烧对。
- **COM6 被 3 个残留的 `device monitor --echo` 进程占着**，抓日志脚本报
  `PermissionError(13)`，监视器窗口里看到的也只是旧日志。关掉进程后立刻恢复。
  以后烧录或抓日志前，先确认没有残留监视器。
- **`wificlr` 命令永远失效（代码 bug，待修）**：`handleCmd()` 里
  `line.startsWith("wifi")` 排在 `line == "wificlr"` 前面，`wificlr` 被当成
  「不带参数的 wifi」吞掉，只回一句 usage，清 NVS 的分支根本到不了。
  把 `wificlr` 那条判断挪到 `startsWith("wifi")` 前面即可。
  今天的绕过办法：用 `wifi <SSID> <密码>` 直接覆盖 NVS 里的旧配置。
- 把 NVS 里那条连不上的 Redmi 覆盖回 geely 之后，`WiFi lost, reconnecting...` 警告消失；
  geely 实测 2.5 秒连上，串口打出 `OTA ready`。

## 待办

- **`wificlr` 的判断顺序 bug**：把 `else if (line == "wificlr")` 挪到
  `else if (line.startsWith("wifi"))` 前面，重烧一次即可。
- **Day 16 的 OTA 无线上传现在具备补验条件了**：geely 已连上
  （`http://10.2.138.79`、`OTA ready`），电脑也在 geely，
  `ping 10.2.138.79` 3 发 3 收、0% 丢包（约 88ms）。下一步把 `platformio.ini`
  改成 `upload_protocol = espota` / `upload_port = 10.2.138.79`，改屏幕一行字无线推送；
  验完把 ini 改回 `COM6`。
- AHT20 温湿度传感器还没到，到了挂同一条 I2C 总线（地址 0x38），把芯片内部温度换成真实室温。

## 复现

```powershell
cd D:\100-days-embedded\day17-oled-pages
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```

烧录前先关掉串口监视器，否则报 `Could not open COM6, port is busy`。
