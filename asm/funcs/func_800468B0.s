glabel func_800468B0
    /* 370B0 800468B0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 370B4 800468B4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 370B8 800468B8 21288000 */  addu       $a1, $a0, $zero
    /* 370BC 800468BC 4415010C */  jal        func_80045510
    /* 370C0 800468C0 08000424 */   addiu     $a0, $zero, 0x8
    /* 370C4 800468C4 8C14010C */  jal        func_80045230
    /* 370C8 800468C8 21200000 */   addu      $a0, $zero, $zero
    /* 370CC 800468CC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 370D0 800468D0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 370D4 800468D4 0800E003 */  jr         $ra
    /* 370D8 800468D8 00000000 */   nop
endlabel func_800468B0
