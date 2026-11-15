# Day 21 — 空闲自动回首页（idle timeout）（2026-11-15）

## 今天练什么
`millis()` 第三次出场了：Day 18 用它判断「按住超过 1 秒」，Day 20 用它拿掉
`setup()` 的 20 秒死等，今天用它判断「有多久没人按 BOOT」。
另一个重点是 **别急着新建变量** —— `loop()` 里已经有一个变量在记这件事，先把它找出来。

## 目标
连续 `IDLE_MS` 毫秒（默认 5000）没按过 BOOT，OLED 就自动回到 `1/3` 页，
串口打印一句 `idle -> home`。中途按一下按键，计时重新开始。

## 已经给你写好的部分（不用动）
- 文件顶部的 `const unsigned long IDLE_MS = 5000;`（改这个数字就能看到现象变化）
- Day 1~20 所有功能原样保留：三页 OLED、短按/长按/计数、非阻塞 WiFi、OTA、网页、告警、NVS
- 版本标记改成 `Day 21: idle back home`
- Day 18/19/20 那些已经填完的 `TODO 空` 注释清理掉了，工程里现在**只有一个** TODO 区

## 今天自己写的部分：`src/main.cpp` 里 `Day 21 TODO 区` 的 3~5 行
位置在 `void loop()` 中段、长按判断之后、`lastBtn = btn;` 之前。

提示（不给答案）：
1. `loop()` 里已经有一个变量在记「上次按 BOOT 是什么时候」，**自己往上找**，
   直接用它，不要另起炉灶声明一个新的
2. 时间差还是 `millis() - 那个变量`，跟 Day 18 长按一模一样；两个条件用 `&&`
   （Day 18 踩过 `if (a, b)` 逗号运算符的坑）
3. **今天最容易踩的坑**：那个变量刚开机时是 `0`。等 `millis()` 走过 `IDLE_MS`
   之后，`millis() - 0 >= IDLE_MS` 会**每一轮都成立** → 串口刷屏。
   想想条件里还要挡住什么
4. 回首页用 `oledPage = 0`，串口打印 `idle -> home`
5. 这几行空着时编译照样能过，只是不会自动回首页

编译报错时我只给提示，不贴答案。

## 现象 / 验收标准（按顺序）
1. 烧录后按 **RST**，串口第一行 `Day 21: idle back home`
2. 短按两下到 `3/3`，**放手不动** → 数 5 秒，屏幕自己跳回 `1/3`，
   串口 `idle -> home` **只打印一次**，不刷屏
3. 计时途中按一下 BOOT → 重新开始计时（不是从开机算）
4. **开机后一直不按**：串口应该安安静静，不能刷 `idle -> home` ← 第 3 条提示说的坑
5. 把 `IDLE_MS` 从 `5000` 改成 `1000`，重新编译烧录 → 回首页明显变快。
   这是今天「改一个数字 → 看现象变」的那一步，别跳过；验完可以改回 5000

## 学到什么
更进一步的了解怎么使用if语句，并且现在能看提示写代码了

## 踩坑预告
- 变量初值 `0` + `millis() - 0` 恒成立，是这类「空闲判断」的经典坑，
  和 Day 18 的 `pressMs != 0`、Day 20 的 `otaStarted` 是同一个道理
- `static` 变量跨 `loop()` 保留值，别写成普通局部变量
- 今天连改四版，卡点不是 C 语法，是**把短按的「按下边沿」抄进了空闲判断**：
  `btn` / `lastBtn` / `pressMs` 都是「按键正在发生」的信号，
  「空闲」只关心一件事 —— 距上次按键过去了多久
- 烧录一律命令行，别点 VS Code 平台栏（会烧成它当前打开的文件夹）
- 烧录前先 `Ctrl+C` 退出串口监视器，否则 `Could not open COM6, port is busy`；
  烧录结束会自己复位，所以**先开监视器、再按 RST** 才看得到版本标记

## 待办
- Day 20 的因果实验（删 `otaStarted = true` 看 `OTA ready` 刷屏）还没亲手做
- Day 16 的 OTA **无线上传**待补验
- AHT20 温湿度传感器未到货

## 复现
```powershell
cd D:\100-days-embedded\day21-idle-home
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```
WiFi 用的还是 NVS 里存的 `Redmi K70`，不用重新配网（重烧不清 NVS）。
