glabel func_8003A2DC
    /* 2AADC 8003A2DC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2AAE0 8003A2E0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2AAE4 8003A2E4 21200000 */  addu       $a0, $zero, $zero
    /* 2AAE8 8003A2E8 21280000 */  addu       $a1, $zero, $zero
    /* 2AAEC 8003A2EC 1931020C */  jal        func_8008C464
    /* 2AAF0 8003A2F0 21300000 */   addu      $a2, $zero, $zero
    /* 2AAF4 8003A2F4 80014230 */  andi       $v0, $v0, 0x180
    /* 2AAF8 8003A2F8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2AAFC 8003A2FC 0100422C */  sltiu      $v0, $v0, 0x1
    /* 2AB00 8003A300 0800E003 */  jr         $ra
    /* 2AB04 8003A304 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_8003A2DC
