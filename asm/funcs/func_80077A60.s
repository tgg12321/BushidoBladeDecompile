glabel func_80077A60
    /* 68260 80077A60 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 68264 80077A64 1000BFAF */  sw         $ra, 0x10($sp)
    /* 68268 80077A68 1AB8010C */  jal        func_8006E068
    /* 6826C 80077A6C 00000000 */   nop
    /* 68270 80077A70 1000BF8F */  lw         $ra, 0x10($sp)
    /* 68274 80077A74 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 68278 80077A78 0800E003 */  jr         $ra
    /* 6827C 80077A7C 00000000 */   nop
endlabel func_80077A60
