glabel func_80044F80
    /* 35780 80044F80 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 35784 80044F84 1000BFAF */  sw         $ra, 0x10($sp)
    /* 35788 80044F88 9D13010C */  jal        func_80044E74
    /* 3578C 80044F8C 4D008424 */   addiu     $a0, $a0, 0x4D
    /* 35790 80044F90 1000BF8F */  lw         $ra, 0x10($sp)
    /* 35794 80044F94 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 35798 80044F98 0800E003 */  jr         $ra
    /* 3579C 80044F9C 00000000 */   nop
endlabel func_80044F80
