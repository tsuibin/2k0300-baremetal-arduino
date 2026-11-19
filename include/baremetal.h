#ifndef BAREMETAL_H
#define BAREMETAL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* uart.c */
void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);

/* gpio.c */
void gpio_set_output(int pin);
void gpio_set_input(int pin);
void gpio_write(int pin, int val);
int  gpio_read(int pin);

/* main.c */
void delay(volatile unsigned int loops);

#ifdef __cplusplus
}
#endif

#endif
