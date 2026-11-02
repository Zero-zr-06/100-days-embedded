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

// ---- 温度历史：环形数组，最多存 60 个点 ----
#define HIST_N 60
float hist[HIST_N];
int        histCount = 0;   // 已经存了几个点
int        histPos   = 0;   // 下一个要写的位置
unsigned long lastSample = 0;
const unsigned long SAMPLE_MS = 2000;   // 每 2 秒采一次

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
  s += "<title>ESP32-S3 Panel</title></head><body style='font-family:sans-serif'>";
  s += "<h2>ESP32-S3 RGB Control</h2>";
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
       "document.getElementById('box').innerHTML='Uptime: '+j.uptime+' s<br>Heap: '+j.heap+' B<br>"
       "WiFi RSSI: '+j.rssi+' dBm<br>Temp now: '+j.temp+' C<br>Now: '+j.color"
       "+(j.breathing?' (breathing)':' (solid)');"
       "draw(j.hist);}catch(e){}}"
       "upd();setInterval(upd,2000);</script>";

  s += "</body></html>";
  return s;
}

// 数据接口：返回 JSON（含历史数组）
void handleApi() {
  String j = "{";
  j += "\"uptime\":"    + String((millis() - bootMs) / 1000);
  j += ",\"heap\":"     + String(ESP.getFreeHeap());
  j += ",\"rssi\":"     + String(WiFi.RSSI());
  j += ",\"temp\":"     + String(temperatureRead(), 1);
  j += ",\"color\":\""  + String(NAMES[colorIdx]) + "\"";
  j += ",\"breathing\":" + String(breathing ? 1 : 0);

  j += ",\"hist\":[";
  for (int i = 0; i < histCount; i++) {          // 从最老的点写到最新的点
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
  Serial.println("\nDay 8: temperature chart");

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
    Serial.println("WiFi FAILED");
  }

  server.on("/",      []() { server.send(200, "text/html; charset=utf-8", page()); });
  server.on("/api",   handleApi);
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

  // 每 2 秒采一个温度点（非阻塞，不耽误别的事）
  if (millis() - lastSample >= SAMPLE_MS) {
    lastSample = millis();
    hist[histPos] = temperatureRead();
    histPos = (histPos + 1) % HIST_N;           // 写满就回到开头覆盖，这就是"环形"
    if (histCount < HIST_N) histCount++;
    Serial.print("temp = ");
    Serial.println(hist[(histPos - 1 + HIST_N) % HIST_N], 1);
  }

  if (breathing && millis() - lastBreath >= BREATH_INTERVAL) {
    lastBreath = millis();
    level += step;
    if (level >= MAX_LEVEL) { level = MAX_LEVEL; step = -1; }
    if (level <= 0) { level = 0; step = 1; colorIdx = (colorIdx + 1) % 6; }
    applyLevel();
  }
}
