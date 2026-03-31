glabel func_8004748C
    /* 37C8C 8004748C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 37C90 80047490 1000BFAF */  sw         $ra, 0x10($sp)
    /* 37C94 80047494 0012010C */  jal        efc_rob_set_type_flash
    /* 37C98 80047498 00000000 */   nop
    /* 37C9C 8004749C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 37CA0 800474A0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 37CA4 800474A4 0800E003 */  jr         $ra
    /* 37CA8 800474A8 00000000 */   nop
endlabel func_8004748C
