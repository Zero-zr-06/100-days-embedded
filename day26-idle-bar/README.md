# Day 26：把空闲时间画成屏幕上的进度条

## 今天做什么

Day 21 写了"多久没按就回首页"，Day 22~25 把时间、开关都显示到了屏幕上。
但"还剩多久回首页"还是看不见。今天在温度页底部画一条进度条：**时间越久，条越长**，
按 RST 后能看到它从空慢慢涨满——第一次把 `millis()` 这个抽象数字变成屏幕上的一条线。

## 已写好的部分

- 版本标记：`Day 26: idle countdown bar`
- 已给你：`elapsed` 已经算好（距上次按键过了多久），你只管把它变成像素
- **今天替你改了一处前提**：`lastBtnMs`（上次按键时刻）原来藏在 `loop()` 里的
  `static` 局部变量，`refreshOled()` 够不着。我把它提到了文件顶部当全局变量
  （跟 `bootMs` 一样）。这就是"作用域"：局部变量只有那个函数能用，全局变量谁都能读

## 今天写两处填空（共 4 行，都在 `refreshOled()` 温度页分支的 Day 26 区）

- **填空 1（3 行）**：把 `elapsed` 换算成 0~127 的像素宽 `w`
  1. 算比例：`elapsed / idleMs`，**注意整数相除会得到 0，要先转 float**
  2. 比例超过 1 就当 1（`if` 挡一下，别让 w 超出屏幕）
  3. `w = 比例 * 127`，转成 int
- **填空 2（1 行）**：`oled.fillRect(0, 56, w, 6, SSD1306_WHITE)` 画出来

提示在代码注释框里，不给答案。`idleEnabled` 为 false 时这段不执行（已经用 if 包好）。

## 编译

在 `D:\100-days-embedded\day26-idle-bar` 目录下：

```
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run
```

卡超过两分钟就用跳过联网的两条环境变量（见 day25 README）。

## 烧录 + 验收

先退出监视器再烧：

```
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
```

等 `Hash of data verified` + `[SUCCESS]`，再先开监视器、按 RST：

1. 版本标记 `Day 26: idle countdown bar`
2. **温度页底部出现一条横线**，按 RST 后从几乎看不见慢慢变长（约 5 秒涨满）← 核心
3. 涨满时屏幕跳回首页（其实本来就在首页，看串口 `idle -> home` 不会打印——
   因为空闲回首页有 `oledPage != 0` 挡板，首页不触发。这条是帮你理解挡板的作用）
4. 翻到 3/3，敲 `idle off` → 回温度页，条不再画（开关关了）
5. 敲 `idle on` → 回温度页，条又出现
6. 敲 `idle 1000` → 按 RST，条涨满只要 1 秒（时间常量也能控制条的快慢）

第 2 条是今天的核心：**变量 → 像素**。`elapsed` 是个数字，`fillRect` 把它变成看得见的长度。

## 学到什么

把比例乘屏幕宽度就变成了像素长度

## 踩坑预告

- **作用域**：`lastBtnMs` 原来是 `loop()` 里的 `static` 局部变量，别处看不到；
  今天两个函数都要用，就提成全局变量。局部变量=只有那个函数能用，全局=全文件能用
- 整数相除：`5000 / 10000` 在 C 里等于 `0`（不是 0.5），要先转成 float 再除
- 比例到像素：比例是 0~1，屏幕宽是 0~127，乘起来才是像素坐标——这就是"变量映射成图形"
- 越界防护：`elapsed` 可能超过 `idleMs`（比如开机没按键时），比例 >1 会把条画到屏幕外，要先挡住
- 画条用 `oled.fillRect(x, y, w, h, color)`：x/y 是左上角，w/h 是宽高

## 复现命令

```
cd D:\100-days-embedded\day26-idle-bar
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload
& "C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" device monitor --echo
```
