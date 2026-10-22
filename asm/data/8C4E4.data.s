.include "macro.inc"

.section .data, "wa"
    .align 0

/* 64FD8.c's .data (0x8009BCE4..0x8009BD68), still asm: D_8009BD20 (u8[2][2],
 * read by 63D2C.c and 64FD8.c) is a 4-byte initialized global in .data, where
 * the per-file gp model (A3) puts an object of <= 8 bytes in .sdata. */

nonmatching D_8009BCE4

dlabel D_8009BCE4
    /* 8C4E4 8009BCE4 */ .byte 0x02
    /* 8C4E5 8009BCE5 */ .byte 0x02
    /* 8C4E6 8009BCE6 */ .byte 0x02
    /* 8C4E7 8009BCE7 */ .byte 0x02
    /* 8C4E8 8009BCE8 */ .byte 0x02
    /* 8C4E9 8009BCE9 */ .byte 0x02
    /* 8C4EA 8009BCEA */ .byte 0x02
    /* 8C4EB 8009BCEB */ .byte 0x02
    /* 8C4EC 8009BCEC */ .byte 0x02
    /* 8C4ED 8009BCED */ .byte 0x02
    /* 8C4EE 8009BCEE */ .byte 0x00
    /* 8C4EF 8009BCEF */ .byte 0x00
    /* 8C4F0 8009BCF0 */ .byte 0x00
    /* 8C4F1 8009BCF1 */ .byte 0x00
    /* 8C4F2 8009BCF2 */ .byte 0x00
    /* 8C4F3 8009BCF3 */ .byte 0x00
    /* 8C4F4 8009BCF4 */ .byte 0x00
    /* 8C4F5 8009BCF5 */ .byte 0x00
    /* 8C4F6 8009BCF6 */ .byte 0x00
    /* 8C4F7 8009BCF7 */ .byte 0x00
enddlabel D_8009BCE4

nonmatching D_8009BCF8

dlabel D_8009BCF8
    /* 8C4F8 8009BCF8 */ .byte 0x00
enddlabel D_8009BCF8

nonmatching D_8009BCF9

dlabel D_8009BCF9
    /* 8C4F9 8009BCF9 */ .byte 0x00
    /* 8C4FA 8009BCFA */ .byte 0x02
    /* 8C4FB 8009BCFB */ .byte 0x01
    /* 8C4FC 8009BCFC */ .byte 0x01
    /* 8C4FD 8009BCFD */ .byte 0x02
    /* 8C4FE 8009BCFE */ .byte 0x03
    /* 8C4FF 8009BCFF */ .byte 0x03
    /* 8C500 8009BD00 */ .byte 0x04
    /* 8C501 8009BD01 */ .byte 0x04
    /* 8C502 8009BD02 */ .byte 0x09
    /* 8C503 8009BD03 */ .byte 0x05
    /* 8C504 8009BD04 */ .byte 0x07
    /* 8C505 8009BD05 */ .byte 0x06
    /* 8C506 8009BD06 */ .byte 0x08
    /* 8C507 8009BD07 */ .byte 0x07
    /* 8C508 8009BD08 */ .byte 0x06
    /* 8C509 8009BD09 */ .byte 0x08
    /* 8C50A 8009BD0A */ .byte 0x05
    /* 8C50B 8009BD0B */ .byte 0x09
    /* 8C50C 8009BD0C */ .byte 0x0A
    /* 8C50D 8009BD0D */ .byte 0x0C
    /* 8C50E 8009BD0E */ .byte 0x0B
    /* 8C50F 8009BD0F */ .byte 0x0D
    /* 8C510 8009BD10 */ .byte 0x0C
    /* 8C511 8009BD11 */ .byte 0x0E
    /* 8C512 8009BD12 */ .byte 0x10
    /* 8C513 8009BD13 */ .byte 0x0F
    /* 8C514 8009BD14 */ .byte 0x13
    /* 8C515 8009BD15 */ .byte 0x10
    /* 8C516 8009BD16 */ .byte 0x11
    /* 8C517 8009BD17 */ .byte 0x11
    /* 8C518 8009BD18 */ .byte 0x0D
    /* 8C519 8009BD19 */ .byte 0x12
    /* 8C51A 8009BD1A */ .byte 0x12
    /* 8C51B 8009BD1B */ .byte 0x13
    /* 8C51C 8009BD1C */ .byte 0x0F
    /* 8C51D 8009BD1D */ .byte 0x14
    /* 8C51E 8009BD1E */ .byte 0x0E
    /* 8C51F 8009BD1F */ .byte 0x15
enddlabel D_8009BCF9

nonmatching D_8009BD20

dlabel D_8009BD20
    /* 8C520 8009BD20 */ .byte 0x01
enddlabel D_8009BD20

nonmatching D_8009BD21

dlabel D_8009BD21
    /* 8C521 8009BD21 */ .byte 0x02
    /* 8C522 8009BD22 */ .byte 0x03
    /* 8C523 8009BD23 */ .byte 0x04
enddlabel D_8009BD21

nonmatching D_8009BD24

dlabel D_8009BD24
    /* 8C524 8009BD24 */ .byte 0x00
    /* 8C525 8009BD25 */ .byte 0x00
    /* 8C526 8009BD26 */ .byte 0x00
    /* 8C527 8009BD27 */ .byte 0x00
    /* 8C528 8009BD28 */ .byte 0x00
    /* 8C529 8009BD29 */ .byte 0x00
    /* 8C52A 8009BD2A */ .byte 0x00
    /* 8C52B 8009BD2B */ .byte 0x00
    /* 8C52C 8009BD2C */ .byte 0x00
    /* 8C52D 8009BD2D */ .byte 0x00
    /* 8C52E 8009BD2E */ .byte 0x00
    /* 8C52F 8009BD2F */ .byte 0x00
    /* 8C530 8009BD30 */ .byte 0x00
    /* 8C531 8009BD31 */ .byte 0x00
    /* 8C532 8009BD32 */ .byte 0x00
    /* 8C533 8009BD33 */ .byte 0x00
    /* 8C534 8009BD34 */ .byte 0x00
    /* 8C535 8009BD35 */ .byte 0x00
    /* 8C536 8009BD36 */ .byte 0x00
    /* 8C537 8009BD37 */ .byte 0x00
enddlabel D_8009BD24

nonmatching D_8009BD38

dlabel D_8009BD38
    /* 8C538 8009BD38 00000011 */ .word 0x11000000
enddlabel D_8009BD38

nonmatching D_8009BD3C

dlabel D_8009BD3C
    /* 8C53C 8009BD3C 44881144 */ .word 0x44118844
    /* 8C540 8009BD40 */ .byte 0x88
enddlabel D_8009BD3C

nonmatching D_8009BD41

dlabel D_8009BD41
    /* 8C541 8009BD41 */ .byte 0x11
enddlabel D_8009BD41

nonmatching D_8009BD42

dlabel D_8009BD42
    /* 8C542 8009BD42 */ .short 0x8844
enddlabel D_8009BD42

nonmatching D_8009BD44

dlabel D_8009BD44
    /* 8C544 8009BD44 023B0000 */ .word 0x00003B02
    /* 8C548 8009BD48 003B0000 */ .word 0x00003B00
    /* 8C54C 8009BD4C 003B0000 */ .word 0x00003B00
    /* 8C550 8009BD50 00000000 */ .word 0x00000000
    /* 8C554 8009BD54 00000000 */ .word 0x00000000
enddlabel D_8009BD44

nonmatching D_8009BD58

dlabel D_8009BD58
    /* 8C558 8009BD58 */ .byte 0x00
    /* 8C559 8009BD59 */ .byte 0x00
    /* 8C55A 8009BD5A */ .byte 0x01
    /* 8C55B 8009BD5B */ .byte 0x00
    /* 8C55C 8009BD5C */ .byte 0x02
    /* 8C55D 8009BD5D */ .byte 0x00
    /* 8C55E 8009BD5E */ .byte 0x03
    /* 8C55F 8009BD5F */ .byte 0x00
    /* 8C560 8009BD60 */ .byte 0x04
    /* 8C561 8009BD61 */ .byte 0x01
    /* 8C562 8009BD62 */ .byte 0x05
    /* 8C563 8009BD63 */ .byte 0x01
    /* 8C564 8009BD64 */ .byte 0x06
    /* 8C565 8009BD65 */ .byte 0x01
    /* 8C566 8009BD66 */ .byte 0x07
    /* 8C567 8009BD67 */ .byte 0x00
enddlabel D_8009BD58
