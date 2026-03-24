glabel func_80088584
    /* 78D84 80088584 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 78D88 80088588 1000BFAF */  sw         $ra, 0x10($sp)
    /* 78D8C 8008858C 00240400 */  sll        $a0, $a0, 16
    /* 78D90 80088590 AC2B020C */  jal        func_8008AEB0
    /* 78D94 80088594 03240400 */   sra       $a0, $a0, 16
    /* 78D98 80088598 00140200 */  sll        $v0, $v0, 16
    /* 78D9C 8008859C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 78DA0 800885A0 03140200 */  sra        $v0, $v0, 16
    /* 78DA4 800885A4 0800E003 */  jr         $ra
    /* 78DA8 800885A8 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80088584
