glabel func_800355E8
    /* 25DE8 800355E8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 25DEC 800355EC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 25DF0 800355F0 EAD7000C */  jal        func_80035FA8
    /* 25DF4 800355F4 00000000 */   nop
    /* 25DF8 800355F8 44DC000C */  jal        func_80037110
    /* 25DFC 800355FC 01000424 */   addiu     $a0, $zero, 0x1
    /* 25E00 80035600 7ADC000C */  jal        func_800371E8
    /* 25E04 80035604 01000424 */   addiu     $a0, $zero, 0x1
    /* 25E08 80035608 1000BF8F */  lw         $ra, 0x10($sp)
    /* 25E0C 8003560C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 25E10 80035610 0800E003 */  jr         $ra
    /* 25E14 80035614 00000000 */   nop
endlabel func_800355E8
