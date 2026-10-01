.include "macro.inc"

.section .data, "wa"
    .align 0

nonmatching D_800A32C0

dlabel D_800A32C0
    /* 93AC0 800A32C0 7000E101 */ .word 0x01E10070
    /* 93AC4 800A32C4 30000100 */ .word 0x00010030
enddlabel D_800A32C0

nonmatching D_800A32C8

dlabel D_800A32C8
    /* 93AC8 800A32C8 00200000 */ .word 0x00002000
    /* 93ACC 800A32CC 00400000 */ .word 0x00004000
enddlabel D_800A32C8

nonmatching D_800A32D0

dlabel D_800A32D0
    /* 93AD0 800A32D0 00800000 */ .word 0x00008000
    /* 93AD4 800A32D4 00100000 */ .word 0x00001000
enddlabel D_800A32D0

nonmatching D_800A32D8

dlabel D_800A32D8
    /* 93AD8 800A32D8 00000000 */ .word 0x00000000
    /* 93ADC 800A32DC 8002F000 */ .word 0x00F00280
enddlabel D_800A32D8

nonmatching D_800A32E0

dlabel D_800A32E0
    /* 93AE0 800A32E0 00000000 */ .word 0x00000000
    /* 93AE4 800A32E4 8002E001 */ .word 0x01E00280
enddlabel D_800A32E0
