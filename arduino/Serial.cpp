/*
 * Serial.cpp — HardwareSerial 轮询实现（包装 src/uart.c 的 16550 裸机驱动）
 */
#include "Serial.h"
#include "platform.h"
#include "baremetal.h"

HardwareSerial Serial;

void HardwareSerial::begin(unsigned long baud)
{
    (void)baud;   /* 骨架阶段 uart_init 固定 115200；TODO(S1): 按 baud 传参 */
    uart_init();
}

void HardwareSerial::end(void) { /* 轮询模式无资源可释放 */ }

size_t HardwareSerial::write(uint8_t c)
{
    uart_putc((char)c);
    return 1;
}

size_t HardwareSerial::write(const char *s)
{
    size_t n = 0;
    while (*s) {
        uart_putc(*s++);
        n++;
    }
    return n;
}

size_t HardwareSerial::write(const uint8_t *buf, size_t n)
{
    for (size_t i = 0; i < n; i++)
        uart_putc((char)buf[i]);
    return n;
}

int HardwareSerial::available(void)
{
    /* LSR.DR：接收数据就绪 */
    volatile uint8_t *lsr =
        (volatile uint8_t *)(uintptr_t)(MMIO_BASE_UNCACHED | (UART0_BASE + UART_LSR));
    return (*lsr & UART_LSR_DR) ? 1 : 0;
}

int HardwareSerial::read(void)
{
    if (!available())
        return -1;
    volatile uint8_t *dat =
        (volatile uint8_t *)(uintptr_t)(MMIO_BASE_UNCACHED | (UART0_BASE + UART_DAT));
    return *dat;
}

void HardwareSerial::flush(void)
{
    /* 等 LSR.TEMT（发送器全空） */
    volatile uint8_t *lsr =
        (volatile uint8_t *)(uintptr_t)(MMIO_BASE_UNCACHED | (UART0_BASE + UART_LSR));
    while (!(*lsr & 0x40u))
        ;
}

void HardwareSerial::print(const char *s) { write(s); }

void HardwareSerial::print(unsigned long n, int base)
{
    char buf[34];
    int i = 0;
    if (base == 16) {
        const char *hex = "0123456789abcdef";
        do { buf[i++] = hex[n & 0xf]; n >>= 4; } while (n);
    } else {                    /* 10 进制 */
        do { buf[i++] = '0' + (char)(n % 10); n /= 10; } while (n);
    }
    while (i) write((uint8_t)buf[--i]);
}

void HardwareSerial::println(void)            { write("\r\n"); }
void HardwareSerial::println(const char *s)   { print(s); println(); }

void HardwareSerial::println(unsigned long n, int base)
{
    print(n, base);
    println();
}
