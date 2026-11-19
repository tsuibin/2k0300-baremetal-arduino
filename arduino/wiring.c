/*
 * wiring.c — Arduino 数字 IO / 时间 API 的裸机实现（2K0300）
 *
 * 数字 IO：直接映射 src/gpio.c 驱动（bytewise 寄存器，语义待 V5 板上确认）
 * 时间：
 *   - delay/delayMicroseconds：忙等（骨架阶段够用）
 *   - millis/micros：LoongArch64 稳定计数器 rdcnt.id/rdcnt.vl
 *     TODO(T1)：CC_FREQ（计数器频率）需按 2K0300 手册/PLL 配置校准，
 *     当前按晶振 100MHz 假设；板上用 blink 秒级节拍实测校准
 */
#include "wiring.h"
#include "platform.h"
#include "baremetal.h"

/* ---------- 数字 IO ---------- */

void pinMode(uint32_t pin, uint32_t mode)
{
    if (mode == OUTPUT)
        gpio_set_output((int)pin);
    else
        gpio_set_input((int)pin);   /* INPUT 与 INPUT_PULLUP 暂同 */
}

/* 注意：蜂鸟板 LED 低电平点亮——Arduino 约定 HIGH=亮，由 variant 层处理极性 */
void digitalWrite(uint32_t pin, uint32_t val)
{
    uint32_t level = val;

    if (pin == LED_BUILTIN)
        level = val ? 0u : 1u;      /* 低电平点亮的板载 LED 反相 */
    gpio_write((int)pin, (int)level);
}

int digitalRead(uint32_t pin)
{
    return gpio_read((int)pin);
}

/* ---------- 时间 ---------- */

/* 稳定计数器：rdtime.d 一次读 64 位（官方写法，无高低位回绕问题） */
static inline uint64_t counter64(void)
{
    uint64_t v;
    __asm__ volatile("rdtime.d %0, $zero" : "=r"(v));
    return v;
}

/* TODO(T1)：计数器频率占位。LA264 复位后 CC_FREQ 默认值需查手册 2.4 节 */
#define COUNTER_FREQ_HZ     100000000u   /* 假设 100MHz */

uint64_t arduino_uptime_us(void)
{
    return counter64() / (COUNTER_FREQ_HZ / 1000000u);
}

uint32_t micros(void)
{
    return (uint32_t)arduino_uptime_us();
}

uint32_t millis(void)
{
    return (uint32_t)(arduino_uptime_us() / 1000u);
}

void delayMicroseconds(uint32_t us)
{
    uint64_t target = arduino_uptime_us() + us;
    while (arduino_uptime_us() < target)
        ;
}

void delay(uint32_t ms)
{
    /* 分段避免 us 值溢出 32 位 */
    while (ms > 1000u) {
        delayMicroseconds(1000000u);
        ms -= 1000u;
    }
    delayMicroseconds(ms * 1000u);
}
