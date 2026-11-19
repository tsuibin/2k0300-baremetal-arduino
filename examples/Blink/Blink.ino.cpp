/*
 * Blink — 第一个 Arduino sketch（2K0300 裸机）
 *
 * 板载 LED（蜂鸟板 GPIO85，低电平点亮，Arduino.h 已做极性处理）
 * Serial 波特率 115200 8N1
 */
#include "Arduino.h"

void setup(void)
{
    Serial.println("2K0300 Arduino core - Blink");
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop(void)
{
    Serial.println("LED on");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    Serial.println("LED off");
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
