/*
 * 2K0300 裸机启动汇编（LoongArch64, LA264 = la464 子集）
 *
 * v2：CSR 序列按 OpenLACores verify/apps/common/crt/start.S 的已验证写法
 *     （LA32S 仿真版）移植到 LA64，原理已在 Verilator 仿真中通过：
 *       1. 配置 DMW0/DMW1 → ertn 进入直连地址模式
 *       2. MMIO 一律走 DMW1 非缓存窗口（VA[63:60]=8），不依赖 CRMD.DATM
 *     与 LA32 仿真版的差异：LA64 无需依赖 DATM=1 也能保证 MMIO 强序。
 *
 * 入口假设：BootROM 把控制权交给 Flash XIP 映射区镜像首地址（待验证项 V1/V2）。
 */
    .section .text.start, "ax"
    .globl _start
_start:
    /* 清零通用寄存器（与官方 crt 一致；$sp 稍后设置，$r1 返回地址置零） */
    li.d        $r1, 0
    li.d        $r4, 0
    li.d        $r5, 0
    li.d        $r6, 0
    li.d        $r7, 0
    li.d        $r8, 0
    li.d        $r9, 0
    li.d        $r10, 0
    li.d        $r11, 0
    li.d        $r12, 0
    li.d        $r13, 0
    li.d        $r14, 0
    li.d        $r15, 0
    li.d        $r16, 0
    li.d        $r17, 0
    li.d        $r18, 0
    li.d        $r19, 0
    li.d        $r20, 0
    li.d        $r21, 0
    li.d        $r22, 0
    li.d        $r23, 0
    li.d        $r24, 0
    li.d        $r25, 0
    li.d        $r26, 0
    li.d        $r27, 0
    li.d        $r28, 0
    li.d        $r29, 0
    li.d        $r30, 0
    li.d        $r31, 0

    /*
     * DMW0 = 0x11：直连地址窗口（PSEG=0, VA=PA），PLV0，MAT=0 一致性缓存
     *   —— 代码（Flash XIP）与 DDR 数据走此窗口，可缓存
     * DMW1 = 0x80000011：PSEG=8 窗口，PLV0，MAT=1 强序非缓存
     *   —— MMIO 统一走 VA = 0x8000_0000_0000_0000 | PA
     *   仿真（LA32）实证：MMIO 若落入缓存属性访问，写事务会被吸收
     */
    li.d        $t0, 0x11
    csrwr       $t0, 0x180
    li.d        $t0, 0x80000011
    csrwr       $t0, 0x181

    /* CRMD：PLV0 | IE=0 | PG=1 | DATF=一致缓存（取指可缓存） */
    li.d        $t0, 0x10
    csrwr       $t0, 0x0

    /* 经 ertn 进入直连地址模式（与官方 crt 的已验证序列一致） */
    la.global   $t1, __enter_main
    csrwr       $t1, 0x6               /* ERA */
    ertn

__enter_main:
    /* CRMD 维持 PLV0|IE=0|PG=1，DATF/DATM=缓存（MMIO 由 DMW1 保证非缓存） */
    li.d        $t0, 0x10
    csrwr       $t0, 0x0

    /* 栈顶：DDR 区域（见链接脚本 RAM 段；若 BootROM 未初始化 DDR，见 README V3 回退方案） */
    la.global   $sp, _stack_top

    /* .data：从 Flash 装载地址（LMA）拷贝到 RAM（原 TODO(V3)，此处补齐） */
    la.local    $t0, __data_load_start
    la.global   $t1, __data_start
    la.global   $t2, __data_end
1:
    beq         $t1, $t2, 2f
    ld.d        $t3, $t0, 0
    st.d        $t3, $t1, 0
    addi.d      $t0, $t0, 8
    addi.d      $t1, $t1, 8
    b           1b

    /* 清零 .bss（位于 DDR，8 字节步进） */
2:
    la.global   $t0, __bss_start
    la.global   $t1, __bss_end
3:
    beq         $t0, $t1, 4f
    st.d        $zero, $t0, 0
    addi.d      $t0, $t0, 8
    b           3b

    /* 跳入 C */
4:
    bl          main

    /* main 返回即挂死 */
5:
    b           5b
