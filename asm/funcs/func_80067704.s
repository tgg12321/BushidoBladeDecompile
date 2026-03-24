glabel func_80067704
    /* 57F04 80067704 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 57F08 80067708 1000BFAF */  sw         $ra, 0x10($sp)
    /* 57F0C 8006770C 01000424 */  addiu      $a0, $zero, 0x1
    /* 57F10 80067710 2A9E010C */  jal        func_800678A8
    /* 57F14 80067714 01000524 */   addiu     $a1, $zero, 0x1
    /* 57F18 80067718 01000424 */  addiu      $a0, $zero, 0x1
    /* 57F1C 8006771C 459F010C */  jal        func_80067D14
    /* 57F20 80067720 01000524 */   addiu     $a1, $zero, 0x1
    /* 57F24 80067724 01000424 */  addiu      $a0, $zero, 0x1
    /* 57F28 80067728 62A3010C */  jal        func_80068D88
    /* 57F2C 8006772C 01000524 */   addiu     $a1, $zero, 0x1
    /* 57F30 80067730 1000BF8F */  lw         $ra, 0x10($sp)
    /* 57F34 80067734 FF004230 */  andi       $v0, $v0, 0xFF
    /* 57F38 80067738 0800E003 */  jr         $ra
    /* 57F3C 8006773C 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80067704
