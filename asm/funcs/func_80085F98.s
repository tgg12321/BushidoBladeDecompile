glabel func_80085F98
    /* 76798 80085F98 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7679C 80085F9C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 767A0 80085FA0 5827020C */  jal        saEft03Start2
    /* 767A4 80085FA4 21200000 */   addu      $a0, $zero, $zero
    /* 767A8 80085FA8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 767AC 80085FAC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 767B0 80085FB0 0800E003 */  jr         $ra
    /* 767B4 80085FB4 00000000 */   nop
endlabel func_80085F98
