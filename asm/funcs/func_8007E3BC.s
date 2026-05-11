glabel func_8007E3BC
    /* 6EBBC 8007E3BC 00008890 */  lbu        $t0, 0x0($a0)
    /* 6EBC0 8007E3C0 01008990 */  lbu        $t1, 0x1($a0)
    /* 6EBC4 8007E3C4 02008A90 */  lbu        $t2, 0x2($a0)
    /* 6EBC8 8007E3C8 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EBCC 8007E3CC 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EBD0 8007E3D0 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EBD4 8007E3D4 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EBD8 8007E3D8 00000000 */  nop
    /* 6EBDC 8007E3DC 3D00904B */  gpf        0
    /* 6EBE0 8007E3E0 0000A890 */  lbu        $t0, 0x0($a1)
    /* 6EBE4 8007E3E4 0100A990 */  lbu        $t1, 0x1($a1)
    /* 6EBE8 8007E3E8 0200AA90 */  lbu        $t2, 0x2($a1)
    /* 6EBEC 8007E3EC 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EBF0 8007E3F0 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EBF4 8007E3F4 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EBF8 8007E3F8 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EBFC 8007E3FC 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EC00 8007E400 0C000B24 */  addiu      $t3, $zero, 0xC
    /* 6EC04 8007E404 3E00A04B */  gpl        0
    /* 6EC08 8007E408 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EC0C 8007E40C 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6EC10 8007E410 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6EC14 8007E414 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 6EC18 8007E418 07406801 */  srav       $t0, $t0, $t3
    /* 6EC1C 8007E41C 07486901 */  srav       $t1, $t1, $t3
    /* 6EC20 8007E420 07506A01 */  srav       $t2, $t2, $t3
    /* 6EC24 8007E424 0000A8A1 */  sb         $t0, 0x0($t5)
    /* 6EC28 8007E428 0100A9A1 */  sb         $t1, 0x1($t5)
    /* 6EC2C 8007E42C 0200AAA1 */  sb         $t2, 0x2($t5)
    /* 6EC30 8007E430 0800E003 */  jr         $ra
    /* 6EC34 8007E434 00000000 */   nop
endlabel func_8007E3BC
