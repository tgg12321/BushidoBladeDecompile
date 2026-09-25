glabel func_80046DA8
    /* 375A8 80046DA8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 375AC 80046DAC 01008430 */  andi       $a0, $a0, 0x1
    /* 375B0 80046DB0 04008010 */  beqz       $a0, .L80046DC4
    /* 375B4 80046DB4 1000BFAF */   sw        $ra, 0x10($sp)
    /* 375B8 80046DB8 F402848F */  lw         $a0, %gp_rel(D_800A33C0)($gp)
    /* 375BC 80046DBC A81B010C */  jal        func_80046EA0
    /* 375C0 80046DC0 00000000 */   nop
  .L80046DC4:
    /* 375C4 80046DC4 4708010C */  jal        func_8004211C
    /* 375C8 80046DC8 00000000 */   nop
    /* 375CC 80046DCC 2F11010C */  jal        func_800444BC
    /* 375D0 80046DD0 00000000 */   nop
    /* 375D4 80046DD4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 375D8 80046DD8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 375DC 80046DDC 0800E003 */  jr         $ra
    /* 375E0 80046DE0 00000000 */   nop
endlabel func_80046DA8
