.include "macro.inc"

.section .data, "wa"
    .align 0

/* LIBC2 CTYPE's 129-byte .data (_ctype_, 0x8009BD8C..0x8009BE0D) and the
 * 3 pad bytes before SPRINTF's 4-aligned .data (0x8009BE10). It stays asm
 * while ctype.c, prnt.c and putchar.c read it as _ctype__plus_0x1. */

nonmatching _ctype_

dlabel _ctype_
    /* 8C58C 8009BD8C */ .byte 0x00
enddlabel _ctype_

nonmatching _ctype__plus_0x1

dlabel _ctype__plus_0x1
    /* 8C58D 8009BD8D */ .byte 0x20
    /* 8C58E 8009BD8E */ .short 0x2020
    /* 8C590 8009BD90 20202020 */ .word 0x20202020
    /* 8C594 8009BD94 20200808 */ .word 0x08082020
    /* 8C598 8009BD98 08080820 */ .word 0x20080808
    /* 8C59C 8009BD9C 20202020 */ .word 0x20202020
    /* 8C5A0 8009BDA0 20202020 */ .word 0x20202020
    /* 8C5A4 8009BDA4 20202020 */ .word 0x20202020
    /* 8C5A8 8009BDA8 20202020 */ .word 0x20202020
    /* 8C5AC 8009BDAC 20181010 */ .word 0x10101820
    /* 8C5B0 8009BDB0 10101010 */ .word 0x10101010
    /* 8C5B4 8009BDB4 10101010 */ .word 0x10101010
    /* 8C5B8 8009BDB8 10101010 */ .word 0x10101010
    /* 8C5BC 8009BDBC 10040404 */ .word 0x04040410
    /* 8C5C0 8009BDC0 04040404 */ .word 0x04040404
    /* 8C5C4 8009BDC4 04040410 */ .word 0x10040404
    /* 8C5C8 8009BDC8 10101010 */ .word 0x10101010
    /* 8C5CC 8009BDCC 10104141 */ .word 0x41411010
    /* 8C5D0 8009BDD0 41414141 */ .word 0x41414141
    /* 8C5D4 8009BDD4 01010101 */ .word 0x01010101
    /* 8C5D8 8009BDD8 01010101 */ .word 0x01010101
    /* 8C5DC 8009BDDC 01010101 */ .word 0x01010101
    /* 8C5E0 8009BDE0 01010101 */ .word 0x01010101
    /* 8C5E4 8009BDE4 01010101 */ .word 0x01010101
    /* 8C5E8 8009BDE8 10101010 */ .word 0x10101010
    /* 8C5EC 8009BDEC 10104242 */ .word 0x42421010
    /* 8C5F0 8009BDF0 42424242 */ .word 0x42424242
    /* 8C5F4 8009BDF4 02020202 */ .word 0x02020202
    /* 8C5F8 8009BDF8 02020202 */ .word 0x02020202
    /* 8C5FC 8009BDFC 02020202 */ .word 0x02020202
    /* 8C600 8009BE00 02020202 */ .word 0x02020202
    /* 8C604 8009BE04 02020202 */ .word 0x02020202
    /* 8C608 8009BE08 10101010 */ .word 0x10101010
    /* 8C60C 8009BE0C 20000000 */ .word 0x00000020
enddlabel _ctype__plus_0x1
