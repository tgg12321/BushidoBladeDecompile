glabel func_80080640
    /* 70E40 80080640 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70E44 80080644 1000BFAF */  sw         $ra, 0x10($sp)
    /* 70E48 80080648 8707020C */  jal        func_80081E1C
    /* 70E4C 8008064C 00000000 */   nop
    /* 70E50 80080650 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70E54 80080654 0100422C */  sltiu      $v0, $v0, 0x1
    /* 70E58 80080658 0800E003 */  jr         $ra
    /* 70E5C 8008065C 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80080640
