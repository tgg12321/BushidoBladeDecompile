glabel func_8007E1AC
    /* 6E9AC 8007E1AC 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6E9B0 8007E1B0 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6E9B4 8007E1B4 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6E9B8 8007E1B8 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6E9BC 8007E1BC 00000000 */  nop
    /* 6E9C0 8007E1C0 3D00984B */  gpf        1
    /* 6E9C4 8007E1C4 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6E9C8 8007E1C8 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6E9CC 8007E1CC 0000A9C8 */  lwc2       $9, 0x0($a1)
    /* 6E9D0 8007E1D0 0400AAC8 */  lwc2       $10, 0x4($a1)
    /* 6E9D4 8007E1D4 0800ABC8 */  lwc2       $11, 0x8($a1)
    /* 6E9D8 8007E1D8 00000000 */  nop
    /* 6E9DC 8007E1DC 3E00A84B */  gpl        1
    /* 6E9E0 8007E1E0 1000A88F */  lw         $t0, 0x10($sp)
    /* 6E9E4 8007E1E4 00000000 */  nop
    /* 6E9E8 8007E1E8 000009E9 */  swc2       $9, 0x0($t0)
    /* 6E9EC 8007E1EC 04000AE9 */  swc2       $10, 0x4($t0)
    /* 6E9F0 8007E1F0 08000BE9 */  swc2       $11, 0x8($t0)
    /* 6E9F4 8007E1F4 0800E003 */  jr         $ra
    /* 6E9F8 8007E1F8 00000000 */   nop
endlabel func_8007E1AC
