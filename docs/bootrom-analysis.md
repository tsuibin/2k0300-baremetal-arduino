# 2K0300 BootROM / SPI Flash 启动链分析

整理时间：2026-11-20。信息源：官方数据手册/用户手册 + open-loongarch U-Boot 源码 + 社区文章。

## 1. 启动链

```
上电 → PMU 时序 → BootROM → SPI Flash (XIP / SPL?) → U-Boot 2022.04 → Linux
                        ↑ 本项目接管点
```

## 2. 两条相互矛盾的信息（核心待验证项 V2）

| 信息源 | 说法 |
|---|---|
| **数据手册 V1.01 §3.6 SPI**（权威） | "SPI0/1 控制器……有一段映射到 SPI Flash 的只读 memory 空间。如果将这段 memory 空间分配在 BOOT 启动地址，**复位后不需要软件干预就可以直接访问**，从而支持处理器从 SPI0-Flash 启动" → 指向**纯 XIP** 模式，无需特殊头部 |
| 社区文章（嵌云网《2K0300 启动流程解析》，针对久久派） | "BootROM 校验 SPI Flash 前 4KB 签名（国密 SM3），加载 PMON 到 L2 Cache 中运行"；非官方 Flash 报 "Signature mismatch" → 指向**有头部校验 + 拷贝执行**模式 |

判断：两者可能都成立但描述对象不同——BootROM 可能兼容"无头部直接 XIP"和"带头部校验"
两种启动镜像类型（类似很多 SoC 的 boot header 开关），或该文章描述不准确。
**结论：必须以 S0 实验（dump 出厂 Flash 前 4KB + 对照 U-Boot SPL 打包源码）为准。**

## 3. U-Boot 侧证据

- 蜂鸟板官方手册的启动日志：`Trying to boot from BootSpace...` → U-Boot 2022.04 使用
  SPL 机制，说明官方镜像至少有 SPL 段，Flash 布局为 `SPL | U-Boot proper | ...`。
- 官方源码仓库：`https://gitee.com/open-loongarch/u-boot`（`arch/loongarch/dts/ls2k300.dtsi`
  已给出全部寄存器地址）与 `https://gitee.com/open-loongarch/build-2k0300`（一键拉取 BSP）。
- **逆向头部最快的路径**：读 U-Boot 源码 `arch/loongarch/cpu/`、`drivers/mtd/spi/`、
  SPL 链接脚本与打包 Makefile（`tools/`），对照出厂 Flash dump 逐字节比对。

## 4. XIP 映射基址（V1）

2K0300 数据手册 §3.2"地址空间分配"给出了 Flash XIP 段的确切地址；本骨架链接脚本
暂用 2K1000LA 惯例值 `0x1c000000` 占位，**必须以手册为准修改**。

## 5. DDR 初始化责任（V3）

- 若走"SPL/PMON 模式"：BootROM 把镜像加载到（L2 Cache 或 DDR），期间可能已完成
  DDR 初始化 → 栈可直接放 DDR。
- 若走"纯 XIP"：BootROM 只负责取指，DDR 大概率未初始化 → 两条回退路：
  1. 移植 U-Boot SPL 的 DDR init 序列到 start.S（BSP 源码开放，约 +1 周）；
  2. L2-Cache-as-RAM 过渡（LoongArch 平台已有先例，用于栈与 .data）。
- 判定方法：XIP 直接 `li.d $sp, RAM_TOP` 试跑，串口无输出即大概率 DDR 未初始化。

## 6. 对 Arduino 层的影响

- 自启动打通后，Arduino HAL（GPIO/UART/定时器）全部基于寄存器直操作，
  `millis()` 用 CLINT 风格稳定时基计数器（rdtime）实现，实时性与 MCU 一致。
- 若 BootROM 头部校验确实存在且不可关闭，则交付物是"带 BootROM 头的固件镜像"，
  对 Arduino 用户透明（烧写流程由 arduino-cli 的 upload recipe 封装）。
