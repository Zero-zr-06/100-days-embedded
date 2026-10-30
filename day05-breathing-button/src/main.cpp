#include <Arduino.h>

#define RGB_PIN 48       // 板载 WS2812
#define KEY_PIN  0       // BOOT 键，按下是低电平
#define LONG_PRESS_MS 600

static const uint8_t MAX_LEVEL = 40;    // 别太亮，晃眼

// 六种呼吸色：红 绿 蓝 黄 青 紫
const uint8_t COLORS[6][3] = {
  {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
  {1, 1, 0}, {0, 1, 1}, {1, 0, 1}
};

int  colorIdx  = 0;
bool breathing = true;

// 呼吸状态：全用变量记，不用 delay
int  level     = 0;      // 当前亮度 0..MAX_LEVEL
int  step      = 1;      // 正=变亮，负=变暗
unsigned long lastBreath = 0;
const int BREATH_INTERVAL = 15;   // 每 15ms 动一格，一轮约 1.2 秒

void applyLevel() {
  if (!breathing) { neopixelWrite(RGB_PIN, 0, 0, 0); return; }
  neopixelWrite(RGB_PIN,
                COLORS[colorIdx][0] * level,
                COLORS[colorIdx][1] * level,
                COLORS[colorIdx][2] * level);
}

void setup() {
  Serial.begin(115200);
  pinMode(KEY_PIN, INPUT_PULLUP);
  delay(500);
  Serial.println("Day 5: short press = next color, long press = pause/resume");
  applyLevel();
}

void loop() {
  // ---------- 任务1：呼吸 ----------
  if (breathing && millis() - lastBreath >= BREATH_INTERVAL) {
    lastBreath = millis();
    level += step;
    if (level >= MAX_LEVEL) { level = MAX_LEVEL; step = -1; }
    if (level <= 0) {
      level = 0;
      step = 1;
      colorIdx = (colorIdx + 1) % 6;   // 呼吸到最暗时，自动换下一种颜色
    }
    applyLevel();
  }

  // ---------- 任务2：按键 ----------
  static bool lastState = HIGH;
  static unsigned long pressStart = 0;
  static bool handled = false;

  bool state = digitalRead(KEY_PIN);

  if (state == LOW && lastState == HIGH) {   // 刚按下的那一瞬间
    pressStart = millis();
    handled = false;
  }

  if (state == LOW && !handled && millis() - pressStart >= LONG_PRESS_MS) {
    handled = true;                          // 按满 0.6 秒 = 长按
    breathing = !breathing;                  // 暂停/恢复
    applyLevel();
    Serial.println(breathing ? "resumed" : "paused");
  }

  if (state == HIGH && lastState == LOW && !handled) {  // 松手且没触发过长按
    colorIdx = (colorIdx + 1) % 6;           // 短按 = 手动换色
    breathing = true;
    level = 0;
    step = 1;
    applyLevel();
    Serial.printf("color -> %d\n", colorIdx);
  }

  lastState = state;
}
