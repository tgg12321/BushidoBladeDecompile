glabel func_8007A400
    /* 6AC00 8007A400 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6AC04 8007A404 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6AC08 8007A408 12E9010C */  jal        func_8007A448
    /* 6AC0C 8007A40C 00000000 */   nop
    /* 6AC10 8007A410 16E9010C */  jal        func_8007A458
    /* 6AC14 8007A414 00000000 */   nop
    /* 6AC18 8007A418 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6AC1C 8007A41C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6AC20 8007A420 0800E003 */  jr         $ra
    /* 6AC24 8007A424 00000000 */   nop
endlabel func_8007A400
