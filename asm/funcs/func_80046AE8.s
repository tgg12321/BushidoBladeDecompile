glabel func_80046AE8
    /* 372E8 80046AE8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 372EC 80046AEC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 372F0 80046AF0 00F2043C */  lui        $a0, (0xF2000001 >> 16)
    /* 372F4 80046AF4 01008434 */  ori        $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 372F8 80046AF8 FFFF0524 */  addiu      $a1, $zero, -0x1
    /* 372FC 80046AFC 9AE2010C */  jal        func_80078A68
    /* 37300 80046B00 00200624 */   addiu     $a2, $zero, 0x2000
    /* 37304 80046B04 00F2043C */  lui        $a0, (0xF2000001 >> 16)
    /* 37308 80046B08 CFE2010C */  jal        func_80078B3C
    /* 3730C 80046B0C 01008434 */   ori       $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 37310 80046B10 1000BF8F */  lw         $ra, 0x10($sp)
    /* 37314 80046B14 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 37318 80046B18 0800E003 */  jr         $ra
    /* 3731C 80046B1C 00000000 */   nop
endlabel func_80046AE8
