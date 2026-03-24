glabel func_80067740
    /* 57F40 80067740 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 57F44 80067744 1000BFAF */  sw         $ra, 0x10($sp)
    /* 57F48 80067748 02000424 */  addiu      $a0, $zero, 0x2
    /* 57F4C 8006774C 2A9E010C */  jal        func_800678A8
    /* 57F50 80067750 01000524 */   addiu     $a1, $zero, 0x1
    /* 57F54 80067754 02000424 */  addiu      $a0, $zero, 0x2
    /* 57F58 80067758 459F010C */  jal        func_80067D14
    /* 57F5C 8006775C 01000524 */   addiu     $a1, $zero, 0x1
    /* 57F60 80067760 02000424 */  addiu      $a0, $zero, 0x2
    /* 57F64 80067764 62A3010C */  jal        func_80068D88
    /* 57F68 80067768 01000524 */   addiu     $a1, $zero, 0x1
    /* 57F6C 8006776C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 57F70 80067770 FF004230 */  andi       $v0, $v0, 0xFF
    /* 57F74 80067774 0800E003 */  jr         $ra
    /* 57F78 80067778 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80067740
