/* MMIO 通路对照实验：console vs 16550 UART 模型 */
#include "platform.h"

void delay(volatile unsigned int loops) { while (loops--) ; }

int main(void)
{
    /* 1. console 口直写 */
    console_putc('C');
    console_putc('O');
    console_putc('N');
    console_putc('S');
    console_putc('\n');

    /* 2. 16550 模型：先设 LCR=8N1（DLAB=0），再写 DAT */
    reg_write(UART_LCR, UART_LCR_8N1);
    reg_write(UART_DAT, 'U');
    reg_write(UART_DAT, 'A');
    reg_write(UART_DAT, 'R');
    reg_write(UART_DAT, 'T');
    reg_write(UART_DAT, '\n');

    /* 3. 读 LSR 验证读通路 */
    unsigned char lsr = reg_read(UART_LSR);
    if (lsr & 0x20) {
        console_putc('L');
        console_putc('S');
        console_putc('R');
        console_putc('\n');
    }

    /* 4. LED 寄存器 */
    led_hw_write(0xA5);

    return 0;
}
