.include "macro.inc"

.section .data, "wa"
    .align 0

nonmatching _spu_AllocBlockNum

dlabel _spu_AllocBlockNum
    /* 93538 800A2D38 00000000 */ .word 0x00000000
enddlabel _spu_AllocBlockNum

nonmatching _spu_AllocLastNum

dlabel _spu_AllocLastNum
    /* 9353C 800A2D3C 00000000 */ .word 0x00000000
enddlabel _spu_AllocLastNum

nonmatching _spu_memList

dlabel _spu_memList
    /* 93540 800A2D40 00000000 */ .word 0x00000000
enddlabel _spu_memList

