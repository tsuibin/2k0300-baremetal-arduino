/*
 * crtmain.c — Arduino sketch 的入口桥
 *
 * start.S 调 main() → 这里：init() 硬件准备 → setup() → 循环 loop()
 * 约定：sketch 里定义 setup()/loop()，不定义 main()
 */
#include "Arduino.h"

extern void setup(void);
extern void loop(void);

void init(void)
{
    Serial.begin(115200);   /* 与 PC 终端约定一致 */
}

int main(void)
{
    init();
    setup();
    for (;;)
        loop();
    return 0;   /* 不可达 */
}
