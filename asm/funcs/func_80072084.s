glabel func_80072084
    /* 62884 80072084 DC04828F */  lw         $v0, %gp_rel(D_800A35A8)($gp)
    /* 62888 80072088 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6288C 8007208C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 62890 80072090 6800458C */  lw         $a1, 0x68($v0)
    /* 62894 80072094 3FC8010C */  jal        func_800720FC
    /* 62898 80072098 21300000 */   addu      $a2, $zero, $zero
    /* 6289C 8007209C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 628A0 800720A0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 628A4 800720A4 0800E003 */  jr         $ra
    /* 628A8 800720A8 00000000 */   nop
endlabel func_80072084
