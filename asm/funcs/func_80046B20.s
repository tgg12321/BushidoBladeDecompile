glabel func_80046B20
    /* 37320 80046B20 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 37324 80046B24 1000BFAF */  sw         $ra, 0x10($sp)
    /* 37328 80046B28 00F2043C */  lui        $a0, (0xF2000001 >> 16)
    /* 3732C 80046B2C C1E2010C */  jal        func_80078B04
    /* 37330 80046B30 01008434 */   ori       $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 37334 80046B34 1000BF8F */  lw         $ra, 0x10($sp)
    /* 37338 80046B38 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 3733C 80046B3C 0800E003 */  jr         $ra
    /* 37340 80046B40 00000000 */   nop
endlabel func_80046B20
