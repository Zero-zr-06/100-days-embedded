#include <Arduino.h>

// Day 4：用板载 BOOT 键控制彩灯
// 短按（< 600ms）：换到下一种颜色
// 长按（>= 600ms）：关灯
// 灯灭着的时候短按 = 重新点亮 + 换色

#define RGB_PIN 48        // 板载 WS2812
#define KEY_PIN 0         // BOOT 键接 GPIO0，按下为低电平
#define LONG_PRESS_MS 600

static const uint8_t MAX_LEVEL = 40;   // 亮度上限，WS2812 很亮

// 红 / 绿 / 蓝 / 黄 / 青 / 紫
const uint8_t COLORS[6][3] = {
  {1, 0, 0},
  {0, 1, 0},
  {0, 0, 1},
  {1, 1, 0},
  {0, 1, 1},
  {1, 0, 1},
};
const char *COLOR_NAMES[6] = {"red", "green", "blue", "yellow", "cyan", "purple"};

int  colorIdx = 0;
bool ledOn    = true;

void showColor() {
  if (!ledOn) {
    neopixelWrite(RGB_PIN, 0, 0, 0);
    return;
  }
  neopixelWrite(RGB_PIN,
                COLORS[colorIdx][0] * MAX_LEVEL,
                COLORS[colorIdx][1] * MAX_LEVEL,
                COLORS[colorIdx][2] * MAX_LEVEL);
}

void setup() {
  Serial.begin(115200);
  pinMode(KEY_PIN, INPUT_PULLUP);     // 内部上拉：不按读到 HIGH，按下读到 LOW
  delay(500);
  Serial.println("Day 4: press BOOT to change color, hold to turn off");
  showColor();
}

void loop() {
  static bool lastState = HIGH;
  static unsigned long pressStart = 0;
  static bool handled = false;        // 这次按压是否已经被"长按"处理掉

  bool state = digitalRead(KEY_PIN);

  // 按下瞬间：记时间
  if (state == LOW && lastState == HIGH) {
    pressStart = millis();
    handled = false;
  }

  // 一直按着超过 600ms：关灯
  if (state == LOW && !handled && millis() - pressStart >= LONG_PRESS_MS) {
    handled = true;
    ledOn = false;
    showColor();
    Serial.println("long press -> LED off");
  }

  // 松手：如果没被长按吃掉，就是短按 -> 换色
  if (state == HIGH && lastState == LOW && !handled) {
    if (millis() - pressStart < LONG_PRESS_MS) {
      colorIdx = (colorIdx + 1) % 6;
      ledOn = true;
      showColor();
      Serial.printf("short press -> color %d (%s)\n", colorIdx, COLOR_NAMES[colorIdx]);
    }
  }

  lastState = state;
  delay(10);                          // 轮询间隔，顺便当消抖
}
