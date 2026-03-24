glabel func_8007E8AC
    /* 6F0AC 8007E8AC 0000888C */  lw         $t0, 0x0($a0)
    /* 6F0B0 8007E8B0 0400898C */  lw         $t1, 0x4($a0)
    /* 6F0B4 8007E8B4 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F0B8 8007E8B8 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 6F0BC 8007E8BC 00000000 */  nop
    /* 6F0C0 8007E8C0 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F0C4 8007E8C4 0000A9E8 */  swc2       $9, 0x0($a1)
    /* 6F0C8 8007E8C8 0400AAE8 */  swc2       $10, 0x4($a1)
    /* 6F0CC 8007E8CC 0800ABE8 */  swc2       $11, 0x8($a1)
    /* 6F0D0 8007E8D0 2110C000 */  addu       $v0, $a2, $zero
    /* 6F0D4 8007E8D4 0800E003 */  jr         $ra
    /* 6F0D8 8007E8D8 00000000 */   nop
endlabel func_8007E8AC
