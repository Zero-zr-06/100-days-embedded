# Day 22：串口直接改空闲时间（不用重烧）

## 今天做什么

昨天 `idleMs` 写在代码里，想从 5 秒改成 1 秒必须改代码 → 编译 → 重烧。
今天给它加一条串口命令 `idle <毫秒>`：在监视器里敲一行回车，空闲时间立刻变，板子不用重启、不用重烧。

## 已经给你写好的部分（不用动）

- 文件顶部 `unsigned long idleMs = 5000;` —— 注意 **`const` 被去掉了**。
  `const` 是常量，程序运行中改不了；想让串口改它，第一步必须去掉 `const`
- 串口命令里 `idle`（不带数字）这个分支：敲 `idle` 回车会打印当前值
- Day 21 的空闲判断原样保留，只是里面的 `IDLE_MS` 改名成了 `idleMs`
- `unknown cmd` 提示里加了 `idle <ms>`
- 版本标记 `Day 22: serial idle setting`

## 今天自己写的部分：`src/main.cpp` 里 `Day 22 TODO 区` 的 3~5 行

位置在 `handleCmd()` 里、`line.startsWith("idle ")` 那个分支中。

**题目**：串口敲 `idle 1000` 回车 → `idleMs` 变成 `1000`，并打印一句确认。

提示（不给答案）：

1. 照上面 `th` 命令那几行抄结构：`float v = line.substring(3).toFloat();`
   `"th "` 是 3 个字符（t、h、空格），所以写 `substring(3)`。
   那 `"idle "` 是几个字符？`substring()` 括号里该填几？（空格也算一个字符）
2. 毫秒是整数，用 `toInt()` 比 `toFloat()` 合适
3. 取出来的数字赋给谁？赋完打印一句确认，格式自己定
4. 这几行空着时编译照样能过，只是敲 `idle 1000` 不会有任何变化

编译不过就把报错原文贴给我，**我只给提示、不贴答案**。

## 现象 / 验收标准（按顺序）

1. 烧录后按 **RST**，串口找 `Day 22: serial idle setting`
2. 敲 `idle` 回车 → 打印 `idle now: 5000 ms`
3. 短按两下到 `3/3` 页，敲 `idle 1000` 回车 → 打印你写的那句确认
4. **放手不动数 1 秒** → 屏幕自己回 `1/3`，串口 `idle -> home`
   （昨天要等 5 秒，今天 1 秒就回 —— 这就是「改一个数字 → 看现象变」）
5. 敲 `idle 5000` 回车改回，再翻到 `3/3` 放手，又要等 5 秒才回
6. 按 **RST** 重启 → 敲 `idle` 又变回 5000（今天没存 NVS，明天再解决）

## 学到什么
知道在定义时一定要找对字符类型，并且要注意字符包括空格

## 踩坑预告

- `const` 修饰的变量运行中改不了，想用串口改必须先把 `const` 去掉
- **跨函数共享**：这次是 `handleCmd()` 改 `idleMs`、`loop()` 读它 —— 跟 Day 19 的 `pressCount` 是同一件事，方向反过来
- `substring(n)` 的 n 是「从第几个字符开始」，从 0 数；`idle ` 结尾那个空格也算一个字符
- 烧录一律命令行，别点 VS Code 平台栏（会烧成它当前打开的文件夹）
- 烧录前先 `Ctrl+C` 退出串口监视器，否则 `Could not open COM6, port is busy`；
  烧录结束会自己复位，所以**先开监视器、再按 RST** 才看得到版本标记

## 待办

- 串口改的值断电就丢 —— 下一步存进 NVS（Day 10 的 `prefs` 已经会用了）
- Day 20 的因果实验（删 `otaStarted = true` 看 `OTA ready` 刷屏）还没亲手做
- Day 16 的 OTA **无线上传**待补验
- AHT20 温湿度传感器未到货

## 复现

```powershell
cd D:\100-days-embedded\day22-serial-idle
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```

监视器里敲 `idle` 看当前值，敲 `idle 1000` 改快，敲 `idle 5000` 改回。
WiFi 用的还是 NVS 里存的 `Redmi K70`，不用重新配网（重烧不清 NVS）。
