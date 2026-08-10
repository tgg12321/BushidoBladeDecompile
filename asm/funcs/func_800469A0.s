glabel func_800469A0
    /* 371A0 800469A0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 371A4 800469A4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 371A8 800469A8 21288000 */  addu       $a1, $a0, $zero
    /* 371AC 800469AC 4415010C */  jal        func_80045510
    /* 371B0 800469B0 09000424 */   addiu     $a0, $zero, 0x9
    /* 371B4 800469B4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 371B8 800469B8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 371BC 800469BC 0800E003 */  jr         $ra
    /* 371C0 800469C0 00000000 */   nop
endlabel func_800469A0
