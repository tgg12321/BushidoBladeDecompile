glabel func_800444BC
    /* 34CBC 800444BC C006848F */  lw         $a0, %gp_rel(D_800A378C)($gp)
    /* 34CC0 800444C0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 34CC4 800444C4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 34CC8 800444C8 4111010C */  jal        func_80044504
    /* 34CCC 800444CC 00000000 */   nop
    /* 34CD0 800444D0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 34CD4 800444D4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 34CD8 800444D8 0800E003 */  jr         $ra
    /* 34CDC 800444DC 00000000 */   nop
endlabel func_800444BC
