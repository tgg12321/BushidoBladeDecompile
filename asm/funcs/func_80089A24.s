glabel func_80089A24
    /* 7A224 80089A24 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7A228 80089A28 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7A22C 80089A2C CA000624 */  addiu      $a2, $zero, 0xCA
    /* 7A230 80089A30 9226020C */  jal        func_80089A48
    /* 7A234 80089A34 CB000724 */   addiu     $a3, $zero, 0xCB
    /* 7A238 80089A38 1000BF8F */  lw         $ra, 0x10($sp)
    /* 7A23C 80089A3C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7A240 80089A40 0800E003 */  jr         $ra
    /* 7A244 80089A44 00000000 */   nop
endlabel func_80089A24
