glabel func_8007E2D4
    /* 6EAD4 8007E2D4 0000888C */  lw         $t0, 0x0($a0)
    /* 6EAD8 8007E2D8 04008A8C */  lw         $t2, 0x4($a0)
    /* 6EADC 8007E2DC 034C0800 */  sra        $t1, $t0, 16
    /* 6EAE0 8007E2E0 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EAE4 8007E2E4 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EAE8 8007E2E8 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EAEC 8007E2EC 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EAF0 8007E2F0 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EAF4 8007E2F4 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EAF8 8007E2F8 00000000 */  nop
    /* 6EAFC 8007E2FC 3D00904B */  gpf        0
    /* 6EB00 8007E300 0000A88C */  lw         $t0, 0x0($a1)
    /* 6EB04 8007E304 0400AA8C */  lw         $t2, 0x4($a1)
    /* 6EB08 8007E308 034C0800 */  sra        $t1, $t0, 16
    /* 6EB0C 8007E30C FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EB10 8007E310 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EB14 8007E314 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EB18 8007E318 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EB1C 8007E31C 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EB20 8007E320 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EB24 8007E324 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EB28 8007E328 00000000 */  nop
    /* 6EB2C 8007E32C 3E00A04B */  gpl        0
    /* 6EB30 8007E330 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6EB34 8007E334 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6EB38 8007E338 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EB3C 8007E33C 004C0900 */  sll        $t1, $t1, 16
    /* 6EB40 8007E340 25400901 */  or         $t0, $t0, $t1
    /* 6EB44 8007E344 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EB48 8007E348 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 6EB4C 8007E34C 0000A8AD */  sw         $t0, 0x0($t5)
    /* 6EB50 8007E350 0400AAAD */  sw         $t2, 0x4($t5)
    /* 6EB54 8007E354 0800E003 */  jr         $ra
    /* 6EB58 8007E358 00000000 */   nop
endlabel func_8007E2D4
