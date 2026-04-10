glabel func_80080600
    /* 70E00 80080600 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70E04 80080604 1000BFAF */  sw         $ra, 0x10($sp)
    /* 70E08 80080608 C605020C */  jal        cdrom_SendCmd
    /* 70E0C 8008060C 00000000 */   nop
    /* 70E10 80080610 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70E14 80080614 01000224 */  addiu      $v0, $zero, 0x1
    /* 70E18 80080618 0800E003 */  jr         $ra
    /* 70E1C 8008061C 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80080600
