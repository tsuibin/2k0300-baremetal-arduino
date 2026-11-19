/*
 * 仿真变体入口：UART + LED 闪烁
 * 验证目标：start32.S 启动流程、UART 16550 驱动逻辑、Blink 主循环
 *
 * SIM_SRAM_ONLY：SRAM-only 仿真器（emu-mmu0-sram1）没有 AXI MMIO 通路，
 * UART 输出改走 console 口，LED 改为打印符号。
 * 默认构建（AXI 仿真器 emu-mmu1-sram0）：走真实 16550 模型 + LED 寄存器。
 */
#include "platform.h"

#ifdef SIM_SRAM_ONLY

void uart_init(void) {}
void uart_putc(char c) { console_putc(c); }
void uart_puts(const char *s) { while (*s) console_putc(*s++); }
/* SRAM-only 下无 AXI 通路，LED 改为打印符号 */
static void led_write(unsigned v) { console_putc(v ? '[' : ']'); }

#else

void uart_init(void)
{
    reg_write(UART_IER, 0x00);
    reg_write(UART_LCR, UART_LCR_DLAB);
    reg_write(UART_DAT, 108);            /* DLL */
    reg_write(UART_IER, 0x00);           /* DLH（寄存器 1 = DLH when DLAB=1） */
    reg_write(UART_LCR, UART_LCR_8N1);
}

void uart_putc(char c)
{
    while (!(reg_read(UART_LSR) & UART_LSR_THRE))
        ;
    reg_write(UART_DAT, (unsigned char)c);
}

void uart_puts(const char *s)
{
    while (*s)
        uart_putc(*s++);
}

static void led_write(unsigned v) { led_hw_write(v); }

#endif /* SIM_SRAM_ONLY */

void delay(volatile unsigned int loops)
{
    while (loops--)
        ;
}

int main(void)
{
    uart_init();
    uart_puts("\n== 2K0300 skeleton @ OpenLA132 sim ==\n");
#ifdef SIM_SRAM_ONLY
    uart_puts("SRAM-only mode: console output, LED = [ ].\n");
#else
    uart_puts("UART(16550) driver OK, entering blink loop.\n");
#endif

    for (int i = 0; i < 3; i++) {
        uart_puts("LED on\n");
        led_write(1);
        delay(200000);

        uart_puts("LED off\n");
        led_write(0);
        delay(200000);
    }

    uart_puts("blink sim passed.\n");
    return 0;
}
