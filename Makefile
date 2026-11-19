# 2K0300 bare-metal (LoongArch64 / LA264)
# 默认用 linux-gnu 交叉链 + freestanding；如有 loongarch64-unknown-elf- 裸机链，改 CROSS 即可
CROSS   ?= loongarch64-linux-gnu-
CC      = $(CROSS)gcc
OBJDUMP = $(CROSS)objdump
OBJCOPY = $(CROSS)objcopy

# -mno-relax 需要 gcc-10+；官方 gcc-8.3 工具链不支持，勿加
ARCHFLAGS = -march=la464 -mabi=lp64d -mcmodel=normal
CFLAGS    = $(ARCHFLAGS) -ffreestanding -nostdlib -fno-builtin -O2 -Wall \
            -Iinclude -Iinclude
ASFLAGS   = $(ARCHFLAGS)
LDFLAGS   = -T ld/2k0300-flash.ld -nostdlib -static -Wl,--gc-sections

OBJS = src/start.o src/main.o src/uart.o src/gpio.o

all: blink.elf blink.bin

blink.elf: $(OBJS) ld/2k0300-flash.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

blink.bin: blink.elf
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

dump: blink.elf
	$(OBJDUMP) -d $<

clean:
	rm -f $(OBJS) blink.elf blink.bin

.PHONY: all clean dump
