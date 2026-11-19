/*
 * 2K0300 GPIO 裸机驱动 @0x16104000（bytewise 访问）
 *
 * U-Boot dtsi 定义：
 *   conf_offset = 0x800, out_offset = 0x900, in_offset = 0xa00, bytewise
 * 语义假设：每路 GPIO 各占 1 字节（基址 + 偏移 + 引脚号），值为 0/1。
 * TODO(V5)：对照 U-Boot drivers/gpio 龙芯驱动确认 conf 的位含义后修正。
 */
#include "platform.h"
#include "baremetal.h"

/* GPIO 走 DMW1 非缓存窗口（见 platform.h MMIO 宏说明） */
#define GPIO_REG(off) \
    MMIO8(GPIO_BASE + (unsigned)(off))

void gpio_set_output(int pin)
{
    GPIO_REG(GPIO_CONF_OFF + pin) = 1;
}

void gpio_set_input(int pin)
{
    GPIO_REG(GPIO_CONF_OFF + pin) = 0;
}

void gpio_write(int pin, int val)
{
    GPIO_REG(GPIO_OUT_OFF + pin) = (uint8_t)(val ? 1 : 0);
}

int gpio_read(int pin)
{
    return GPIO_REG(GPIO_IN_OFF + pin) & 1u;
}
