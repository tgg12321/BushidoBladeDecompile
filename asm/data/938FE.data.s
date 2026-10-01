.include "macro.inc"

.section .data, "wa"
    .align 0
    /* 938FE 800A30FE */ .byte 0x00
    /* 938FF 800A30FF */ .byte 0x00

nonmatching D_800A3100

dlabel D_800A3100
    /* 93900 800A3100 03332400 */ .word 0x00243303
    /* 93904 800A3104 50112D00 */ .word 0x002D1150
    /* 93908 800A3108 26000000 */ .word 0x00000026
enddlabel D_800A3100

nonmatching D_800A310C

dlabel D_800A310C
    /* 9390C 800A310C */ .short 0x00A6
    /* 9390E 800A310E */ .short 0x00C8
    /* 93910 800A3110 */ .short 0x00E9
    /* 93912 800A3112 */ .short 0x0000
enddlabel D_800A310C

nonmatching D_800A3114

dlabel D_800A3114
    /* 93914 800A3114 */ .asciz "ON.BBM"
    .space 1
enddlabel D_800A3114

nonmatching D_800A311C

dlabel D_800A311C
    /* 9391C 800A311C */ .asciz "GS.BBM"
    .space 1
enddlabel D_800A311C

nonmatching D_800A3124

dlabel D_800A3124
    /* 93924 800A3124 */ .asciz "GN.BBM"
    .space 1
enddlabel D_800A3124

nonmatching D_800A312C

dlabel D_800A312C
    /* 9392C 800A312C 001000FC */ .word 0xFC001000
    /* 93930 800A3130 00000000 */ .word 0x00000000
enddlabel D_800A312C

nonmatching D_800A3134

dlabel D_800A3134
    /* 93934 800A3134 B80B0000 */ .word 0x00000BB8
enddlabel D_800A3134

nonmatching D_800A3138

dlabel D_800A3138
    /* 93938 800A3138 00000010 */ .word 0x10000000
    /* 9393C 800A313C 00000000 */ .word 0x00000000
enddlabel D_800A3138
