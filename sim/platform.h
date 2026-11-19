/*
 * OpenLA132 仿真变体 —— 平台抽象层
 *
 * 仿真器地址路由（openla132 Verilator）：
 *   0x9fe00000        console 打印口（字节写入即输出到宿主 stdout）
 *   0xbfe00000+4*n    16550 UART 模型（官方 LA32 RT-Thread 用的 MMIO 窗口，
 *                     寄存器按 4 字节步进；经 AXI 模型 rt_mmio_offset 映射）
 *   0xbfe1f200        LED 寄存器（4 字节，RT_LED_OFFSET = 0x1f200）
 *   0x1c000000        SRAM（代码 + 数据）
 *
 * 注意：不要用 0x1fe00000 物理地址直访——实测（VCD 抓 AXI）该窗口的
 * 部分写事务会被核心吞掉，console 与 0xbfe00000 窗口才是可靠通路。
 */
#ifndef SIM_PLATFORM_H
#define SIM_PLATFORM_H

#define CONSOLE_BASE       0x9fe00000u
#define UART0_BASE         0xbfe00000u
#define UART_REG_STRIDE    4u          /* 仿真模型寄存器按字步进 */
#define LED_REG            0xbfe1f200u
#define SIM_EXIT_GOOD      0x1fe1f300u /* sim flag 区 */

/* 16550 寄存器号 */
#define UART_DAT 0
#define UART_IER 1
#define UART_LCR 3
#define UART_LSR 5
#define UART_LCR_DLAB  0x80u
#define UART_LCR_8N1   0x03u
#define UART_LSR_THRE  0x20u

#define reg_write(n, v) (*(volatile unsigned char *)(UART0_BASE + (n) * UART_REG_STRIDE) = (v))
#define reg_read(n)     (*(volatile unsigned char *)(UART0_BASE + (n) * UART_REG_STRIDE))

static inline void console_putc(char c)
{
    *(volatile unsigned char *)CONSOLE_BASE = (unsigned char)c;
}

static inline void led_hw_write(unsigned v)
{
    *(volatile unsigned char *)LED_REG = (unsigned char)v;
}

#endif
