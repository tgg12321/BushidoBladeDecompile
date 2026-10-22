.include "macro.inc"

.section .data, "wa"
    .align 0

/* 5ED34.c's .data (0x8009BC1C..0x8009BCD4), still asm: its D_8009BCD0
 * (s16[2]) is a 4-byte initialized global in .data, where the per-file gp
 * model (A3) puts an object of <= 8 bytes in .sdata. */

nonmatching D_8009BC1C

dlabel D_8009BC1C
    /* 8C41C 8009BC1C 7CF90680 */ .word menu_UpdateEntryGrid
    /* 8C420 8009BC20 700C0780 */ .word menu_UpdateRow
    /* 8C424 8009BC24 00F10680 */ .word func_8006F100
    /* 8C428 8009BC28 4C1C0780 */ .word func_80071C4C
    /* 8C42C 8009BC2C 84200780 */ .word func_80072084
    /* 8C430 8009BC30 AC200780 */ .word func_800720AC
    /* 8C434 8009BC34 D4200780 */ .word func_800720D4
enddlabel D_8009BC1C

nonmatching D_8009BC38

dlabel D_8009BC38
    /* 8C438 8009BC38 */ .byte 0x04
    /* 8C439 8009BC39 */ .byte 0x03
    /* 8C43A 8009BC3A */ .byte 0x00
    /* 8C43B 8009BC3B */ .byte 0x02
    /* 8C43C 8009BC3C */ .byte 0x01
    /* 8C43D 8009BC3D */ .byte 0x05
    /* 8C43E 8009BC3E */ .byte 0x00
    /* 8C43F 8009BC3F */ .byte 0x00
enddlabel D_8009BC38

nonmatching D_8009BC40

dlabel D_8009BC40
    /* 8C440 8009BC40 */ .byte 0x00
enddlabel D_8009BC40

nonmatching D_8009BC41

dlabel D_8009BC41
    /* 8C441 8009BC41 */ .byte 0x00
    /* 8C442 8009BC42 */ .byte 0x09
    /* 8C443 8009BC43 */ .byte 0x09
enddlabel D_8009BC41

nonmatching D_8009BC44

dlabel D_8009BC44
    /* 8C444 8009BC44 */ .byte 0x0A
    /* 8C445 8009BC45 */ .byte 0x0C
    /* 8C446 8009BC46 */ .byte 0x11
    /* 8C447 8009BC47 */ .byte 0x13
    /* 8C448 8009BC48 */ .byte 0x15
    /* 8C449 8009BC49 */ .byte 0x1A
    /* 8C44A 8009BC4A */ .byte 0x14
    /* 8C44B 8009BC4B */ .byte 0x18
    /* 8C44C 8009BC4C */ .byte 0x02
    /* 8C44D 8009BC4D */ .byte 0x02
    /* 8C44E 8009BC4E */ .byte 0x07
    /* 8C44F 8009BC4F */ .byte 0x07
    /* 8C450 8009BC50 */ .byte 0x0B
    /* 8C451 8009BC51 */ .byte 0x0D
    /* 8C452 8009BC52 */ .byte 0x0D
    /* 8C453 8009BC53 */ .byte 0x0F
    /* 8C454 8009BC54 */ .byte 0x15
    /* 8C455 8009BC55 */ .byte 0x1A
    /* 8C456 8009BC56 */ .byte 0x14
    /* 8C457 8009BC57 */ .byte 0x18
    /* 8C458 8009BC58 */ .byte 0x01
    /* 8C459 8009BC59 */ .byte 0x01
    /* 8C45A 8009BC5A */ .byte 0x08
    /* 8C45B 8009BC5B */ .byte 0x08
    /* 8C45C 8009BC5C */ .byte 0x0C
    /* 8C45D 8009BC5D */ .byte 0x0E
    /* 8C45E 8009BC5E */ .byte 0x12
    /* 8C45F 8009BC5F */ .byte 0x14
    /* 8C460 8009BC60 */ .byte 0x15
    /* 8C461 8009BC61 */ .byte 0x1A
    /* 8C462 8009BC62 */ .byte 0x14
    /* 8C463 8009BC63 */ .byte 0x18
    /* 8C464 8009BC64 */ .byte 0x03
    /* 8C465 8009BC65 */ .byte 0x03
    /* 8C466 8009BC66 */ .byte 0x06
    /* 8C467 8009BC67 */ .byte 0x06
    /* 8C468 8009BC68 */ .byte 0x10
    /* 8C469 8009BC69 */ .byte 0x12
    /* 8C46A 8009BC6A */ .byte 0x0F
    /* 8C46B 8009BC6B */ .byte 0x11
    /* 8C46C 8009BC6C */ .byte 0x15
    /* 8C46D 8009BC6D */ .byte 0x1A
    /* 8C46E 8009BC6E */ .byte 0x14
    /* 8C46F 8009BC6F */ .byte 0x18
    /* 8C470 8009BC70 */ .byte 0x04
    /* 8C471 8009BC71 */ .byte 0x04
enddlabel D_8009BC44

nonmatching D_8009BC72

dlabel D_8009BC72
    /* 8C472 8009BC72 */ .byte 0x05
    /* 8C473 8009BC73 */ .byte 0x05
    /* 8C474 8009BC74 */ .byte 0x13
    /* 8C475 8009BC75 */ .byte 0x15
enddlabel D_8009BC72

nonmatching D_8009BC76

dlabel D_8009BC76
    /* 8C476 8009BC76 */ .byte 0x0E
    /* 8C477 8009BC77 */ .byte 0x10
    /* 8C478 8009BC78 */ .byte 0x15
    /* 8C479 8009BC79 */ .byte 0x1A
    /* 8C47A 8009BC7A */ .byte 0x14
    /* 8C47B 8009BC7B */ .byte 0x18
enddlabel D_8009BC76

nonmatching D_8009BC7C

dlabel D_8009BC7C
    /* 8C47C 8009BC7C */ .byte 0x02
    /* 8C47D 8009BC7D */ .byte 0x02
    /* 8C47E 8009BC7E */ .byte 0x02
    /* 8C47F 8009BC7F */ .byte 0x02
    /* 8C480 8009BC80 */ .byte 0x02
    /* 8C481 8009BC81 */ .byte 0x02
    /* 8C482 8009BC82 */ .byte 0x02
    /* 8C483 8009BC83 */ .byte 0x02
    /* 8C484 8009BC84 */ .byte 0x02
    /* 8C485 8009BC85 */ .byte 0x02
    /* 8C486 8009BC86 */ .byte 0x00
    /* 8C487 8009BC87 */ .byte 0x00
    /* 8C488 8009BC88 */ .byte 0x00
    /* 8C489 8009BC89 */ .byte 0x00
    /* 8C48A 8009BC8A */ .byte 0x00
    /* 8C48B 8009BC8B */ .byte 0x00
    /* 8C48C 8009BC8C */ .byte 0x00
    /* 8C48D 8009BC8D */ .byte 0x00
    /* 8C48E 8009BC8E */ .byte 0x00
    /* 8C48F 8009BC8F */ .byte 0x00
    /* 8C490 8009BC90 */ .byte 0x02
    /* 8C491 8009BC91 */ .byte 0x00
    /* 8C492 8009BC92 */ .byte 0x00
    /* 8C493 8009BC93 */ .byte 0x00
enddlabel D_8009BC7C

nonmatching D_8009BC94

dlabel D_8009BC94
    /* 8C494 8009BC94 */ .short 0xFFC8
enddlabel D_8009BC94

nonmatching D_8009BC96

dlabel D_8009BC96
    /* 8C496 8009BC96 */ .short 0xFFE1
    /* 8C498 8009BC98 */ .short 0xFFFA
    /* 8C49A 8009BC9A */ .short 0xFFF2
    /* 8C49C 8009BC9C */ .short 0xFFE6
    /* 8C49E 8009BC9E */ .short 0xFFF0
    /* 8C4A0 8009BCA0 */ .short 0xFFBC
    /* 8C4A2 8009BCA2 */ .short 0xFFE0
    /* 8C4A4 8009BCA4 */ .short 0xFFDE
    /* 8C4A6 8009BCA6 */ .short 0xFFE0
    /* 8C4A8 8009BCA8 */ .short 0xFFC8
    /* 8C4AA 8009BCAA */ .short 0xFFE1
    /* 8C4AC 8009BCAC */ .short 0x0038
    /* 8C4AE 8009BCAE */ .short 0xFFE1
    /* 8C4B0 8009BCB0 */ .short 0x0006
    /* 8C4B2 8009BCB2 */ .short 0xFFF2
enddlabel D_8009BC96

nonmatching D_8009BCB4

dlabel D_8009BCB4
    /* 8C4B4 8009BCB4 1A00F0FF */ .word 0xFFF0001A
    /* 8C4B8 8009BCB8 4400E0FF */ .word 0xFFE00044
    /* 8C4BC 8009BCBC 2200E0FF */ .word 0xFFE00022
    /* 8C4C0 8009BCC0 3800E1FF */ .word 0xFFE10038
enddlabel D_8009BCB4

nonmatching D_8009BCC4

dlabel D_8009BCC4
    /* 8C4C4 8009BCC4 */ .short 0x0000
enddlabel D_8009BCC4

nonmatching D_8009BCC6

dlabel D_8009BCC6
    /* 8C4C6 8009BCC6 */ .short 0x0017
    /* 8C4C8 8009BCC8 */ .short 0x0099
    /* 8C4CA 8009BCCA */ .short 0x0003
    /* 8C4CC 8009BCCC */ .short 0x00A0
    /* 8C4CE 8009BCCE */ .short 0x0024
enddlabel D_8009BCC6

nonmatching D_8009BCD0

dlabel D_8009BCD0
    /* 8C4D0 8009BCD0 */ .short 0x0000
enddlabel D_8009BCD0

nonmatching D_8009BCD2

dlabel D_8009BCD2
    /* 8C4D2 8009BCD2 */ .short 0x0000
enddlabel D_8009BCD2
