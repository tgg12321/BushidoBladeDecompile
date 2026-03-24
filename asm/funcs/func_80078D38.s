glabel func_80078D38
    /* 69538 80078D38 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6953C 80078D3C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 69540 80078D40 C4E3010C */  jal        func_80078F10
    /* 69544 80078D44 00000000 */   nop
    /* 69548 80078D48 96E2010C */  jal        func_80078A58
    /* 6954C 80078D4C 21200000 */   addu      $a0, $zero, $zero
    /* 69550 80078D50 D8E3010C */  jal        func_80078F60
    /* 69554 80078D54 00000000 */   nop
    /* 69558 80078D58 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6955C 80078D5C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 69560 80078D60 0800E003 */  jr         $ra
    /* 69564 80078D64 00000000 */   nop
endlabel func_80078D38
