glabel func_8007B33C
    /* 6BB3C 8007B33C 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6BB40 8007B340 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6BB44 8007B344 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6BB48 8007B348 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BB4C 8007B34C 21808000 */  addu       $s0, $a0, $zero
    /* 6BB50 8007B350 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6BB54 8007B354 08004014 */  bnez       $v0, .L8007B378
    /* 6BB58 8007B358 1400BFAF */   sw        $ra, 0x14($sp)
    /* 6BB5C 8007B35C 0180043C */  lui        $a0, %hi(D_80015F18)
    /* 6BB60 8007B360 185F8424 */  addiu      $a0, $a0, %lo(D_80015F18)
    /* 6BB64 8007B364 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6BB68 8007B368 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6BB6C 8007B36C 00000000 */  nop
    /* 6BB70 8007B370 09F84000 */  jalr       $v0
    /* 6BB74 8007B374 21280002 */   addu      $a1, $s0, $zero
  .L8007B378:
    /* 6BB78 8007B378 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6BB7C 8007B37C 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6BB80 8007B380 00000000 */  nop
    /* 6BB84 8007B384 3C00428C */  lw         $v0, 0x3C($v0)
    /* 6BB88 8007B388 00000000 */  nop
    /* 6BB8C 8007B38C 09F84000 */  jalr       $v0
    /* 6BB90 8007B390 21200002 */   addu      $a0, $s0, $zero
    /* 6BB94 8007B394 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6BB98 8007B398 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BB9C 8007B39C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6BBA0 8007B3A0 0800E003 */  jr         $ra
    /* 6BBA4 8007B3A4 00000000 */   nop
endlabel func_8007B33C
