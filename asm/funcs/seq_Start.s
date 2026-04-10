glabel seq_Start
    /* 358BC 800450BC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 358C0 800450C0 1000B0AF */  sw         $s0, 0x10($sp)
    /* 358C4 800450C4 2180A000 */  addu       $s0, $a1, $zero
    /* 358C8 800450C8 1400BFAF */  sw         $ra, 0x14($sp)
    /* 358CC 800450CC 9D13010C */  jal        func_80044E74
    /* 358D0 800450D0 25008424 */   addiu     $a0, $a0, 0x25
    /* 358D4 800450D4 01000224 */  addiu      $v0, $zero, 0x1
    /* 358D8 800450D8 CC0290AF */  sw         $s0, %gp_rel(D_800A3398)($gp)
    /* 358DC 800450DC 780182AF */  sw         $v0, %gp_rel(D_800A3244)($gp)
    /* 358E0 800450E0 1400BF8F */  lw         $ra, 0x14($sp)
    /* 358E4 800450E4 1000B08F */  lw         $s0, 0x10($sp)
    /* 358E8 800450E8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 358EC 800450EC 0800E003 */  jr         $ra
    /* 358F0 800450F0 00000000 */   nop
endlabel seq_Start
