glabel func_80045A28
    /* 36228 80045A28 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 3622C 80045A2C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 36230 80045A30 4415010C */  jal        func_80045510
    /* 36234 80045A34 03008424 */   addiu     $a0, $a0, 0x3
    /* 36238 80045A38 8C14010C */  jal        func_80045230
    /* 3623C 80045A3C 21200000 */   addu      $a0, $zero, $zero
    /* 36240 80045A40 1000BF8F */  lw         $ra, 0x10($sp)
    /* 36244 80045A44 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 36248 80045A48 0800E003 */  jr         $ra
    /* 3624C 80045A4C 00000000 */   nop
endlabel func_80045A28
