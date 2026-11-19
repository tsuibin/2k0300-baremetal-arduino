/*
 * 2K0300 bare-metal platform register definitions
 * 来源：open-loongarch/u-boot arch/loongarch/dts/ls2k300.dtsi（官方 BSP）
 */
#ifndef PLATFORM_H
#define PLATFORM_H

/* ---------- 时钟 ---------- */
#define OSC_CLK_HZ          100000000u   /* 晶振 100MHz */
#define APB_CLK_HZ          200000000u   /* APB 外设时钟 200MHz（U-Boot fixed-clock） */

/* ---------- 总线段 ---------- */
#define LS2K0300_FB_BASE    0x0d000000u  /* 显存 32MB */

/* ---------- UART0：16550 兼容 ---------- */
#define UART0_BASE          0x16100000u
#define UART0_REG_SHIFT     0            /* 寄存器间隔 1 字节 */

/* 16550 寄存器偏移（DLAB=0/1） */
#define UART_DAT            0x00
#define UART_IER            0x01
#define UART_IIR            0x02
#define UART_FCR            0x02
#define UART_LCR            0x03
#define UART_MCR            0x04
#define UART_LSR            0x05
#define UART_MSR            0x06
#define UART_DLL            0x00         /* DLAB=1 */
#define UART_DLH            0x01         /* DLAB=1 */
#define UART_LCR_DLAB       0x80u
#define UART_LCR_8N1        0x03u
#define UART_LSR_THRE       0x20u
#define UART_LSR_DR         0x01u

/* ---------- GPIO：bytewise 访问 ---------- */
#define GPIO_BASE           0x16104000u
#define GPIO_CONF_OFF       0x800u       /* 方向/复用？见 README 验证项 V5 */
#define GPIO_OUT_OFF        0x900u
#define GPIO_IN_OFF         0xa00u
#define GPIO_NPINS          106

/* 蜂鸟板板载 LED：GPIO85，低电平点亮 */
#define LED_GPIO            85

/* ---------- pinmux ---------- */
#define PINMUX_BASE         0x16000490u
#define PINMUX_SIZE         0x20u

/* ---------- SPI0（启动 Flash） ---------- */
#define SPI0_BASE           0x16010000u

/* ---------- I2C ---------- */
#define I2C0_BASE           0x16108000u
#define I2C1_BASE           0x16109000u
#define I2C2_BASE           0x1610a000u
#define I2C3_BASE           0x1610b000u

/* ---------- 中断 ---------- */
#define EXTIOI_BASE         0x16001400u  /* 128 向量，级联 CPU 中断 3 */
#define IRQ_UART0           2

/* ---------- PMC ---------- */
#define PMC_BASE            0x16124000u
#define PMC_RESET_OFF       0x0u         /* 写 1 软复位 */

#endif /* PLATFORM_H */
