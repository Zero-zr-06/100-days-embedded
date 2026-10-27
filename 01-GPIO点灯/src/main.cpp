#include <Arduino.h>

// ESP32-S3-DevKitC-1 板载那颗 RGB 灯接在 GPIO48
// 如果你的板子不是这个型号，或者灯没反应，看文件最下面的说明
#define RGB_PIN 48

void setup() {
  Serial.begin(115200);
  delay(1500);                       // 等串口连上，不然前面几句打印会丢
  Serial.println("");
  Serial.println("Day 1: ESP32-S3 起来了");
}

void loop() {
  // 亮蓝色，0-255
  neopixelWrite(RGB_PIN, 0, 0, 80);
  Serial.println("LED ON");
  delay(500);

  neopixelWrite(RGB_PIN, 0, 0, 0);
  Serial.println("LED OFF");
  delay(500);
}

/*
 troubleshooting：
 1) 灯不亮，但串口有打印 -> 你的板子 RGB 灯不在 GPIO48。
    把 RGB_PIN 改成 2 或 13 试试，或者查一下你那块板的原理图。
 2) 串口没输出 -> 确认用的是板子上标着 "USB" 的那个口（不是 UART 口），
    并且 platformio.ini 里的 ARDUINO_USB_CDC_ON_BOOT=1 加上了。
 3) 想接一个普通 LED：LED 长脚串一个 220 欧电阻接 GPIO2，短脚接 GND，
    然后把 RGB_PIN 改成 2，neopixelWrite 换成 digitalWrite(pin, HIGH/LOW)。
*/
