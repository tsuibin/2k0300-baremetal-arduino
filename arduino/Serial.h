/*
 * Serial.h — HardwareSerial 最小集（轮询式，无缓冲，无中断）
 * 对齐 Arduino Print 语义：print/println/available/read/write/flush
 */
#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stddef.h>

class HardwareSerial {
public:
    void begin(unsigned long baud);
    void end(void);

    size_t write(uint8_t c);
    size_t write(const char *s);
    size_t write(const uint8_t *buf, size_t n);

    int  available(void);
    int  read(void);
    void flush(void);

    /* Print 子集 */
    void print(const char *s);
    void print(unsigned long n, int base = 10);
    void println(void);
    void println(const char *s);
    void println(unsigned long n, int base = 10);
};

extern HardwareSerial Serial;

#endif /* SERIAL_H */
