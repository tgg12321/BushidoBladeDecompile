glabel func_80046E18
    /* 37618 80046E18 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 3761C 80046E1C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 37620 80046E20 4F05010C */  jal        func_8004153C
    /* 37624 80046E24 00000000 */   nop
    /* 37628 80046E28 02004014 */  bnez       $v0, .L80046E34
    /* 3762C 80046E2C 2C004224 */   addiu     $v0, $v0, 0x2C
    /* 37630 80046E30 21100000 */  addu       $v0, $zero, $zero
  .L80046E34:
    /* 37634 80046E34 1000BF8F */  lw         $ra, 0x10($sp)
    /* 37638 80046E38 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 3763C 80046E3C 0800E003 */  jr         $ra
    /* 37640 80046E40 00000000 */   nop
endlabel func_80046E18
