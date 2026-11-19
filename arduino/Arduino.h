/*
 * Arduino.h — 2K0300 裸机 Arduino 核心的对外头文件
 *
 * 目标：让 sketch 用标准 Arduino 语法（setup/loop/digitalWrite/...）
 * 在 2K0300 上运行。命名与语义对齐 ArduinoCore-API。
 */
#ifndef ARDUINO_2K0300_H
#define ARDUINO_2K0300_H

#include <stdint.h>
#include "wiring.h"
#include "Serial.h"

/* 数字电平与引脚模式定义见 wiring.h（wiring 层拥有） */

/* 骨架阶段直接用 GPIO 编号作为 Arduino 引脚号；
   后续 variant_2k0300 将提供排针映射表（PA/PB 名字、pinmux 复用）
   LED_BUILTIN 定义见 wiring.h（板级常量） */

/* 系统初始化（由 crtmain 调用，sketch 作者无需关心） */
void init(void);

#endif /* ARDUINO_2K0300_H */
