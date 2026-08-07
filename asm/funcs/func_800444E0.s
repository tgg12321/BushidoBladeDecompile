glabel func_800444E0
    /* 34CE0 800444E0 C006848F */  lw         $a0, %gp_rel(D_800A378C)($gp)
    /* 34CE4 800444E4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 34CE8 800444E8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 34CEC 800444EC 4111010C */  jal        func_80044504
    /* 34CF0 800444F0 00000000 */   nop
    /* 34CF4 800444F4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 34CF8 800444F8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 34CFC 800444FC 0800E003 */  jr         $ra
    /* 34D00 80044500 00000000 */   nop
endlabel func_800444E0
