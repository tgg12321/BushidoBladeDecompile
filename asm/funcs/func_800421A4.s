glabel func_800421A4
    /* 329A4 800421A4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 329A8 800421A8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 329AC 800421AC 21200000 */  addu       $a0, $zero, $zero
    /* 329B0 800421B0 AC07010C */  jal        decBs0
    /* 329B4 800421B4 21280000 */   addu      $a1, $zero, $zero
    /* 329B8 800421B8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 329BC 800421BC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 329C0 800421C0 0800E003 */  jr         $ra
    /* 329C4 800421C4 00000000 */   nop
endlabel func_800421A4
