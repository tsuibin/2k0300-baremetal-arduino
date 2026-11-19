/*
 * 裸机入口：UART 打印 + LED 闪烁
 * 硬件假设：蜂鸟板（GPIO85 板载 LED，低电平点亮）
 */
#include "platform.h"
#include "baremetal.h"

void delay(volatile unsigned int loops)
{
    while (loops--)
        ;
}

int main(void)
{
    uart_init();
    uart_puts("\n== 2K0300 bare-metal skeleton ==\n");
    uart_puts("If you can read this, UART0 @0x16100000 works.\n");

    gpio_set_output(LED_GPIO);

    while (1) {
        uart_puts("LED on\n");
        gpio_write(LED_GPIO, 0);        /* 低电平点亮 */
        delay(2000000);

        uart_puts("LED off\n");
        gpio_write(LED_GPIO, 1);
        delay(2000000);
    }
    return 0;
}
