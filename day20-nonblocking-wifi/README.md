# Day 20 — 不再死等 WiFi（非阻塞开机）（2026-11-14）

## 为什么做这个
昨天（Day 19）按 BOOT「没反应」折腾了很久，最后查出来根本不是按键代码错：
`setup()` 里有一句

```cpp
while (WiFi.status() != WL_CONNECTED && tries < 40) { delay(500); ... }
```

最多阻塞 **20 秒**。这段时间 `loop()` 一次都不跑，按键、串口命令、屏幕刷新全停。
geely 在时几秒连上感觉不到；那天 geely 消失，20 秒空等就暴露了。
今天把这个等待彻底拿掉，改用 `millis()` 的思路在 `loop()` 里查状态。

## 目标
- `setup()` 只做「开机一次性」的事，**不许有任何长时间等待**
- WiFi 连上之后，才初始化 OTA、打印网页 IP（Day 16 的顺序要求）
- 而且只做一次，不许每轮 `loop()` 都重复初始化

## 已经给你写好的部分（不用动）
- `startOta()` 函数：打印 IP、`ArduinoOTA` 全套初始化、打印 `OTA ready`
- `setup()`：已删掉 `while` 死等和里面那段 OTA，改成 `WiFi.begin()` 之后立刻结束
- Day 1~19 的所有功能原样保留（三处填空、配网、曲线、告警…一律没动）
- 版本标记改成 `Day 20: non-blocking WiFi`

## 今天自己写的部分：`src/main.cpp` 里 `Day 20 TODO 区` 的 3~5 行
在 `void loop() {` 正下方那段注释框里，自己写一个小 `if` 块：
- 条件：WiFi 已连上 **且** 还没调用过 `startOta()`
- 动作：调用 `startOta()`，并且让「只调用一次」这件事成立

提示（不给答案）：
1. 需要一个 `static bool` 记住状态。回想 Day 19 的 `pressCount`、Day 18 的
   `pressMs` —— 它们都是靠一个变量跨 `loop()` 记住「上次发生了什么」
2. 两个条件同时成立用 `&&`，别用逗号（Day 18 踩过 `if (a, b)` 的坑）
3. 进 `if` 之后**第一件事**别漏。漏了会怎样？填完故意删掉那一行烧一次，看串口
4. 现在这几行空着，编译照样能过，只是不会有 IP 和 `OTA ready`

编译报错时我只给提示，不贴答案。

## 现象 / 验收标准（按顺序看）
1. 命令行烧录后按 **RST**，串口第一行 `Day 20: non-blocking WiFi`
2. **立刻**（不用等）按 BOOT → 屏幕马上翻页、串口马上打印 `OLED page -> 2`；
   串口敲 `show` 也立刻有回显 ← 这条是今天的核心，昨天要等 20 秒
3. 几秒后 WiFi 连上，串口出现 `>>> open: http://10.x.x.x` 和 `OTA ready`，
   **只出现一次**，不会每轮刷屏
4. 故意把「置 true」那行注释掉再烧一次 → 看 `OTA ready` 刷屏；改回来 → 恢复。
   这一步是今天真正要弄懂的因果，别跳过
5. 顺便看 3/3 系统页，WiFi 状态和 IP 都正常

## 学到什么
知道调用startOta（）函数，ststic关键字的使用

## 踩坑预告
- `ArduinoOTA.begin()` 必须在 WiFi 连上之后（Day 16 因此崩溃循环过），
  所以移进 `loop()` 时，判断条件里一定要带 `WiFi.status() == WL_CONNECTED`
- `static` 变量在 `loop()` 里只初始化一次，跨轮保留值；写成普通局部变量就记不住
- 烧录一律命令行，别点 VS Code 平台栏（会烧成它当前打开的文件夹，已踩四次）
- 烧录前关掉串口监视器，否则 `Could not open COM6, port is busy`

## 今天真实踩的（11-14 晚，全是环境不是代码）
1. **两种「串口打不开」不是一回事**：
   `PermissionError(13, 拒绝访问)` = COM6 在、被别的程序占着（多半是 VS Code 图形监视器）；
   `FileNotFoundError(2, 系统找不到指定的文件)` = 端口此刻根本不存在（拔插 / 重枚举 /
   插到 OTG 口）。`Get-PnpDevice -Class Ports` 列的是注册过的设备，Status 全是 Unknown，
   **不能**用来判断能不能打开；要查当前可用端口用 `[System.IO.Ports.SerialPort]::GetPortNames()`。
2. **编译 ≠ 烧录**：`pio run` 只生成 `firmware.bin`，`pio run -t upload` 才把 bin 写进板子。
   版本标记一直是 `Day 19: press count`，就是因为只编译过、从没烧成功过。
3. **烧录被打断 = Flash 变空**：esptool 已经擦完 app 区，写入没做完 → 复位后
   `invalid segment length 0xffffffff` / `image at 0x650000 has invalid magic byte` /
   `No bootable app partitions in the partition table`。
   `boot:0x8 (SPI_FAST_FLASH_BOOT)` 说明芯片和串口都是好的，不是板子坏。
   **重烧一次即可**，代码一行都不用改，必须等到 `Hash of data verified` + `[SUCCESS]`。
4. **`upload` 不会自动开监视器**；烧录窗口刷的 `Writing at 0x...` 是烧录进度，不是固件日志。
   而且串口日志不会补发：烧录结束时 esptool 已经复位过一次，开机那几行早发完了 →
   必须**先开监视器、再按 RST**，否则永远看不到版本标记。
5. 监视器要在工程目录里跑（PlatformIO 以当前目录读 `platformio.ini` 里的 COM6 / 115200）；
   退出用 `Ctrl+C`，烧录前一定要退出，不然又 `port busy` 再打断一次。

## 待办
- 「删掉 `otaStarted = true` 看 `OTA ready` 刷屏」的因果实验还没亲手做（今天串口折腾太久，留到明天）
- Day 16 的 OTA **无线上传**还没补验（今天只是把初始化挪对了位置）
- AHT20 温湿度传感器未到货

## 复现
```powershell
cd D:\100-days-embedded\day20-nonblocking-wifi
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```
WiFi 用的还是 NVS 里存的 `Redmi K70`，不用重新配网（重烧不清 NVS）。
