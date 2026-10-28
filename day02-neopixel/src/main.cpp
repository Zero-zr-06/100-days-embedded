#include <Arduino.h>

// ESP32-S3-N16R8 开发板上的 WS2812 RGB 彩灯挂在 GPIO48。
// 它不吃 digitalWrite()，必须用 neopixelWrite() 发 800kHz 的专用时序。
#define RGB_PIN 48

// 亮度上限（0-255）。WS2812 非常亮，别怼到 255，40 左右已经很晃眼。
static const uint8_t MAX_LEVEL = 40;

// 彩虹轮：把 0-255 的位置换算成一种纯色，再整体按 brightness 缩放（呼吸用）
void setWheelColor(uint8_t pos, uint8_t brightness) {
  uint8_t r, g, b;
  if (pos < 85) {
    r = 255 - pos * 3;
    g = pos * 3;
    b = 0;
  } else if (pos < 170) {
    r = 0;
    g = 255 - (pos - 85) * 3;
    b = (pos - 85) * 3;
  } else {
    r = (pos - 170) * 3;
    g = 0;
    b = 255 - (pos - 170) * 3;
  }
  r = (uint8_t)((uint16_t)r * brightness / 255);
  g = (uint8_t)((uint16_t)g * brightness / 255);
  b = (uint8_t)((uint16_t)b * brightness / 255);
  neopixelWrite(RGB_PIN, r, g, b);
}

uint8_t colorPos = 0;   // 当前颜色

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Day02: WS2812 breathing light on GPIO48");
}

void loop() {
  // 吸气：亮度 0 -> MAX_LEVEL
  for (int i = 0; i <= 255; i++) {
    setWheelColor(colorPos, (uint8_t)((uint16_t)i * MAX_LEVEL / 255));
    delay(6);
  }
  // 呼气：亮度 MAX_LEVEL -> 0
  for (int i = 255; i >= 0; i--) {
    setWheelColor(colorPos, (uint8_t)((uint16_t)i * MAX_LEVEL / 255));
    delay(6);
  }

  colorPos += 24;   // 每呼吸一轮换一种颜色
  Serial.printf("breath done, color=%d\n", colorPos);
}
