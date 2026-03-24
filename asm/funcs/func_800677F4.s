glabel func_800677F4
    /* 57FF4 800677F4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 57FF8 800677F8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 57FFC 800677FC 05000424 */  addiu      $a0, $zero, 0x5
    /* 58000 80067800 2A9E010C */  jal        func_800678A8
    /* 58004 80067804 03000524 */   addiu     $a1, $zero, 0x3
    /* 58008 80067808 05000424 */  addiu      $a0, $zero, 0x5
    /* 5800C 8006780C 459F010C */  jal        func_80067D14
    /* 58010 80067810 03000524 */   addiu     $a1, $zero, 0x3
    /* 58014 80067814 05000424 */  addiu      $a0, $zero, 0x5
    /* 58018 80067818 62A3010C */  jal        func_80068D88
    /* 5801C 8006781C 03000524 */   addiu     $a1, $zero, 0x3
    /* 58020 80067820 1000BF8F */  lw         $ra, 0x10($sp)
    /* 58024 80067824 FF004230 */  andi       $v0, $v0, 0xFF
    /* 58028 80067828 0800E003 */  jr         $ra
    /* 5802C 8006782C 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_800677F4
