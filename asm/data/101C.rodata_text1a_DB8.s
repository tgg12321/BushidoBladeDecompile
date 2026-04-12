.include "macro.inc"

.section .rodata, "a"

.align 2
nonmatching jtbl_80010DB8

dlabel jtbl_80010DB8
    /* 15B8 80010DB8 000E0480 */ .word .L80040E00
    /* 15BC 80010DBC A00F0480 */ .word .L80040FA0
    /* 15C0 80010DC0 B80F0480 */ .word .L80040FB8
    /* 15C4 80010DC4 B80F0480 */ .word .L80040FB8
    /* 15C8 80010DC8 B80F0480 */ .word .L80040FB8
    /* 15CC 80010DCC B80F0480 */ .word .L80040FB8
    /* 15D0 80010DD0 B80F0480 */ .word .L80040FB8
enddlabel jtbl_80010DB8
