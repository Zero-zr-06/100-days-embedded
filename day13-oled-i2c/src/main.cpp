#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2C_SDA 8
#define I2C_SCL 9

Adafruit_SSD1306 oled(128, 64, &Wire, -1);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nDay 13: OLED");
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {   // 扫到 0x3D 就改成 0x3D
    Serial.println("SSD1306 init failed");
    for (;;) delay(1000);
  }
  Serial.println("OLED ready");
}

void loop() {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println("ESP32-S3 OK");
  oled.printf("up  %lu s\n", millis() / 1000);
  oled.printf("temp %.1f C\n", temperatureRead());
  oled.setCursor(0, 48);
  oled.print("Day 13 - 100 days");
  oled.display();
  delay(1000);
}
