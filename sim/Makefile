# OpenLA132 仿真变体构建
# 依赖：OpenLACores 仓库（提供链接脚本）、LA32S 工具链、已编译的 emu-mmu0-sram1
#
# 用法（在 OpenLACores 已就位的环境下）:
#   make -C sim OPENLA=/opt/OpenLACores TOOLCHAIN=/opt/la32s/loongarch32s-unknown-linux-gnusf-gcc-16.2.0-ubuntu22.04/bin/loongarch32s-unknown-linux-gnusf-
#   /opt/OpenLACores/verify/verilator/build/emu-mmu0-sram1 --no-diff --image sim/blink.exe

OPENLA   ?= /opt/OpenLACores
CROSS    ?= /opt/la32s/loongarch32s-unknown-linux-gnusf-gcc-16.2.0-ubuntu22.04/bin/loongarch32s-unknown-linux-gnusf-
CC       = $(CROSS)gcc
LD       = $(CROSS)ld
OBJCOPY  = $(CROSS)objcopy

# 与 OpenLACores 官方裸机应用相同的编译约定
CFLAGS  = -static -G0 -pipe -Wa,-mla-global-with-pcrel -fPIC -g -ffreestanding \
          -O0 -fno-builtin -Wall -I.
ENTRY   ?= 0x1C000000   # 与官方 hello/coremark 相同的 SRAM 加载地址

OBJS = start32.o main.o

main.o: main.c platform.h
start32.o: start32.S
test_mmio.o: test_mmio.c platform.h

all: blink.exe blink.bin blink-sram.exe

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

blink-sram.o: main.c
	$(CC) $(CFLAGS) -DSIM_SRAM_ONLY -c -o $@ $<

blink-sram.exe: start32.o blink-sram.o $(OPENLA)/verify/apps/common/linker.script
	$(LD) --defsym=MEM_START=$(ENTRY) -static -G0 -T $(OPENLA)/verify/apps/common/linker.script -Map=link-sram.map \
	      -o $@ start32.o blink-sram.o

blink-sram.bin: blink-sram.exe
	$(OBJCOPY) -O binary --set-section-flags .bss=alloc,load,contents \
	           --set-section-flags .stack=alloc,load,contents \
	           --set-section-flags .dymmy_exit=alloc,load,contents $< $@

blink.exe: $(OBJS) $(OPENLA)/verify/apps/common/linker.script
	$(LD) --defsym=MEM_START=$(ENTRY) -static -G0 -T $(OPENLA)/verify/apps/common/linker.script -Map=link.map \
	      -o $@ $(OBJS)
	$(OBJCOPY) -O binary --set-section-flags .bss=alloc,load,contents \
	           --set-section-flags .stack=alloc,load,contents \
	           --set-section-flags .dymmy_exit=alloc,load,contents $< blink.bin

run: blink.exe
	$(OPENLA)/verify/verilator/build/emu-mmu0-sram1 --no-diff --image blink.exe

clean:
	rm -f $(OBJS) blink.exe blink.bin link.map

.PHONY: all run clean
