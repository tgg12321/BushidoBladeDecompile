glabel func_80069A8C
    /* 5A28C 80069A8C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 5A290 80069A90 1000B0AF */  sw         $s0, 0x10($sp)
    /* 5A294 80069A94 1400BFAF */  sw         $ra, 0x14($sp)
    /* 5A298 80069A98 40DF010C */  jal        func_80077D00
    /* 5A29C 80069A9C 21808000 */   addu      $s0, $a0, $zero
    /* 5A2A0 80069AA0 2000428C */  lw         $v0, 0x20($v0)
    /* 5A2A4 80069AA4 00000000 */  nop
    /* 5A2A8 80069AA8 01004230 */  andi       $v0, $v0, 0x1
    /* 5A2AC 80069AAC 04004010 */  beqz       $v0, .L80069AC0
    /* 5A2B0 80069AB0 08000224 */   addiu     $v0, $zero, 0x8
    /* 5A2B4 80069AB4 040002A2 */  sb         $v0, 0x4($s0)
    /* 5A2B8 80069AB8 B3A60108 */  j          .L80069ACC
    /* 5A2BC 80069ABC 050002A2 */   sb        $v0, 0x5($s0)
  .L80069AC0:
    /* 5A2C0 80069AC0 31000224 */  addiu      $v0, $zero, 0x31
    /* 5A2C4 80069AC4 040000A2 */  sb         $zero, 0x4($s0)
    /* 5A2C8 80069AC8 050000A2 */  sb         $zero, 0x5($s0)
  .L80069ACC:
    /* 5A2CC 80069ACC 060002A2 */  sb         $v0, 0x6($s0)
    /* 5A2D0 80069AD0 1400BF8F */  lw         $ra, 0x14($sp)
    /* 5A2D4 80069AD4 1000B08F */  lw         $s0, 0x10($sp)
    /* 5A2D8 80069AD8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 5A2DC 80069ADC 0800E003 */  jr         $ra
    /* 5A2E0 80069AE0 00000000 */   nop
endlabel func_80069A8C
