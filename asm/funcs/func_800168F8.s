glabel func_800168F8
    /* 70F8 800168F8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70FC 800168FC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7100 80016900 BA1A010C */  jal        func_80046AE8
    /* 7104 80016904 00000000 */   nop
    /* 7108 80016908 1000BF8F */  lw         $ra, 0x10($sp)
    /* 710C 8001690C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7110 80016910 0800E003 */  jr         $ra
    /* 7114 80016914 00000000 */   nop
endlabel func_800168F8
