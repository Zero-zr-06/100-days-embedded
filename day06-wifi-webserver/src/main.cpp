#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "wifi_secrets.h"      // 账号密码单独放，不进 Git

#define RGB_PIN 48
#define LED_PIN 48

const char* WIFI_SSID = WIFI_SSID_NAME;
const char* WIFI_PASS = WIFI_PASSWORD;

WebServer server(80);

static const uint8_t MAX_LEVEL = 40;
const uint8_t COLORS[6][3] = {
  {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
  {1, 1, 0}, {0, 1, 1}, {1, 0, 1}
};
const char* NAMES[6] = {"Red", "Green", "Blue", "Yellow", "Cyan", "Purple"};

int  colorIdx  = 0;
bool breathing = true;
int  level = 0, step = 1;
unsigned long lastBreath = 0;
const int BREATH_INTERVAL = 15;

unsigned long hitCount = 0;          // 累计被访问次数
unsigned long lastStatus = 0;        // 上次打印状态的时间

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
  s += "<title>ESP32-S3 Light</title></head><body style='font-family:sans-serif'>";
  s += "<h2>ESP32-S3 RGB Control</h2>";
  for (int i = 0; i < 6; i++) {
    s += "<a href='/color?i=" + String(i) + "'>";
    s += "<button style='font-size:18px;padding:12px 20px;margin:5px'>" + String(NAMES[i]) + "</button></a>";
  }
  s += "<br><br>";
  s += "<a href='/breath?on=1'><button style='font-size:18px;padding:12px 20px;margin:5px'>Breath ON</button></a>";
  s += "<a href='/breath?on=0'><button style='font-size:18px;padding:12px 20px;margin:5px'>Breath OFF</button></a>";
  s += "<a href='/off'><button style='font-size:18px;padding:12px 20px;margin:5px'>OFF</button></a>";
  s += "<p>Now: " + String(NAMES[colorIdx]) + (breathing ? " (breathing)" : " (solid)") + "</p>";
  s += "</body></html>";
  return s;
}

void sendHome() {
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nDay 6: WiFi + WebServer");

  // AP + STA 双模：STA 连路由器（有外网，手机/电脑都待得住），AP 作为兜底直连通道
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP32-S3", "12345678", 1, 0, 4);   // 名称/密码/信道1/不隐藏/最多4个设备
  Serial.print("AP ready -> http://");
  Serial.println(WiFi.softAPIP());               // 192.168.4.1

  // 连路由器：这个网有外网，所以手机和电脑都愿意待在上面
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("connecting to ");
  Serial.println(WIFI_SSID);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print(">>> 用这个地址访问: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("路由器没连上（密码/5G频段），只能用上面的 AP 地址");
  }

  // 有设备连进来时，串口立刻报它的 IP
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
      Serial.print(">>> 有设备连上热点了! ");
      Serial.println(WiFi.softAPgetStationNum());
    }
  });

  server.on("/", []() {
    hitCount++;
    Serial.print(">>> 有人访问了, 来自 ");
    Serial.println(server.client().remoteIP());
    server.send(200, "text/html; charset=utf-8", page());
  });
  server.onNotFound([]() {
    Serial.print(">>> 404 请求: ");
    Serial.println(server.uri());
    server.send(404, "text/plain", "not found");
  });
  server.on("/color", []() {
    if (server.hasArg("i")) {
      colorIdx = server.arg("i").toInt() % 6;
      breathing = true; level = 0; step = 1;
      applyLevel();
      Serial.printf("web -> color %d\n", colorIdx);
    }
    sendHome();
  });
  server.on("/breath", []() {
    if (server.hasArg("on")) {
      breathing = server.arg("on").toInt() == 1;
      applyLevel();
      Serial.println(breathing ? "web -> breath on" : "web -> breath off");
    }
    sendHome();
  });
  server.on("/off", []() {
    breathing = false;
    applyLevel();
    Serial.println("web -> off");
    sendHome();
  });
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();                    // 接待手机请求（必须一直调用）

  // 每 5 秒汇报一次：有几个设备连着热点、被访问过几次
  if (millis() - lastStatus >= 5000) {
    lastStatus = millis();
    Serial.printf("status: AP设备数=%d, 累计访问=%lu, STA=%s\n",
                  WiFi.softAPgetStationNum(), hitCount,
                  WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString().c_str() : "未连");
  }

  if (breathing && millis() - lastBreath >= BREATH_INTERVAL) {   // 呼吸，非阻塞
    lastBreath = millis();
    level += step;
    if (level >= MAX_LEVEL) { level = MAX_LEVEL; step = -1; }
    if (level <= 0) { level = 0; step = 1; colorIdx = (colorIdx + 1) % 6; }
    applyLevel();
  }
}
