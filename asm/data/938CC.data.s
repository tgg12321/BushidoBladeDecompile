.include "macro.inc"

.section .data, "wa"
    .align 0

nonmatching g_gpu_clear_rect

dlabel g_gpu_clear_rect
    /* 938CC 800A30CC 00000000 */ .word 0x00000000
    /* 938D0 800A30D0 8002E001 */ .word 0x01E00280
enddlabel g_gpu_clear_rect

nonmatching D_800A30D4

dlabel D_800A30D4
    /* 938D4 800A30D4 20006000 */ .word 0x00600020
    /* 938D8 800A30D8 00013000 */ .word 0x00300100
enddlabel D_800A30D4
