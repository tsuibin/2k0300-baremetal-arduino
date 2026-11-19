/*
 * wiring.h — Arduino 数字 IO / 时间 API 的底层声明
 * 实现见 wiring.c，底层调用 src/gpio.c 的裸机驱动
 */
#ifndef WIRING_H
#define WIRING_H

#include <stdint.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 数字电平与引脚模式（ArduinoCore-API 约定，由 wiring 层定义） */
#define LOW           0
#define HIGH          1
#define INPUT         0
#define OUTPUT        1
#define INPUT_PULLUP  2   /* TODO: 2K0300 GPIO 无内置上拉，暂映射为 INPUT */

/* 板级定义：蜂鸟板板载 LED（GPIO85，低电平点亮），wiring 层处理极性反相 */
#define LED_BUILTIN   LED_GPIO

void     pinMode(uint32_t pin, uint32_t mode);
void     digitalWrite(uint32_t pin, uint32_t val);
int      digitalRead(uint32_t pin);

void     delay(uint32_t ms);
void     delayMicroseconds(uint32_t us);
uint32_t millis(void);
uint32_t micros(void);

#ifdef __cplusplus
}
#endif

#endif /* WIRING_H */
