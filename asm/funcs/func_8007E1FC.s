glabel func_8007E1FC
    /* 6E9FC 8007E1FC 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EA00 8007E200 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6EA04 8007E204 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6EA08 8007E208 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6EA0C 8007E20C 00000000 */  nop
    /* 6EA10 8007E210 3D00904B */  gpf        0
    /* 6EA14 8007E214 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EA18 8007E218 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EA1C 8007E21C 0000A9C8 */  lwc2       $9, 0x0($a1)
    /* 6EA20 8007E220 0400AAC8 */  lwc2       $10, 0x4($a1)
    /* 6EA24 8007E224 0800ABC8 */  lwc2       $11, 0x8($a1)
    /* 6EA28 8007E228 00000000 */  nop
    /* 6EA2C 8007E22C 3E00A04B */  gpl        0
    /* 6EA30 8007E230 1000A88F */  lw         $t0, 0x10($sp)
    /* 6EA34 8007E234 00000000 */  nop
    /* 6EA38 8007E238 000009E9 */  swc2       $9, 0x0($t0)
    /* 6EA3C 8007E23C 04000AE9 */  swc2       $10, 0x4($t0)
    /* 6EA40 8007E240 08000BE9 */  swc2       $11, 0x8($t0)
    /* 6EA44 8007E244 0800E003 */  jr         $ra
    /* 6EA48 8007E248 00000000 */   nop
endlabel func_8007E1FC
