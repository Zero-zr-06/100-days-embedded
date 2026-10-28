#include <Arduino.h>
#define LED_PIN 48   // 板载 WS2812 彩灯在 GPIO48（digitalWrite 点不亮它，仅保留原逻辑）

unsigned long lastTick = 0;
int counter = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  delay(500);
  Serial.println("hello from ESP32-S3");
  Serial.println("serial is alive, COM6 @ 115200");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    Serial.printf("tick %d, uptime %lu ms\n", counter++, millis());
  }
}
