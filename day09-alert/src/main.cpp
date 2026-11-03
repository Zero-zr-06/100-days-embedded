#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Esp.h>
#include "wifi_secrets.h"

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
float threshold = 38.0;     // 告警阈值，网页可改
bool  alert     = false;    // 当前是否告警
const float HYST = 1.0;     // 滞回：低于 阈值-1 才解除，防止临界点反复跳

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

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nDay 9: threshold alert + auto reconnect");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("connecting to ");
  Serial.println(WIFI_SSID);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) {
    delay(500); Serial.print("."); tries++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(">>> open: http://");
    Serial.println(WiFi.localIP());
    if (MDNS.begin("esp32-s3")) Serial.println(">>> or:   http://esp32-s3.local");
  } else {
    Serial.println("WiFi FAILED - will keep retrying in loop");
  }

  server.on("/",      []() { server.send(200, "text/html; charset=utf-8", page()); });
  server.on("/api",   handleApi);
  server.on("/set",   []() {                 // 网页下发新阈值
    if (server.hasArg("th")) {
      float v = server.arg("th").toFloat();
      if (v > 20.0 && v < 80.0) {
        threshold = v;
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
  bootMs = millis();
}

void loop() {
  server.handleClient();

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
    Serial.printf("temp %.1f C | th %.1f | %s\n", t, threshold, alert ? "ALERT" : "OK");
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
