.section .text, "ax"
.global _start
_start:
    .cfi_startproc
    .cfi_undefined ra
    .option push
    .option norelax
    1: auipc gp, %pcrel_hi(__global_pointer$)
    addi gp, gp, %pcrel_lo(1b)
    .option pop
    la sp, kernel_stack_top
    add s0, sp, zero
    jal zero, main
    .cfi_endproc
    .end
