# Day 23：串口设的空闲时间，断电也记得住（NVS）

## 今天做什么

昨天敲 `idle 1000` 好用，但一按 RST 就变回 5000 —— 因为值只在内存里，断电就丢。
今天把它写进 NVS（板子上的 Flash 掉电不丢的存储区），重启后自己读回来。

本质是复用 Day 10 那套：存阈值用 `prefs.putFloat("th", threshold)`，
开机读回用 `threshold = prefs.getFloat("th", 38.0)`。今天给空闲时间再来一遍，
但注意类型不同：`idleMs` 是 `unsigned long`，不是 `float`。

## 已经给你写好的部分（不用动）

- 版本标记 `Day 23: idle saved in NVS`
- Day 22 的串口 `idle` 命令整套保留（你自己写的那段，一个字没动）
- NVS 已经打开：`prefs.begin("cfg", false);` 在 setup 里
- 昨天阈值 `th` 的存/读原样保留，照着它抄结构就行

## 今天自己写的部分：两个填空，共 3 行

### 填空 A（`setup()` 里，`threshold restored` 下面）—— 2 行

目标：开机把存过的 `idleMs` 读回来，没存过用 5000，再打印一句确认。

提示（不给答案）：

1. 照上面那行 `threshold = prefs.getFloat("th", 38.0);` 的形状写
2. 但 `idleMs` 是 `unsigned long`（昨天你自己选的类型），**不是 float** ——
   把 `getFloat` 里的 `Float` 换成你这个类型对应的名字（`U` = unsigned）
3. key 用 `"idle"`，默认值 `5000`；打印格式用 `%lu`（昨天用过）

### 填空 B（`handleCmd()` 的 `idle` 分支里）—— 1 行

目标：敲 `idle 1000` 时，除了改内存里的值，顺手写进 NVS。

提示（不给答案）：

1. 照 `th` 命令里那行 `prefs.putFloat("th", threshold);` 写
2. 同样把 `putFloat` 的 `Float` 换成 `idleMs` 的类型名，key 用 `"idle"`，值用 `idleMs`

编译不过把报错原文贴给我，**我只给提示、不贴答案**。

## 现象 / 验收标准（按顺序）

1. 烧录后按 **RST**，串口找 `Day 23: idle saved in NVS`
2. 开机那行 `idle restored: 5000 ms`（第一次跑，NVS 里还没存过 idle，用默认值）
3. 敲 `idle` → `idle now: 5000 ms`
4. 敲 `idle 1000` → `idle -> 1000 ms`
5. **再按一次 RST**（关键）→ 串口打 `idle restored: 1000 ms` —— 不是 5000 了，说明记住了
6. 敲 `idle` 回车 → `idle now: 1000 ms`，确认读回来的值真的在用
7. 翻到 `3/3` 放手数 1 秒就回首页（昨天验过的现象，今天值是从 NVS 来的）

## 学到什么
学会怎么定义默认值，并且知道类型对应了


## 踩坑预告

- `unsigned long` 存进 NVS 要用**配套的函数名**（跟 `float` 用的不是一套），
  名字写错编译直接报错 —— 这正是今天要练的「类型对应」
- NVS 是**掉电不丢**的：今天存进去的东西，明天开机还在。
  验收完记得敲 `idle 5000` 把默认值存回去，不然接下来每天开机都是 1 秒回首页
- 函数名记不住就去看现成的 `th` 那两行，照着换类型名，别凭空想
- 烧录一律命令行，别点 VS Code 平台栏（会烧成它当前打开的文件夹）
- 烧录前先 `Ctrl+C` 退出串口监视器；烧录结束会自己复位，所以**先开监视器、再按 RST**

## 待办

- Day 20 的因果实验（删 `otaStarted = true` 看 `OTA ready` 刷屏）还没亲手做
- Day 16 的 OTA **无线上传**待补验
- AHT20 温湿度传感器未到货

## 复现

```powershell
cd D:\100-days-embedded\day23-idle-nvs
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```

监视器里敲 `idle` 看当前值，敲 `idle 1000` 改快，按 RST 验证记住了，
最后敲 `idle 5000` 改回默认值。WiFi 用的还是 NVS 里存的 `Redmi K70`。
