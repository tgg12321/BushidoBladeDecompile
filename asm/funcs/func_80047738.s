glabel func_80047738
    /* 37F38 80047738 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 37F3C 8004773C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 37F40 80047740 01000424 */  addiu      $a0, $zero, 0x1
    /* 37F44 80047744 D623010C */  jal        func_80048F58
    /* 37F48 80047748 21280000 */   addu      $a1, $zero, $zero
    /* 37F4C 8004774C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 37F50 80047750 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 37F54 80047754 0800E003 */  jr         $ra
    /* 37F58 80047758 00000000 */   nop
endlabel func_80047738
