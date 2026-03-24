glabel func_8008A904
    /* 7B104 8008A904 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7B108 8008A908 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7B10C 8008A90C CC000624 */  addiu      $a2, $zero, 0xCC
    /* 7B110 8008A910 9226020C */  jal        func_80089A48
    /* 7B114 8008A914 CD000724 */   addiu     $a3, $zero, 0xCD
    /* 7B118 8008A918 1000BF8F */  lw         $ra, 0x10($sp)
    /* 7B11C 8008A91C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7B120 8008A920 0800E003 */  jr         $ra
    /* 7B124 8008A924 00000000 */   nop
endlabel func_8008A904
