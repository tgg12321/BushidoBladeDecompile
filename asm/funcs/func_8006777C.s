glabel func_8006777C
    /* 57F7C 8006777C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 57F80 80067780 1000BFAF */  sw         $ra, 0x10($sp)
    /* 57F84 80067784 03000424 */  addiu      $a0, $zero, 0x3
    /* 57F88 80067788 2A9E010C */  jal        func_800678A8
    /* 57F8C 8006778C 21280000 */   addu      $a1, $zero, $zero
    /* 57F90 80067790 03000424 */  addiu      $a0, $zero, 0x3
    /* 57F94 80067794 459F010C */  jal        func_80067D14
    /* 57F98 80067798 21280000 */   addu      $a1, $zero, $zero
    /* 57F9C 8006779C 03000424 */  addiu      $a0, $zero, 0x3
    /* 57FA0 800677A0 62A3010C */  jal        func_80068D88
    /* 57FA4 800677A4 21280000 */   addu      $a1, $zero, $zero
    /* 57FA8 800677A8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 57FAC 800677AC FF004230 */  andi       $v0, $v0, 0xFF
    /* 57FB0 800677B0 0800E003 */  jr         $ra
    /* 57FB4 800677B4 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_8006777C
