# 本机（x86 沙箱）仿真验证流水线

目的：在拿到 2K0300 真板之前，用龙芯官方 **OpenLACores**（OpenLA132 = LA32S
开源核）的 Verilator 仿真器验证裸机骨架的启动流程、UART 驱动逻辑和 Blink
主循环。LA264（2K0300）与 OpenLA132 同属 LoongArch，CSR/启动序列的官方
"已验证写法" 直接复用到板级代码中。

## 依赖（已在本机装好）

| 组件 | 版本/路径 |
|---|---|
| Verilator | 5.020 (apt) |
| OpenLACores | /opt/OpenLACores（含 verify/loong-emu 子克隆） |
| 仿真器 | /opt/OpenLACores/verify/verilator/build/emu-mmu0-sram1（CONFIG_MMU=0 CONFIG_SRAM=1） |
| DiffTest 参考模型 | /opt/OpenLACores/verify/loong-emu/build/la_emu_ref.so（--no-diff 模式不加载） |
| LA32S 工具链 | /opt/la32s/loongarch32s-unknown-linux-gnusf-gcc-16.2.0-ubuntu22.04（GCC 16.2.0） |

## 仿真器地址路由（实测修订）

| 地址 | 用途 | 状态 |
|---|---|---|
| `0x9fe00000` | console 打印口（写字节 → 宿主 stdout） | ✅ 可靠 |
| `0xbfe00000 + 4n` | 16550 UART 模型（官方 LA32 RT-Thread 的 MMIO 窗口，寄存器 4 字节步进） | ✅ 可靠 |
| `0xbfe1f200` | LED 寄存器（RT_LED_OFFSET = 0x1f200） | ✅ 可靠 |
| `0x1fe00000` 起 | 物理 RT-MMIO 窗口 | ⚠️ 部分写事务会被核心吸收（VCD 抓 AXI 实测：LCR 写可达、DAT 写丢失），勿用 |
| `0x1c000000` | SRAM（ELF 加载基址） | ✅ |

## 文件

- `start32.S` — LA32S 启动代码。CSR 序列（DMW0/DMW1/CRMD/BTB/ERA+ertn）
  照搬官方 crt/start.S 的已验证写法，单 hart 简化，增加 bss 清零，
  `__enter_main` 处把 CRMD.DATF/DATM 设为强序非缓存（0xA0）。
- `platform.h` — 仿真器地址路由定义（UART0_BASE=0xbfe00000）。
- `main.c` — UART 初始化 + 3 次 LED 闪烁 + 打印；`SIM_SRAM_ONLY` 宏用于
  SRAM-only 仿真器（console 输出、打印符号代替 LED）。
- `test_mmio.c` — MMIO 通路对照实验（console vs 16550 vs LSR 读 vs LED）。
- `Makefile` — 用官方 `linker.script`（MEM_START=0x1C000000）链接，
  产物 `blink.exe`（AXI 全功能）与 `blink-sram.exe`（SRAM-only）。

## 运行

```bash
make -C sim
# 全功能（16550 UART + LED 寄存器），AXI 模式：
/opt/OpenLACores/verify/verilator/build/emu-mmu1-sram0 --no-diff -i sim/blink.exe
# 仅启动流程验证（console 输出），SRAM 模式：
/opt/OpenLACores/verify/verilator/build/emu-mmu0-sram1 --no-diff -i sim/blink-sram.exe
```

## 已知差异（仿真 vs 2K0300 真板）

1. UART 寄存器步进：仿真模型 4 字节、真板 1 字节 → `platform.h` 的
   `UART_REG_STRIDE` 是仿真专属参数，板级代码为 1。
2. UART/LED 地址：真板为 0x16100000 / 0x16104000，仿真为 0xbfe00000 /
   0xbfe1f200。
3. 启动方式：仿真器直接加载 ELF 到 SRAM；真板需经 BootROM/SPI Flash XIP。
   **被验证的是启动代码逻辑（CSR 序列、栈、bss）、驱动逻辑与主循环。**

## 验证结果（2026-11-20，本机 x86 沙箱）

```
--- SRAM 模式（emu-mmu0-sram1, blink-sram.exe）---
== 2K0300 skeleton @ OpenLA132 sim ==
SRAM-only mode: console output, LED = [ ].
LED on / off ×3
blink sim passed.
[sim] GOOD TRAP at pc=0x1c0000e8  cycles=7205627 commits=4803583

--- AXI 模式（emu-mmu1-sram0, blink.exe，真实 16550 驱动 + LED 寄存器）---
== 2K0300 skeleton @ OpenLA132 sim ==
UART(16550) driver OK, entering blink loop.
LED on / off ×3（经 16550 DAT 寄存器逐字节输出）
blink sim passed.
[sim] GOOD TRAP at pc=0x1c0000f0  cycles=8408582 commits=4804484
```

### 调试中踩过的坑（重要经验）

1. **0x1fe00000 物理窗口的 DAT 写丢失**：VCD 抓 AXI 发现 LCR 写（0x1fe0000c）
   可达而连续的 DAT 写（0x1fe00000）未产生 AXI 事务；换官方
   0xbfe00000 虚拟窗口后完全正常。LA32 裸机访问仿真外设一律走
   console(0x9fe00000) / RT-MMIO(0xbfe00000) 两个 VA 窗口。
2. **CRMD 初始状态**：官方 crt 的 `csrwr 0x10, 0x0` 后经 ertn 进入
   DA/PG 状态，CRMD.DATM 默认可缓存；启动后显式设 CRMD=0xA0
   （DATF/DATM=强序非缓存）保证 MMIO 写直达 AXI。
3. **make 不跟踪头文件依赖**：改 platform.h 后必须 rm main.o 强制重建。
