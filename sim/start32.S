/*
 * LA32S (OpenLA132) 仿真变体启动代码
 *
 * CSR 序列完全照搬 OpenLACores verify/apps/common/crt/start.S 的已验证写法
 * （单 hart 简化版），最终进入直连地址模式（VA=PA）后调用 main。
 * 仿真目标：verify/verilator/build/emu-mmu0-sram1（CONFIG_MMU=0 CONFIG_SRAM=1）
 */
    .text
    .globl _start

_start:
    /* 清零通用寄存器（与官方 crt 一致） */
    li.w    $r1, 0
    li.w    $r2, 0
    li.w    $r4, 0
    li.w    $r5, 0
    li.w    $r6, 0
    li.w    $r7, 0
    li.w    $r8, 0
    li.w    $r9, 0
    li.w    $r10, 0
    li.w    $r11, 0
    li.w    $r12, 0
    li.w    $r13, 0
    li.w    $r14, 0
    li.w    $r15, 0
    li.w    $r16, 0
    li.w    $r17, 0
    li.w    $r18, 0
    li.w    $r19, 0
    li.w    $r20, 0
    li.w    $r21, 0
    li.w    $r22, 0
    li.w    $r23, 0
    li.w    $r24, 0
    li.w    $r25, 0
    li.w    $r26, 0
    li.w    $r27, 0
    li.w    $r28, 0
    li.w    $r29, 0
    li.w    $r30, 0
    li.w    $r31, 0

    /* DMW0: PA=VA, PLV0, cached */
    li.w    $r11, 0x11
    csrwr   $r11, 0x180
    /* DMW1: VSEG=8, PLV0, uncached —— MMIO 窗口 */
    li.w    $r11, 0x80000009
    csrwr   $r11, 0x181
    /* CRMD: DATF/DATM = coherent cached */
    li.w    $r12, 0x10
    csrwr   $r12, 0x0
    /* 使能 BTB 与周期计数器 */
    li.w    $r11, 0x00100000
    li.w    $r12, 0x00100010
    csrxchg $r11, $r12, 0xaf

    /* 经 ertn 进入直连地址模式 */
    la.global $r13, __enter_main
    csrwr   $r13, 0x6          /* ERA */
    ertn

__enter_main:
    /* CRMD: PLV0 | DATF=01 | DATM=01 —— 访存 MAT 设为强序非缓存，
       否则对 0x1fe00000 UART/LED 的写会被缓存吸收，AXI 模型收不到 */
    li.w    $t0, 0xA0
    csrwr   $t0, 0x0

    /* 栈顶（来自链接脚本 .stack 段） */
    la.global $r3, _stack_end

    /* 清零 .bss */
    la.global $t0, _bss_start
    la.global $t1, _bss_end
1:
    beq     $t0, $t1, 2f
    st.w    $zero, $t0, 0
    addi.w  $t0, $t0, 4
    b       1b
2:
    la.global $t2, main
    jirl    $r1, $t2, 0

    /* main 返回：进入官方约定的 GOOD TRAP 标记循环
       （addi.w $r0,$r0,52 —— 仿真器识别为正常退出） */
1:
    addi.w  $r0, $r0, 52
    b       1b
