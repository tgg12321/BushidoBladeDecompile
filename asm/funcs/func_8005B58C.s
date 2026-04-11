glabel func_8005B58C
    /* 4BD8C 8005B58C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 4BD90 8005B590 1000BFAF */  sw         $ra, 0x10($sp)
    /* 4BD94 8005B594 3416020C */  jal        title_mv_exec2
    /* 4BD98 8005B598 21200000 */   addu      $a0, $zero, $zero
    /* 4BD9C 8005B59C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 4BDA0 8005B5A0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 4BDA4 8005B5A4 0800E003 */  jr         $ra
    /* 4BDA8 8005B5A8 00000000 */   nop
endlabel func_8005B58C
