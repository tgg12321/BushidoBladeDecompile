glabel func_8007E35C
    /* 6EB5C 8007E35C 00008890 */  lbu        $t0, 0x0($a0)
    /* 6EB60 8007E360 01008990 */  lbu        $t1, 0x1($a0)
    /* 6EB64 8007E364 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EB68 8007E368 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EB6C 8007E36C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EB70 8007E370 00000000 */  nop
    /* 6EB74 8007E374 3D00904B */  gpf        0
    /* 6EB78 8007E378 0000A890 */  lbu        $t0, 0x0($a1)
    /* 6EB7C 8007E37C 0100A990 */  lbu        $t1, 0x1($a1)
    /* 6EB80 8007E380 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EB84 8007E384 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EB88 8007E388 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EB8C 8007E38C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EB90 8007E390 0C000B24 */  addiu      $t3, $zero, 0xC
    /* 6EB94 8007E394 3E00A04B */  gpl        0
    /* 6EB98 8007E398 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EB9C 8007E39C 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6EBA0 8007E3A0 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6EBA4 8007E3A4 07406801 */  srav       $t0, $t0, $t3
    /* 6EBA8 8007E3A8 07486901 */  srav       $t1, $t1, $t3
    /* 6EBAC 8007E3AC 0000A8A1 */  sb         $t0, 0x0($t5)
    /* 6EBB0 8007E3B0 0100A9A1 */  sb         $t1, 0x1($t5)
    /* 6EBB4 8007E3B4 0800E003 */  jr         $ra
    /* 6EBB8 8007E3B8 00000000 */   nop
endlabel func_8007E35C
