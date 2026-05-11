glabel func_8007E24C
    /* 6EA4C 8007E24C 0000888C */  lw         $t0, 0x0($a0)
    /* 6EA50 8007E250 04008A8C */  lw         $t2, 0x4($a0)
    /* 6EA54 8007E254 034C0800 */  sra        $t1, $t0, 16
    /* 6EA58 8007E258 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EA5C 8007E25C FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EA60 8007E260 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EA64 8007E264 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EA68 8007E268 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EA6C 8007E26C 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EA70 8007E270 00000000 */  nop
    /* 6EA74 8007E274 3D00984B */  gpf        1
    /* 6EA78 8007E278 0000A88C */  lw         $t0, 0x0($a1)
    /* 6EA7C 8007E27C 0400AA8C */  lw         $t2, 0x4($a1)
    /* 6EA80 8007E280 034C0800 */  sra        $t1, $t0, 16
    /* 6EA84 8007E284 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EA88 8007E288 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EA8C 8007E28C 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EA90 8007E290 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EA94 8007E294 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EA98 8007E298 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EA9C 8007E29C 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EAA0 8007E2A0 00000000 */  nop
    /* 6EAA4 8007E2A4 3E00A84B */  gpl        1
    /* 6EAA8 8007E2A8 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6EAAC 8007E2AC 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6EAB0 8007E2B0 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EAB4 8007E2B4 004C0900 */  sll        $t1, $t1, 16
    /* 6EAB8 8007E2B8 25400901 */  or         $t0, $t0, $t1
    /* 6EABC 8007E2BC 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EAC0 8007E2C0 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 6EAC4 8007E2C4 0000A8AD */  sw         $t0, 0x0($t5)
    /* 6EAC8 8007E2C8 0400AAAD */  sw         $t2, 0x4($t5)
    /* 6EACC 8007E2CC 0800E003 */  jr         $ra
    /* 6EAD0 8007E2D0 00000000 */   nop
endlabel func_8007E24C
