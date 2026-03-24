glabel func_80077B00
    /* 68300 80077B00 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 68304 80077B04 1000BFAF */  sw         $ra, 0x10($sp)
    /* 68308 80077B08 AAB8010C */  jal        func_8006E2A8
    /* 6830C 80077B0C 00000000 */   nop
    /* 68310 80077B10 1000BF8F */  lw         $ra, 0x10($sp)
    /* 68314 80077B14 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 68318 80077B18 0800E003 */  jr         $ra
    /* 6831C 80077B1C 00000000 */   nop
endlabel func_80077B00
