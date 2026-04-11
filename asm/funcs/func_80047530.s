glabel func_80047530
    /* 37D30 80047530 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 37D34 80047534 1000BFAF */  sw         $ra, 0x10($sp)
    /* 37D38 80047538 FA1D010C */  jal        motion_CheckSituation
    /* 37D3C 8004753C 00000000 */   nop
    /* 37D40 80047540 1000BF8F */  lw         $ra, 0x10($sp)
    /* 37D44 80047544 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 37D48 80047548 0800E003 */  jr         $ra
    /* 37D4C 8004754C 00000000 */   nop
endlabel func_80047530
