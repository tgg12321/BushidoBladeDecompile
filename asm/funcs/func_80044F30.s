glabel func_80044F30
    /* 35730 80044F30 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 35734 80044F34 1000BFAF */  sw         $ra, 0x10($sp)
    /* 35738 80044F38 9D13010C */  jal        func_80044E74
    /* 3573C 80044F3C 27008424 */   addiu     $a0, $a0, 0x27
    /* 35740 80044F40 1000BF8F */  lw         $ra, 0x10($sp)
    /* 35744 80044F44 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 35748 80044F48 0800E003 */  jr         $ra
    /* 3574C 80044F4C 00000000 */   nop
endlabel func_80044F30
