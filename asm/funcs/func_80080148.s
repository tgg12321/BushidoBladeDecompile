glabel func_80080148
    /* 70948 80080148 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7094C 8008014C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 70950 80080150 E805020C */  jal        cdrom_ClearIrq
    /* 70954 80080154 00000000 */   nop
    /* 70958 80080158 1000BF8F */  lw         $ra, 0x10($sp)
    /* 7095C 8008015C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 70960 80080160 0800E003 */  jr         $ra
    /* 70964 80080164 00000000 */   nop
endlabel func_80080148
