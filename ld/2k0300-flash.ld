/*
 * 2K0300 裸机 XIP 链接脚本
 *
 * FLASH_ORIGIN：SPI Flash 在 CPU 地址空间的 XIP 映射基址（待验证项 V1）。
 *   - 先在数据手册 3.2 节"地址空间分配"中查 Boot 地址；
 *   - 2K1000LA 平台惯例是 0x1c000000，2K0300 需实测确认，勿直接信任。
 * RAM：DDR 起始区域（若 BootROM 未初始化 DDR，见 README V3 回退方案）。
 */
OUTPUT_ARCH(loongarch)
ENTRY(_start)

FLASH_ORIGIN = 0x1c000000;   /* TODO(V1): 以数据手册/实测为准 */
RAM_ORIGIN   = 0x00010000;   /* DDR 低端，避开可能被 BootROM/SPL 使用的区域 */
RAM_SIZE     = 8M;
STACK_TOP    = RAM_ORIGIN + RAM_SIZE;

MEMORY
{
    FLASH (rx)  : ORIGIN = FLASH_ORIGIN, LENGTH = 2M
    RAM   (rwx) : ORIGIN = RAM_ORIGIN,   LENGTH = RAM_SIZE
}

SECTIONS
{
    .text : {
        KEEP(*(.text.start))
        *(.text*)
    } > FLASH

    .rodata : {
        *(.rodata*)
        *(.srodata*)
    } > FLASH

    /* TODO(V3)：若 DDR 未被 BootROM 初始化，.data 初始化需在 start.S 增加
       "从 FLASH 拷贝 .data 到 RAM" 代码段（copy table 方式） */
    .data : ALIGN(8) {
        __data_start = .;
        *(.data*)
        *(.sdata*)
        __data_end = .;
    } > RAM AT> FLASH
    __data_load_start = LOADADDR(.data);

    .bss (NOLOAD) : ALIGN(8) {
        __bss_start = .;
        *(.bss*)
        *(.sbss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    } > RAM

    . = ALIGN(16);
    __heap_start = .;

    /DISCARD/ : { *(.comment) *(.note*) }
}
