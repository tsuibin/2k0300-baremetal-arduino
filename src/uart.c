/* 16550 兼容 UART0 裸机驱动 @0x16100000, APB 200MHz */
#include "platform.h"
#include "baremetal.h"

#define uart ((volatile uint8_t *)(uintptr_t)(MMIO_BASE_UNCACHED | UART0_BASE))

static inline void reg_write(unsigned off, uint8_t v) { uart[off] = v; }
static inline uint8_t reg_read(unsigned off)          { return uart[off]; }

void uart_init(void)
{
    /* 分频 = APB / 16 / baud = 200e6 / 16 / 115200 ≈ 108 */
    const uint16_t div = APB_CLK_HZ / 16u / 115200u;

    reg_write(UART_IER, 0x00);                  /* 关中断 */
    reg_write(UART_LCR, UART_LCR_DLAB);         /* 进 DLAB */
    reg_write(UART_DLL, div & 0xff);
    reg_write(UART_DLH, (div >> 8) & 0xff);
    reg_write(UART_LCR, UART_LCR_8N1);          /* 8N1, DLAB=0 */
    reg_write(UART_FCR, 0x07);                  /* 使能并清 FIFO */
    reg_write(UART_MCR, 0x03);                  /* DTR/RTS */
}

void uart_putc(char c)
{
    while (!(reg_read(UART_LSR) & UART_LSR_THRE))
        ;
    reg_write(UART_DAT, (uint8_t)c);
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}
