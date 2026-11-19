/*
 * 2K0300 裸机启动汇编（LoongArch64, LA264 = la464 子集）
 *
 * 入口假设：BootROM 已把控制权交给 Flash XIP 映射区的镜像首地址
 * （数据手册：SPI Flash 映射为只读 memory，复位后直接取指）。
 * 待验证项见 README V1/V2/V3。
 */
    .section .text.start, "ax"
    .globl _start
_start:
    /*
     * 注意：复位后默认运行环境（CSR.CRMD 等）未做任何假设。
     * BootROM 交给我们的状态需要在 S2/S3 实验中用寄存器 dump 确认，
     * 这里保守起见先关中断（CRMD.IE 清零）。
     * CRMD 编号 = 0，IE 为 bit2；csrxchg: rd=新值, rj=掩码, csr=0
     */
    li.d        $t0, 0xFFFFFFFFFFFFFFF9   /* ~CRMD_IE */
    li.d        $t1, 0                    /* 目标值 0 */
    csrxchg     $t1, $t0, 0

    /* 栈顶：DDR 区域（见链接脚本 RAM 段；若 BootROM 未初始化 DDR，见 README V3 回退方案） */
    la.local    $sp, STACK_TOP

    /* 清零 .bss（位于 DDR） */
    la.pcrel    $t0, __bss_start
    la.pcrel    $t1, __bss_end
1:
    beq         $t0, $t1, 2f
    st.d        $zero, $t0, 0
    addi.d      $t0, $t0, 8
    b           1b
2:
    /* 跳入 C */
    bl          main

    /* main 返回即挂死 */
3:
    b           3b
