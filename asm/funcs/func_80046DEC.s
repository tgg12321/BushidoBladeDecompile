glabel func_80046DEC
    /* 375EC 80046DEC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 375F0 80046DF0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 375F4 80046DF4 4F05010C */  jal        func_8004153C
    /* 375F8 80046DF8 00000000 */   nop
    /* 375FC 80046DFC 02004014 */  bnez       $v0, .L80046E08
    /* 37600 80046E00 94194224 */   addiu     $v0, $v0, 0x1994
    /* 37604 80046E04 21100000 */  addu       $v0, $zero, $zero
  .L80046E08:
    /* 37608 80046E08 1000BF8F */  lw         $ra, 0x10($sp)
    /* 3760C 80046E0C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 37610 80046E10 0800E003 */  jr         $ra
    /* 37614 80046E14 00000000 */   nop
endlabel func_80046DEC
