glabel func_80046A80
    /* 37280 80046A80 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 37284 80046A84 1000BFAF */  sw         $ra, 0x10($sp)
    /* 37288 80046A88 F753010C */  jal        func_80054FDC
    /* 3728C 80046A8C 2120A000 */   addu      $a0, $a1, $zero
    /* 37290 80046A90 1000BF8F */  lw         $ra, 0x10($sp)
    /* 37294 80046A94 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 37298 80046A98 0800E003 */  jr         $ra
    /* 3729C 80046A9C 00000000 */   nop
endlabel func_80046A80
