glabel func_80077A28
    /* 68228 80077A28 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6822C 80077A2C 01000424 */  addiu      $a0, $zero, 0x1
    /* 68230 80077A30 21280000 */  addu       $a1, $zero, $zero
    /* 68234 80077A34 21300000 */  addu       $a2, $zero, $zero
    /* 68238 80077A38 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6823C 80077A3C 180580AF */  sw         $zero, %gp_rel(D_800A35E4)($gp)
    /* 68240 80077A40 DA59000C */  jal        func_80016768
    /* 68244 80077A44 21380000 */   addu      $a3, $zero, $zero
    /* 68248 80077A48 FFB5010C */  jal        func_8006D7FC
    /* 6824C 80077A4C 00000000 */   nop
    /* 68250 80077A50 1000BF8F */  lw         $ra, 0x10($sp)
    /* 68254 80077A54 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 68258 80077A58 0800E003 */  jr         $ra
    /* 6825C 80077A5C 00000000 */   nop
endlabel func_80077A28
