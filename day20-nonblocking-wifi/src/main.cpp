#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Esp.h>
#include "wifi_secrets.h"
#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoOTA.h>

#define RGB_PIN 48

const char* WIFI_SSID = WIFI_SSID_NAME;
const char* WIFI_PASS = WIFI_PASSWORD;

WebServer server(80);

static const uint8_t MAX_LEVEL = 40;
const uint8_t COLORS[6][3] = {
  {1,0,0},{0,1,0},{0,0,1},{1,1,0},{0,1,1},{1,0,1}
};
const char* NAMES[6] = {"Red","Green","Blue","Yellow","Cyan","Purple"};

int  colorIdx  = 0;
bool breathing = true;
int  level = 0, step = 1;
unsigned long lastBreath = 0;
const int BREATH_INTERVAL = 15;
unsigned long bootMs = 0;

// ---- 温度历史（Day 8 的环形数组）----
#define HIST_N 60
float hist[HIST_N];
int  histCount = 0, histPos = 0;
unsigned long lastSample = 0;
const unsigned long SAMPLE_MS = 2000;

// ---- Day 9 新增：阈值 + 滞回 + 状态 ----
float threshold = 38.0;     // 首次开机默认（实测基线+2，见 day09 踩坑）
bool  alert     = false;
Preferences prefs;          // NVS：往 Flash 里存参数，掉电不丢
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
const float HYST = 1.0;     // 滞回：低于 阈值-1 才解除，防止临界点反复跳
int oledPage = 0;              // 0=温度 1=曲线 2=系统
int pressCount = 0;            // BOOT 按过几次

void applyLevel() {
  if (!breathing) { neopixelWrite(RGB_PIN, 0, 0, 0); return; }
  neopixelWrite(RGB_PIN,
                COLORS[colorIdx][0] * level,
                COLORS[colorIdx][1] * level,
                COLORS[colorIdx][2] * level);
}

String page() {
  String s = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  s += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  s += "<title>ESP32-S3 Monitor</title></head><body style='font-family:sans-serif'>";
  s += "<h2>ESP32-S3 Monitor</h2>";

  // 告警横幅
  s += "<div id='warn' style='font-size:22px;font-weight:bold'>checking...</div>";

  // 阈值设置表单：填数字点 Set，会请求 /set?th=xx
  s += "<form action='/set'>Threshold: <input name='th' value='";
  s += String(threshold, 1);
  s += "' style='width:70px'> C <button>Set</button></form><br>";

  for (int i = 0; i < 6; i++) {
    s += "<a href='/color?i=" + String(i) + "'>";
    s += "<button style='font-size:18px;padding:12px 20px;margin:5px'>" + String(NAMES[i]) + "</button></a>";
  }
  s += "<br><br><a href='/breath?on=1'><button style='font-size:18px;padding:12px 20px;margin:5px'>Breath ON</button></a>";
  s += "<a href='/breath?on=0'><button style='font-size:18px;padding:12px 20px;margin:5px'>Breath OFF</button></a>";
  s += "<a href='/off'><button style='font-size:18px;padding:12px 20px;margin:5px'>OFF</button></a>";

  s += "<h3>Live Data</h3><div id='box'>loading...</div>";
  s += "<h3>Temp (last 2 min)</h3>";
  s += "<canvas id='cv' width='320' height='120' style='border:1px solid #aaa'></canvas>";

  s += "<script>"
       "function draw(a){var c=document.getElementById('cv');if(!c)return;"
       "var g=c.getContext('2d');g.clearRect(0,0,c.width,c.height);"
       "if(!a||a.length<2)return;"
       "var mn=Math.min.apply(null,a),mx=Math.max.apply(null,a);if(mx-mn<1){mx=mn+1;}"
       "g.beginPath();"
       "for(var i=0;i<a.length;i++){"
       "var x=2+i/(a.length-1)*(c.width-4);"
       "var y=c.height-4-(a[i]-mn)/(mx-mn)*(c.height-8);"
       "if(i==0)g.moveTo(x,y);else g.lineTo(x,y);}"
       "g.strokeStyle='#e33';g.lineWidth=2;g.stroke();}"
       "async function upd(){try{var r=await fetch('/api');var j=await r.json();"
       "var w=document.getElementById('warn');"
       "if(j.alert){w.innerHTML='&#9888; ALERT! Temp too high';w.style.color='#d00';}"
       "else{w.innerHTML='OK (temp below '+j.th+' C)';w.style.color='#080';}"
       "document.getElementById('box').innerHTML='Uptime: '+j.uptime+' s<br>Heap: '+j.heap+' B<br>"
       "WiFi RSSI: '+j.rssi+' dBm<br>Temp now: '+j.temp+' C<br>Now: '+j.color"
       "+(j.breathing?' (breathing)':' (solid)');"
       "draw(j.hist);}catch(e){}}"
       "upd();setInterval(upd,2000);</script>";

  s += "</body></html>";
  return s;
}
// ---- Day 11 新增：串口命令处理 ----
String cmdBuf = "";
void handleCmd(String line) {
  line.trim();                          // 去掉末尾回车/空格
  if (line.length() == 0) return;
  if (line.startsWith("th ")) {
    float v = line.substring(3).toFloat();
    if (v > 20.0 && v < 80.0) {
      threshold = v;
      prefs.putFloat("th", threshold);  // 顺手存 NVS，Day 10 的功能直接复用
      Serial.printf("threshold -> %.1f (saved)\n", threshold);
    } else Serial.println("th out of range (20-80)");
  } else if (line == "show") {
    Serial.printf("temp %.1f C | th %.1f | %s | heap %u B\n",
                  temperatureRead(), threshold,
                  alert ? "ALERT" : "OK", ESP.getFreeHeap());
                    } else if (line == "wifi" || line.startsWith("wifi ")) {   // 精确匹配，别把 wificlr 吞掉
    int s1 = line.indexOf(' ');
    if (s1 < 0) {                     // 不带参数：显示当前
      Serial.printf("wifi now: %s (RSSI %d dBm)\n", WiFi.SSID().c_str(), WiFi.RSSI());
      Serial.println("usage: wifi <ssid> <password>");
    } else {
      int s2 = line.lastIndexOf(' ');   // 用最后一个空格切：SSID 可带空格，密码放最后且不含空格
      if (s2 < 0) { Serial.println("usage: wifi <ssid> <password>"); return; }
      String nssid = line.substring(s1 + 1, s2); nssid.trim();
      String npass = line.substring(s2 + 1);     npass.trim();
      if (nssid.length() == 0 || npass.length() < 8) {
        Serial.println("bad args (password must be >= 8 chars)");
      } else {
        prefs.putString("ssid", nssid);
        prefs.putString("pass", npass);
        Serial.printf("wifi saved: %s, type reboot to apply\n", nssid.c_str());
      }
    }
  } else if (line == "wificlr") {      // 清掉存的，回到 wifi_secrets.h 那套
    prefs.remove("ssid"); prefs.remove("pass");
    Serial.println("wifi config cleared, reboot to use wifi_secrets.h");
  } else if (line == "reboot") {
    Serial.println("rebooting...");
    delay(300);
    ESP.restart();
  } else {
    Serial.println("unknown cmd. try: show / th <num> / reboot");
  }
}
void refreshOled() {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);

  if (oledPage == 0) {                       // 页 1/3：大字温度
    oled.println("== TEMP ==");
    oled.setTextSize(2);
    oled.printf("%.1f C\n", temperatureRead());
    oled.setTextSize(1);
    oled.printf("th    %.1f C\n", threshold);
    oled.println(alert ? "!! ALERT !!" : "status OK");
    oled.println("BOOT -> next page");

  } else if (oledPage == 1) {               // 页 2/3：全屏曲线
    oled.println("== CHART / 2min ==");
    if (histCount > 1) {
      float mn = 200, mx = -50;
      for (int i = 0; i < histCount; i++) {
        float v = hist[(histPos - histCount + i + HIST_N) % HIST_N];
        if (v < mn) mn = v;
        if (v > mx) mx = v;
      }
      if (mx - mn < 0.5) mx = mn + 0.5;
      int lastX = 0, lastY = 0;
      for (int i = 0; i < histCount; i++) {
        int k = (histPos - histCount + i + HIST_N) % HIST_N;
        int x = 2 + i * 124 / (histCount - 1);
        int y = 63 - (int)((hist[k] - mn) / (mx - mn) * 50);
        if (i > 0) oled.drawLine(lastX, lastY, x, y, SSD1306_WHITE);
        lastX = x; lastY = y;
      }
    }

  } else {                                  // 页 3/3：系统信息
    oled.println("== SYSTEM ==");
    oled.println(WiFi.status() == WL_CONNECTED ? "WiFi connected" : "WiFi offline");
    oled.printf("IP   %s\n", WiFi.localIP().toString().c_str());
    oled.printf("RSSI %d dBm\n", WiFi.RSSI());
        oled.printf("heap %u B\n", ESP.getFreeHeap());
    oled.printf("up   %u s\n", (millis() - bootMs) / 1000);
    oled.printf("pressCount %d\n", pressCount);
    // TODO 空 3: 在这里把 pressCount 显示出来
  }

  oled.setTextSize(1);
  oled.setCursor(104, 0);
  oled.printf("%d/3", oledPage + 1);
  oled.display();
}



void handleApi() {
  String j = "{";
  j += "\"uptime\":"    + String((millis() - bootMs) / 1000);
  j += ",\"heap\":"     + String(ESP.getFreeHeap());
  j += ",\"rssi\":"     + String(WiFi.RSSI());
  j += ",\"temp\":"     + String(temperatureRead(), 1);
  j += ",\"th\":"       + String(threshold, 1);
  j += ",\"alert\":"    + String(alert ? 1 : 0);
  j += ",\"color\":\""  + String(NAMES[colorIdx]) + "\"";
  j += ",\"breathing\":" + String(breathing ? 1 : 0);
  j += ",\"hist\":[";
  for (int i = 0; i < histCount; i++) {
    int k = (histPos - histCount + i + HIST_N) % HIST_N;
    if (i) j += ",";
    j += String(hist[k], 1);
  }
  j += "]}";
  server.send(200, "application/json", j);
}

void sendHome() {
  server.sendHeader("Location", "/");
  server.send(303);
}

// ---- Day 20：把「连上 WiFi 之后才能做的事」单独装进一个函数 ----
// 这个函数我写好了，你不用改。今天你只负责在 loop() 里决定：
//   什么时候调用它、以及调用几次。
void startOta() {
  Serial.print(">>> open: http://");
  Serial.println(WiFi.localIP());
  ArduinoOTA.setHostname("esp32-s3-ota");
  ArduinoOTA.onStart([]() { Serial.println("OTA start"); });
  ArduinoOTA.onEnd([]()   { Serial.println("\nOTA done, rebooting"); });
  ArduinoOTA.onProgress([](unsigned int p, unsigned int t) {
    Serial.printf("OTA: %u%%\r", p * 100 / t);
  });
  ArduinoOTA.onError([](ota_error_t e) { Serial.printf("OTA Error[%u]\n", e); });
  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

void setup() {
  Serial.begin(115200);
  delay(500);
    Wire.begin(8, 9);                      // SDA=GPIO8, SCL=GPIO9
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("OLED init failed");
  else Serial.println("OLED ready");
  Serial.println("\nDay 20: non-blocking WiFi");
  pinMode(0, INPUT_PULLUP);   // BOOT 键：GPIO0，按下为低
  prefs.begin("cfg", false);               // 打开叫 "cfg" 的存储区
  threshold = prefs.getFloat("th", 38.0);  // 有存过就读，没存过用 38.0
  Serial.printf("threshold restored: %.1f C\n", threshold);


   // ---- WiFi 凭据优先用 NVS 存的，没有才用 wifi_secrets.h ----
  String ssid = prefs.getString("ssid", WIFI_SSID_NAME);
  String pass = prefs.getString("pass", WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.print("connecting to ");
  Serial.println(ssid);

  // ---- Day 20：这里原来是 while 死等最多 20 秒 ----
  // WiFi.begin() 是异步的：调用完就返回，真正的连接在后台自己跑。
  // setup() 只做「开机一次性」的事，一旦卡住，这 20 秒里 loop() 完全不执行，
  // 于是按键没反应、串口命令也没回显 —— 就是昨天那个坑。
  // 现在删掉等待，setup() 立刻结束，连没连上交给 loop() 每轮去查。
  server.on("/",      []() { server.send(200, "text/html; charset=utf-8", page()); });
  server.on("/api",   handleApi);
  server.on("/set",   []() {                 // 网页下发新阈值
    if (server.hasArg("th")) {
      float v = server.arg("th").toFloat();
      if (v > 20.0 && v < 80.0) {
        threshold = v;
        prefs.putFloat("th", threshold);         // 立刻写进 Flash
        Serial.printf("threshold -> %.1f C\n", threshold);
      }
    }
    sendHome();
  });
  server.on("/color", []() {
    if (server.hasArg("i")) {
      colorIdx = server.arg("i").toInt() % 6;
      breathing = true; level = 0; step = 1; applyLevel();
    }
    sendHome();
  });
  server.on("/breath", []() {
    if (server.hasArg("on")) { breathing = server.arg("on").toInt() == 1; applyLevel(); }
    sendHome();
  });
  server.on("/off", []() { breathing = false; applyLevel(); sendHome(); });
  server.begin();
  Serial.println("HTTP server started");
  // Day 20：OTA 不在这儿开了。连上 WiFi 之后由 loop() 调用 startOta()，
  // 具体什么时候调用、调用几次，是今天留给你的那几行。
  bootMs = millis();
}

void loop() {
  // ==========================================================
  // Day 20 TODO 区 —— 今天自己写 3~5 行，就写在这个位置
  // ==========================================================
  // 目标：WiFi 连上之后，调用 startOta() 一次，而且只调用一次。
  //
  // 已经给你的：
  //   * startOta() 函数（在 setup 上面，写好了，别改）
  //   * WiFi.status() == WL_CONNECTED 表示连上了
if(WiFi.status() == WL_CONNECTED){
  static bool otaStarted = false; // 记住是否已经调用过 startOta()
  if(!otaStarted){// 如果还没调用过，就调用一次
    startOta();// 调用 startOta()，然后把 otaStarted 设为 true
    otaStarted = true;
  }
}
  // 提示（不给答案）：
  //   1. 需要一个 static bool 记住「是不是已经调用过」。
  //      想想 Day 19 的 pressCount、Day 18 的 pressMs，
  //      它们都是靠一个变量跨 loop() 记住状态
  //   2. 两个条件要同时成立，用 &&（Day 18 踩过逗号运算符的坑）
  //   3. 进 if 之后第一件事该干什么？漏掉会怎样 —— 填完故意删一次看串口
  //   4. 现在这几行空着，编译照样能过，只是串口不会有 IP 和 "OTA ready"
  //
  // 填完再编译烧录，按 README 的顺序验收。
  // ==========================================================

    // ---- Day 11：读串口命令（非阻塞）----
    // ---- Day 17：BOOT 按键切页 ----
   // ---- Day 17: short press = next page ----
  static bool lastBtn = HIGH;
  static unsigned long lastBtnMs = 0;
  bool btn = digitalRead(0);

  // ---- Day 18: long press = back to page 1 ----
  static unsigned long pressMs = 0;   // 0 means: not pressed, or already handled
  if (lastBtn == HIGH && btn == LOW) pressMs = millis();  // press edge
  if (lastBtn == LOW && btn == HIGH) pressMs = 0;         // release edge

      if (lastBtn == HIGH && btn == LOW && millis() - lastBtnMs > 200) {
    lastBtnMs = millis();
    oledPage = (oledPage + 1) % 3;
    Serial.printf("OLED page -> %d\n", oledPage + 1);
    // TODO 空 1: 每短按一次，把 pressCount 加 1
    pressCount++;
  }

  // TODO: fill in these 5 lines
  // hint: compare millis() - pressMs with 1000
  //       set oledPage to 0, print "long press -> home", then clear pressMs
     if (millis() - pressMs >= 1000 && pressMs != 0) {
    oledPage = 0;
    Serial.println("long press -> home");
    pressCount = 0;// TODO 空 2: 长按顺便把 pressCount 归零
    pressMs = 0;
  }

  lastBtn = btn;
                  // 把「这次」存起来，下轮它就成了「上次」
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n')      { handleCmd(cmdBuf); cmdBuf = ""; }
    else if (c != '\r') { cmdBuf += c; }
  }
  server.handleClient();    
  ArduinoOTA.handle();
  static unsigned long lastOled = 0;
  if (millis() - lastOled >= 500) { lastOled = millis(); refreshOled(); }

  // ---- 掉线自动重连：每 10 秒查一次，不用 delay ----
  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck >= 10000) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi lost, reconnecting...");
      WiFi.reconnect();
    }
  }

  // ---- 每 2 秒采样 + 阈值判断 ----
  if (millis() - lastSample >= SAMPLE_MS) {
    lastSample = millis();
    float t = temperatureRead();
    hist[histPos] = t;
    histPos = (histPos + 1) % HIST_N;
    if (histCount < HIST_N) histCount++;

    // 状态机：只在"状态变化"的那一刻打印，不每轮刷屏
    if (!alert && t >= threshold) {
      alert = true;
      Serial.printf("ALERT ON : %.1f >= %.1f\n", t, threshold);
    } else if (alert && t <= threshold - HYST) {
      alert = false;
      Serial.printf("ALERT OFF: %.1f <= %.1f\n", t, threshold - HYST);
    }
    //Serial.printf("temp %.1f C | th %.1f | %s\n", t, threshold, alert ? "ALERT" : "OK");
  }

  // ---- 告警时红灯快闪，否则照常呼吸 ----
  if (alert) {
    bool on = (millis() / 300) % 2 == 0;
    neopixelWrite(RGB_PIN, on ? MAX_LEVEL : 0, 0, 0);
  } else if (breathing && millis() - lastBreath >= BREATH_INTERVAL) {
    lastBreath = millis();
    level += step;
    if (level >= MAX_LEVEL) { level = MAX_LEVEL; step = -1; }
    if (level <= 0) { level = 0; step = 1; colorIdx = (colorIdx + 1) % 6; }
    applyLevel();
  }
}
