glabel func_800468DC
    /* 370DC 800468DC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 370E0 800468E0 1000B0AF */  sw         $s0, 0x10($sp)
    /* 370E4 800468E4 2180A000 */  addu       $s0, $a1, $zero
    /* 370E8 800468E8 1400BFAF */  sw         $ra, 0x14($sp)
    /* 370EC 800468EC E322010C */  jal        func_80048B8C
    /* 370F0 800468F0 21200002 */   addu      $a0, $s0, $zero
    /* 370F4 800468F4 09000424 */  addiu      $a0, $zero, 0x9
    /* 370F8 800468F8 4010010C */  jal        func_80044100
    /* 370FC 800468FC 21280002 */   addu      $a1, $s0, $zero
    /* 37100 80046900 1400BF8F */  lw         $ra, 0x14($sp)
    /* 37104 80046904 1000B08F */  lw         $s0, 0x10($sp)
    /* 37108 80046908 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 3710C 8004690C 0800E003 */  jr         $ra
    /* 37110 80046910 00000000 */   nop
endlabel func_800468DC
