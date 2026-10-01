.include "macro.inc"

.section .data, "wa"
    .align 0
    /* 93AEA 800A32EA */ .byte 0x00
    /* 93AEB 800A32EB */ .byte 0x00

nonmatching D_800A32EC

dlabel D_800A32EC
    /* 93AEC 800A32EC 9000FB01 */ .word 0x01FB0090
    /* 93AF0 800A32F0 30000100 */ .word 0x00010030
enddlabel D_800A32EC

nonmatching D_800A32F4

dlabel D_800A32F4
    /* 93AF4 800A32F4 80038001 */ .word 0x01800380
    /* 93AF8 800A32F8 40004000 */ .word 0x00400040
enddlabel D_800A32F4

nonmatching D_800A32FC

dlabel D_800A32FC
    /* 93AFC 800A32FC 80020000 */ .word 0x00000280
    /* 93B00 800A3300 4000B800 */ .word 0x00B80040
enddlabel D_800A32FC
