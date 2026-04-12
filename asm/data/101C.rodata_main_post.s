.include "macro.inc"

.section .rodata, "a"

.align 2
nonmatching D_800163D8

dlabel D_800163D8
    /* 6BD8 800163D8 */ .asciz "SPU:T/O [%s]\n"
    /* 5350553A542F4F205B25735D0A000000 */
.align 2
enddlabel D_800163D8

.align 2
nonmatching D_800163E8

dlabel D_800163E8
    /* 6BE8 800163E8 */ .asciz "wait (reset)"
    /* 77616974202872657365742900000000 */
.align 2
enddlabel D_800163E8

.align 2
nonmatching D_800163F8

dlabel D_800163F8
    /* 6BF8 800163F8 */ .asciz "wait (wrdy H -> L)"
    /* 776169742028777264792048202D3E204C290000 */
.align 2
enddlabel D_800163F8

.align 2
nonmatching D_8001640C

dlabel D_8001640C
    /* 6C0C 8001640C */ .asciz "wait (dmaf clear/W)"
    /* 776169742028646D616620636C6561722F572900 */
.align 2
enddlabel D_8001640C

.align 2
nonmatching jtbl_80016420

dlabel jtbl_80016420
    /* 6C20 80016420 30B00880 */ .word .L8008B030
    /* 6C24 80016424 F8AF0880 */ .word .L8008AFF8
    /* 6C28 80016428 00B00880 */ .word .L8008B000
    /* 6C2C 8001642C 08B00880 */ .word .L8008B008
    /* 6C30 80016430 10B00880 */ .word .L8008B010
    /* 6C34 80016434 18B00880 */ .word .L8008B018
    /* 6C38 80016438 20B00880 */ .word .L8008B020
    /* 6C3C 8001643C 28B00880 */ .word .L8008B028
enddlabel jtbl_80016420

.align 2
nonmatching jtbl_80016440

dlabel jtbl_80016440
    /* 6C40 80016440 F8B00880 */ .word .L8008B0F8
    /* 6C44 80016444 C0B00880 */ .word .L8008B0C0
    /* 6C48 80016448 C8B00880 */ .word .L8008B0C8
    /* 6C4C 8001644C D0B00880 */ .word .L8008B0D0
    /* 6C50 80016450 D8B00880 */ .word .L8008B0D8
    /* 6C54 80016454 E0B00880 */ .word .L8008B0E0
    /* 6C58 80016458 E8B00880 */ .word .L8008B0E8
    /* 6C5C 8001645C F0B00880 */ .word .L8008B0F0
enddlabel jtbl_80016440

.align 2
nonmatching jtbl_80016460

dlabel jtbl_80016460
    /* 6C60 80016460 CCB50880 */ .word .L8008B5CC
    /* 6C64 80016464 D4B50880 */ .word .L8008B5D4
    /* 6C68 80016468 DCB50880 */ .word .L8008B5DC
    /* 6C6C 8001646C E4B50880 */ .word .L8008B5E4
    /* 6C70 80016470 ECB50880 */ .word .L8008B5EC
    /* 6C74 80016474 F4B50880 */ .word .L8008B5F4
    /* 6C78 80016478 FCB50880 */ .word .L8008B5FC
    /* 6C7C 8001647C 00000000 */ .word 0x00000000
enddlabel jtbl_80016460

.align 2
nonmatching jtbl_80016480

dlabel jtbl_80016480
    /* 6C80 80016480 ACB60880 */ .word .L8008B6AC
    /* 6C84 80016484 B4B60880 */ .word .L8008B6B4
    /* 6C88 80016488 BCB60880 */ .word .L8008B6BC
    /* 6C8C 8001648C C4B60880 */ .word .L8008B6C4
    /* 6C90 80016490 CCB60880 */ .word .L8008B6CC
    /* 6C94 80016494 D4B60880 */ .word .L8008B6D4
    /* 6C98 80016498 DCB60880 */ .word .L8008B6DC
enddlabel jtbl_80016480

.align 2
nonmatching D_8001649C

dlabel D_8001649C
    /* 6C9C 8001649C */ .asciz "SIO console"
    /* 53494F20636F6E736F6C6500 */
.align 2
enddlabel D_8001649C

.align 2
nonmatching D_800164A8

dlabel D_800164A8
    /* 6CA8 800164A8 */ .asciz "sio"
    /* 73696F00 */
.align 2
enddlabel D_800164A8
