glabel func_80047550
    /* 37D50 80047550 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 37D54 80047554 1000BFAF */  sw         $ra, 0x10($sp)
    /* 37D58 80047558 A41E010C */  jal        saEft03Start
    /* 37D5C 8004755C 00000000 */   nop
    /* 37D60 80047560 1000BF8F */  lw         $ra, 0x10($sp)
    /* 37D64 80047564 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 37D68 80047568 0800E003 */  jr         $ra
    /* 37D6C 8004756C 00000000 */   nop
endlabel func_80047550
