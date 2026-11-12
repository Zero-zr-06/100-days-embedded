# Day 18 — BOOT 长按回首页（2026-11-12）

## 目标
在 Day 17「短按翻页」之上，加一个长按：按住 BOOT 超过 1 秒，
不管当前在第几页都跳回 1/3。**这 5 行判断是自己写的**，不是复制现成代码。

## 接线
沿用 Day 13，不用动：OLED GND→GND、VCC→3V3、SCL→GPIO9、SDA→GPIO8。
BOOT 是板载 GPIO0，无外接元件。

## 现象 / 验收标准
- 短按：`1/3 → 2/3 → 3/3 → 1/3`，串口 `OLED page -> 2`
- 按住超过 1 秒：屏幕跳回 `1/3`，串口 `long press -> home`，**只打印一次**
- 开机 2 秒不按键：串口安静，不会刷 `long press -> home`
- 按 RST 后串口第一行是 `Day 18: long press`（版本标记）

## 学到什么
学会if语句怎么写还有c++的一些固定语法，可以稍微看懂并简单写一些代码

## 踩坑（今天真实踩的，全在刚写的那 5 行里）
1. `oledpPage` 多打一个 p、`serial` 写成小写 → 编译报 `was not declared in this scope`。
   C++ 区分大小写，看到这句先查名字拼写，不用慌。
2. `if` 整行被 `//` 注释掉，末尾那个 `}` 却留着 → 报
   `expected declaration before '}' token`，`{` 和 `}` 必须成对。
3. **`if (a, b)` 里的逗号不是「并且」**：逗号运算符只取最后一个值，
   前面的 `millis() - pressMs >= 1000` 被整个丢掉，结果一按就回首页、根本不用等 1 秒。
   两个条件同时成立要用 `&&`。
4. **必须加 `pressMs != 0`**：`pressMs` 平时是 0，开机 1 秒后
   `millis() - 0 >= 1000` 恒成立 → 屏幕锁死在 1/3、串口每轮刷屏、BOOT 翻页全废。
5. 触发后把 `pressMs` 清零不能漏：按住期间 loop 每轮都会满足条件，会重复触发。
6. 短按正常、长按没反应，一度以为逻辑写错 —— **其实是板子里跑的还是 Day 17 旧固件**。
   把启动打印改成 `Day 18: long press` 当版本标记，按 RST 看第一行就一锤定音。
   烧录一律 `cd day18-long-press && platformio.exe run -t upload` 命令行，
   点 VS Code 平台栏会烧成它当前打开的文件夹（Day 5 / 11 / 17 已踩三次）。

## 待办
- `wificlr` 判断顺序 bug 未修（`startsWith("wifi")` 吞掉了 `wificlr`）
- Day 16 OTA 无线上传待补验：geely 已连通、`ping` 通，改 ini 走 espota 即可
- AHT20 温湿度传感器未到货

## 复现
```powershell
cd D:\100-days-embedded\day18-long-press
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```
烧录前先关掉串口监视器，否则报 `Could not open COM6, port is busy`。
