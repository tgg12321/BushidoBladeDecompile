glabel func_8007A318
    /* 6AB18 8007A318 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6AB1C 8007A31C 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6AB20 8007A320 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6AB24 8007A324 D8E8010C */  jal        func_8007A360
    /* 6AB28 8007A328 21808000 */   addu      $s0, $a0, $zero
    /* 6AB2C 8007A32C 21200002 */  addu       $a0, $s0, $zero
    /* 6AB30 8007A330 3F000524 */  addiu      $a1, $zero, 0x3F
    /* 6AB34 8007A334 D4E8010C */  jal        func_8007A350
    /* 6AB38 8007A338 21300000 */   addu      $a2, $zero, $zero
    /* 6AB3C 8007A33C 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6AB40 8007A340 1000B08F */  lw         $s0, 0x10($sp)
    /* 6AB44 8007A344 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6AB48 8007A348 0800E003 */  jr         $ra
    /* 6AB4C 8007A34C 00000000 */   nop
endlabel func_8007A318
