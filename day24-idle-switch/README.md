# Day 24：给空闲回首页装个开关（bool + NVS）

## 今天做什么

昨天把空闲时间存进 NVS 了。今天再加一个**总开关**：

- 敲 `idle off` → 关掉，翻到 `3/3` 放手再久也**不**自动回首页
- 敲 `idle on` → 打开，恢复昨天那样 5 秒回首页
- 开关状态也存 NVS，重启后还记得（关掉了重启还是关的）

核心新东西：`bool` 型的 NVS 读写（`getBool` / `putBool`）—— 照昨天的 `getULong` / `putULong` 换个类型名，套路一模一样。

## 已经给你写好的部分（不用动）

- 版本标记 `Day 24: idle on/off switch`
- 全局开关变量 `bool idleEnabled = true;`（默认开着，就是昨天的行为）
- 命令 `idle`（不带参数）：打印 `idle now: xxx ms | switch ON/OFF`，方便看状态
- 命令分支的骨架和提示（填空 B 的位置）
- 打印开关状态的语句（填空 A 下面那行）
- Day 21~23 全部保留：空闲计时、串口改值、NVS 存值

⚠️ 命令顺序：`idle off` / `idle on` 是**精确匹配**，已经放在 `startsWith("idle ")` 前面了，所以不会被当成数字命令吃掉。

## 今天自己写的部分：三个填空，共 4 行

### 填空 A（`setup()` 里，`idle restored` 下面）—— 1 行

目标：开机把开关状态读回来。照 `getULong` 那行写：`get` + 类型 `Bool`，key 用 `"idleOn"`，默认值 `true`。

### 填空 B（`handleCmd()` 的 `idle off/on` 分支里）—— 2 行

目标：改 `idleEnabled`，并存进 NVS。

提示（不给答案）：

1. `line == "idle on"` 这个比较本身就是 `true`/`false`（敲 on 时为真、off 时为假），
   直接赋值给 `idleEnabled` 就行，**不用写 if**
2. 存 NVS 照昨天 `putULong` 的形状：动作 `put` + 类型 `Bool`，key 用 `"idleOn"`，值是 `idleEnabled`

### 填空 C（`loop()` 的空闲判断里）—— 1 行

目标：开关关掉时别回首页。在 `if (millis() - lastBtnMs >= idleMs && oledPage != 0)` 的条件里加一个判断：`idleEnabled` 得是 `true`。照 Day 21 的 `&&` 用法。

编译不过贴报错原文给我，**我只给提示、不贴答案**。

## 现象 / 验收标准（按顺序）

1. 烧录后按 **RST**，串口找 `Day 24: idle on/off switch`
2. 开机 `idle switch: ON`（第一次跑，没存过，用默认值 true）
3. 敲 `idle` → `idle now: 5000 ms | switch ON`
4. 翻到 `3/3`，敲 `idle off` → 敲 `idle` 看 `switch OFF`
5. **放手等 6 秒** → 屏幕**停在 3/3 不动**（这是今天的核心，跟昨天必回首页对比）
6. 敲 `idle on` → 再翻到 `3/3` 放手，5 秒又自动回 `1/3`，串口 `idle -> home`
7. **按 RST 重启**（先敲 off 再重启）→ 开机 `idle switch: OFF`，翻到 3/3 放手还是不回（记住了）

## 学到什么
函数名是动作+类型


## 踩坑预告

- 命令分支顺序：`idle off` 必须以**精确匹配**放在 `startsWith("idle ")` 前面，
  否则 "off" 会被 `toInt()` 当成 0，掉进数字分支（骨架已经给你排好序了）
- `bool` 存 NVS 用配套的 `getBool`/`putBool`，跟昨天 `unsigned long` 的 `ULong` 又不一样 —— 类型对应这套今天是第三遍了
- `line == "idle on"` 直接当值用（比较结果是 true/false），这是个省事的写法，
  想不通就改成普通 if 也行
- NVS 掉电不丢：验收完记得敲 `idle on` 恢复默认，不然明天开机开关还是关的
- 烧录一律命令行，别点 VS Code 平台栏；烧录前 `Ctrl+C` 退监视器；烧完**先开监视器再按 RST**

## 待办

- Day 20 的因果实验（删 `otaStarted = true` 看 `OTA ready` 刷屏）还没亲手做
- Day 16 的 OTA **无线上传**待补验
- AHT20 温湿度传感器未到货

## 复现

```powershell
cd D:\100-days-embedded\day24-idle-switch
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```

监视器里敲 `idle` 看状态，`idle off` 关、`idle on` 开。
编译若卡住超过 2 分钟（PlatformIO 联网检查更新），加环境变量跳过：

```powershell
$env:PLATFORMIO_SETTING_CHECK_PLATFORMIO_INTERVAL="999999"
$env:PLATFORMIO_SETTING_ENABLE_TELEMETRY="No"
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
```
