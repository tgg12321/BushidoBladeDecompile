.include "macro.inc"

.section .data, "wa"
    .align 0

nonmatching D_800A31DC

dlabel D_800A31DC
    /* 939DC 800A31DC */ .asciz "%s"
    .space 1
    /* 939E0 800A31E0 DC310A80 */ .word D_800A31DC
enddlabel D_800A31DC
