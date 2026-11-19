# 2K0300 完全独立裸机（SPI Flash 自启动）技术方案与验证骨架

目标：让 2K0300 上电后**不经过 Linux**，直接从 SPI Flash 自启动运行最小裸机固件
（UART 打印 + LED 点灯），为后续把 Arduino 核心层叠在裸机 HAL 之上打基础。

> 文档依据：
> - 《龙芯 2K0300 处理器用户手册 V1.01》（2025-05，loongson.cn 官网可下载）
> - 《龙芯 2K0300 处理器数据手册 V1.01》（SPI Flash 启动映射机制）
> - open-loongarch 官方 U-Boot 仓库（gitee.com/open-loongarch/u-boot）`arch/loongarch/dts/ls2k300.dtsi`（寄存器地址实测来源）

---

## 1. 已确认的关键事实（可直接用于编码）

### 1.1 寄存器基地址（来自官方 U-Boot 设备树，可信度高）

| 外设 | 基地址 | 说明 |
|---|---|---|
| UART0（调试串口） | `0x16100000` | `ns16550a` 兼容，APB 时钟 200 MHz，中断号 2 |
| GPIO 控制器 | `0x16104000` | 106 路，`bytewise` 访问；conf=+0x800 / out=+0x900 / in=+0xa00 |
| SPI0（启动 Flash 所在） | `0x16010000` | 数据手册：Flash 映射为只读 memory 空间，"复位后不需要软件干预就可以直接访问" |
| SPI1 | `0x16018000` | |
| I2C0~3 | `0x16108000` 起，步进 0x1000 | 主从模式 |
| 引脚复用 pinmux | `0x16000490`（长 0x20） | pinctrl-single，bit-per-mux，2bit/脚 |
| EXTIOI 中断控制器 | `0x16001400` | 128 个向量，级联 CPU 中断 3 |
| PMC（复位/关机） | `0x16124000` | offset 0 写 1 复位 |
| eMMC0 / SDIO1 | `0x16140000` / `0x16148000` | |
| USB2.0 EHCI/OHCI | `0x16080000` / `0x16088000` | |
| DC 显示控制器 | `0x16090000`（FB `0x0d000000`，32MB） | |

时钟：晶振 100 MHz，boot/APB 时钟 200 MHz（U-Boot 固定时钟定义）。
UART0 115200 波特率分频 = 200MHz / 16 / 115200 ≈ **108**。

### 1.2 启动机制（数据手册结论）

> "SPI0/1 控制器除了若干 IO 寄存器外，还有一段映射到 SPI Flash 的只读 memory
> 空间。如果将这段 memory 空间分配在 BOOT 启动地址，**复位后不需要软件干预就
> 可以直接访问，从而支持处理器从 SPI0-Flash 启动**。"

含义：CPU 复位向量指向 SPI Flash 的 XIP 映射区，**镜像放在 Flash 偏移 0 处
即可被直接取指执行**（与 STM32 类似，不是"先拷贝到 RAM 再跳"的传统 ARM BootROM 模式）。

### 1.3 待验证项（本骨架的核心目的）

| 编号 | 未知项 | 验证方法 |
|---|---|---|
| V1 | CPU 复位后 PC 的确切取指地址（Flash XIP 映射基址） | 读数据手册 3.2 节地址空间分配表；烧一个最小跳转程序实验 |
| V2 | BootROM 是否校验 Flash 前 N 字节（社区文章称有 SM3 签名校验、非官方镜像报 "Signature mismatch"；与数据手册"无需软件干预"矛盾） | 从出厂正常启动的板子用 U-Boot `sf read 0x90000000 0 0x1000; md.b` dump 前 4KB，比对官方 SPL 镜像头部结构 |
| V3 | BootROM/U-Boot SPL 是否已初始化 DDR（决定栈放哪） | XIP 直接把栈指向 DDR 试跑；不通则参考 U-Boot SPL 的 DDR init 序列，或用 L2-Cache-as-RAM 技巧过渡 |
| V4 | 镜像头部格式（若 BootROM 需要头部） | 同 V2 的 dump 对比 + 读 U-Boot SPL 源码打包逻辑 |
| V5 | GPIO bytewise 寄存器语义（conf/out/in 每脚 1 字节？0/1 值域？） | 对照 U-Boot `drivers/gpio` 龙芯 GPIO 驱动源码 + 点灯实验 |

---

## 2. 分步验证计划（先整理、后逐条确认）

| 步骤 | 内容 | 通过标准 |
|---|---|---|
| S0 | 在蜂鸟板上用 U-Boot dump 出厂 SPI Flash 前 4KB，与官方 `u-boot-spl.bin` 头部比对 | 确定 V2/V4（有无头部、字段布局） |
| S1 | 编译本骨架，生成 `blink.bin` | 工具链/链接脚本工作正常 |
| S2 | 用 U-Boot 把 `blink.bin` 烧到 Flash 试验地址，先经 U-Boot `go` 命令运行 | 串口打印 "Hello 2K0300"，LED 闪烁（验证代码正确性，与 BootROM 无关） |
| S3 | 确认 Flash XIP 基址（V1），把镜像按正确偏移烧写 | 断电重新上电，代码直接运行 |
| S4 | 若 S3 失败且确认 BootROM 校验头部 → 按 S0 结论给镜像加头部（tools/mkimage.py） | 自启动成功 |
| S5 | 若 S3 失败且因 DDR 未初始化 → 移植 U-Boot SPL 的 DDR init 到 start.S | 自启动成功 |
| S6 | S3~S5 任一通过后，开始叠 Arduino HAL 层（时基/中断/PWM/ADC/I2C/SPI） | |

> 建议先做 S0 再动手写镜像——它决定 S3/S4/S5 走哪条分支，是整个项目第一个岔路口。

---

## 3. 代码骨架说明

```
src/start.S        LoongArch64 启动汇编：栈、bss 清零、进 main
src/main.c         裸机入口：串口打印 + GPIO85 LED 闪烁（蜂鸟板板载 LED，低电平点亮）
src/uart.c/.h      16550 兼容 UART0 驱动（0x16100000，APB 200MHz）
src/gpio.c/.h      2K0300 GPIO 驱动（bytewise，0x16104000）
include/platform.h 全部寄存器地址常量（与 dtsi 对齐）
ld/2k0300-flash.ld XIP 链接脚本（.text 在 Flash 映射区，.data/.bss/栈在 DDR）
tools/mkimage.py   镜像打包占位脚本（S0 结论出来后填头部字段）
Makefile           loongarch64 bare-metal 交叉编译
```

### 工具链

任选其一：
- 官方裸机工具链 `loongarch64-unknown-elf-`（龙芯 ftp / LoongIDE 配套）
- `loongarch64-linux-gnu-` + `-ffreestanding -nostdlib`（本骨架默认，方便直接用现成的
  gcc-8.3 / gcc-13.3 交叉链）

### 编译

```bash
# 已用官方 gcc-8.3 工具链验证通过（2026-11-20）：
make CROSS=/opt/loongson-gnu-toolchain-8.3-x86_64-loongarch64-linux-gnu-rc1.6/bin/loongarch64-linux-gnu-
# 生成 blink.elf（入口 0x1c000000）/ blink.bin（656B）/ blink.flash.bin（4KB 对齐）
```

> 注意：gcc-8.3 不支持 `-mno-relax`（gcc-10+ 才有），Makefile 已按 8.3 适配；
> 换用 gcc-13 工具链时可直接复用，不需要 `-mno-relax`（骨架代码全为 pc 相对寻址，
> 链接脚本段布局与松弛无关）。

### 烧写与运行（U-Boot 下）

```bash
# 经 U-Boot 先验证代码本身（S2）
tftp 0x90000000 blink.bin; go 0x90000000   # 或用 usb/串口 xmodem 上传

# 烧写 Flash（S3，具体偏移按 S0 结论调整）
sf probe 0
sf erase 0 0x10000
sf write 0x90000000 0 0x10000
reset
```

---

## 4. 风险与回退

1. **BootROM 校验头部（V2/V4）**：若确认必须头部且无法从公开资料推断，
   退路是把它当成"官方 SPL 兼容镜像"——参考 U-Boot SPL 打包脚本伪造合法头部。
2. **DDR 未初始化（V3）**：移植 U-Boot SPL 的 `arch/loongarch/.../ddr.c` 序列
   （约 300~500 行，BSP 源码开放可抄），工作量 +1 周。
3. **XIP 速度慢**：Flash XIP 取指慢，验证阶段够用；正式版可加"XIP 引导段 +
   拷贝自身到 DDR 全速运行"的两段式结构（类似 MCU 的 ITCM loader）。
4. LA264 是 la464 指令集子集，`-march=la464` 编出的代码可直接跑。
