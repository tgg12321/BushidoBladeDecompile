glabel func_800676C8
    /* 57EC8 800676C8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 57ECC 800676CC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 57ED0 800676D0 21200000 */  addu       $a0, $zero, $zero
    /* 57ED4 800676D4 2A9E010C */  jal        func_800678A8
    /* 57ED8 800676D8 21280000 */   addu      $a1, $zero, $zero
    /* 57EDC 800676DC 21200000 */  addu       $a0, $zero, $zero
    /* 57EE0 800676E0 459F010C */  jal        func_80067D14
    /* 57EE4 800676E4 21280000 */   addu      $a1, $zero, $zero
    /* 57EE8 800676E8 21200000 */  addu       $a0, $zero, $zero
    /* 57EEC 800676EC 62A3010C */  jal        func_80068D88
    /* 57EF0 800676F0 21280000 */   addu      $a1, $zero, $zero
    /* 57EF4 800676F4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 57EF8 800676F8 FF004230 */  andi       $v0, $v0, 0xFF
    /* 57EFC 800676FC 0800E003 */  jr         $ra
    /* 57F00 80067700 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_800676C8
