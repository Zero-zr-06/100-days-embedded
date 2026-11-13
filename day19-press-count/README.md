# Day 19 — BOOT 按键计数 + 配网支持带空格的 SSID（2026-11-13）

## 目标
在 Day 18「长按回首页」之上，加一个「BOOT 按过几次」的计数。
`src/main.cpp` 里标了 3 个 `TODO 空`，**这 3 行是自己填的**：
1. 短按一次 → `pressCount++`
2. 长按回首页时 → `pressCount = 0`
3. 系统页把 `pressCount` 显示出来

## 接线
沿用 Day 13，不用动：OLED GND→GND、VCC→3V3、SCL→GPIO9、SDA→GPIO8。
BOOT 是板载 GPIO0，无外接元件。

## 现象 / 验收标准（今天已验完）
- 按 RST，串口第一行 `Day 19: press count`（版本标记）
- 短按两下到 `3/3` 系统页，看到 `pressCount 2`；继续按，数字跟着涨
- 按住 BOOT 超过 1 秒：跳回 `1/3`，串口 `long press -> home` 只打印一次
- 再切回 `3/3`，`pressCount` 回到 `0`
- WiFi 连上后 `setup()` 几秒结束，按键按下立刻有反应，不用等 20 秒

## 学到什么
记住了输出时该用什么字符型，if语句千万不能乱嵌套，改wifi时要注意

## 踩坑（今天真实踩的）
1. **按 BOOT 像没反应**，其实不是按键代码错：`setup()` 里
   `while (WiFi.status() != WL_CONNECTED && tries < 40) { delay(500); }`
   最多阻塞 20 秒，这段时间 `loop()` 根本不跑，按键和串口命令全都没响应。
   geely 在的时候几秒连上感觉不到，今天 geely 消失就暴露了。
   排查手法：启动打印当版本标记 → 串口 `show` 有回显说明 `loop()` 活着 →
   `netsh wlan show networks` 确认那个 AP 到底存不存在。
2. **热点名自带空格**：真实 AP 是 `Redmi K70`。旧的配网解析用
   `indexOf(' ', s1+1)` 取第二个空格切分，`wifi Redmi K70 密码` 被存成
   ssid=`Redmi`、pass=`K70 密码` → 串口一直 `connecting to Redmi`、40 个点。
   改成 `lastIndexOf(' ')` 用最后一个空格切，SSID 就能带空格（密码放最后、不含空格）。
3. **SSID 必须逐字符完全一致**（含空格、大小写）。去掉空格敲 `RedmiK70`
   等于换了另一个网，照样连不上。这次只差一个空格：`RedmiK70` → 连不上，
   `Redmi K70` → 拿到 IP。
4. `wificlr` 老 bug 顺手修了：原来 `startsWith("wifi")` 把它吞掉、永远不生效，
   改成 `line == "wifi" || line.startsWith("wifi ")`，判断顺序要把精确匹配放前面。
5. `pressCount` 只在 **3/3 系统页**显示，在 1/3 页按是看不到数字的。
6. COM6 被多个残留监视器进程占着，敲命令的窗口未必是真正握着串口的那个。
   **监视器只留一个窗口**，烧录前先关掉，否则 `Could not open COM6, port is busy`。

## 待办
- Day 16 OTA 无线上传待补验：改 ini 走 espota
- `setup()` 里 20 秒阻塞 WiFi 的写法，后面应改成非阻塞（不卡住 loop）
- AHT20 温湿度传感器未到货

## 复现
```powershell
cd D:\100-days-embedded\day19-press-count
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```
串口配网（SSID 带空格直接照抄）：
```
wifi Redmi K70 <密码>
reboot
```
烧录前先关掉串口监视器。重烧不清 NVS，改完配置要 `reboot` 才生效。
