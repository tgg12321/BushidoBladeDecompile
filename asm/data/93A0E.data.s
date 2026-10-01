.include "macro.inc"

.section .data, "wa"
    .align 0
    /* 93A0E 800A320E */ .byte 0x00
    /* 93A0F 800A320F */ .byte 0x00

nonmatching g_str_sio_800A3210

dlabel g_str_sio_800A3210
    /* 93A10 800A3210 */ .asciz "sio:"
    .space 3
enddlabel g_str_sio_800A3210
