# 2K0300 bare-metal (LoongArch64 / LA264)
# 默认用 linux-gnu 交叉链 + freestanding；如有 loongarch64-unknown-elf- 裸机链，改 CROSS 即可
CROSS   ?= loongarch64-linux-gnu-
CC      = $(CROSS)gcc
OBJDUMP = $(CROSS)objdump
OBJCOPY = $(CROSS)objcopy

# -mno-relax 需要 gcc-10+；官方 gcc-8.3 工具链不支持，勿加
ARCHFLAGS = -march=la464 -mabi=lp64d -mcmodel=normal
CFLAGS    = $(ARCHFLAGS) -ffreestanding -nostdlib -fno-builtin -O2 -Wall \
            -Iinclude -Iarduino
CXXFLAGS  = $(CFLAGS) -fno-exceptions -fno-rtti -fno-threadsafe-statics
ASFLAGS   = $(ARCHFLAGS)
LDFLAGS   = -T ld/2k0300-flash.ld -nostdlib -static -Wl,--gc-sections \
            -lgcc

# ---------- 原始裸机 blink（驱动级验证） ----------
OBJS = src/start.o src/main.o src/uart.o src/gpio.o

# ---------- Arduino 核心 + Blink 示例 ----------
ARD_OBJS = src/start.o arduino/wiring.o arduino/Serial.o arduino/crtmain.o \
           examples/Blink/Blink.o src/uart.o src/gpio.o

all: blink.elf blink.bin arduino-blink.elf arduino-blink.bin

blink.elf: $(OBJS) ld/2k0300-flash.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

arduino-blink.elf: $(ARD_OBJS) ld/2k0300-flash.ld
	$(CC) $(CXXFLAGS) $(LDFLAGS) -o $@ $(ARD_OBJS)

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

examples/Blink/Blink.o: examples/Blink/Blink.ino.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

dump: blink.elf
	$(OBJDUMP) -d $<

clean:
	rm -f $(OBJS) $(ARD_OBJS) blink.elf blink.bin arduino-blink.elf arduino-blink.bin

.PHONY: all clean dump
