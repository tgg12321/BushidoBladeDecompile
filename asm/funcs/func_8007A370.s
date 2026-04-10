glabel func_8007A370
    /* 6AB70 8007A370 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6AB74 8007A374 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6AB78 8007A378 21808000 */  addu       $s0, $a0, $zero
    /* 6AB7C 8007A37C 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6AB80 8007A380 96E2010C */  jal        func_80078A58
    /* 6AB84 8007A384 21200000 */   addu      $a0, $zero, $zero
    /* 6AB88 8007A388 6EE2010C */  jal        EnterCriticalSection
    /* 6AB8C 8007A38C 00000000 */   nop
    /* 6AB90 8007A390 FCE2010C */  jal        func_80078BF0
    /* 6AB94 8007A394 00000000 */   nop
    /* 6AB98 8007A398 02004014 */  bnez       $v0, .L8007A3A4
    /* 6AB9C 8007A39C 00000000 */   nop
    /* 6ABA0 8007A3A0 21800000 */  addu       $s0, $zero, $zero
  .L8007A3A4:
    /* 6ABA4 8007A3A4 0AE9010C */  jal        func_8007A428
    /* 6ABA8 8007A3A8 21200002 */   addu      $a0, $s0, $zero
    /* 6ABAC 8007A3AC 72E2010C */  jal        ExitCriticalSection
    /* 6ABB0 8007A3B0 00000000 */   nop
    /* 6ABB4 8007A3B4 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6ABB8 8007A3B8 1000B08F */  lw         $s0, 0x10($sp)
    /* 6ABBC 8007A3BC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6ABC0 8007A3C0 0800E003 */  jr         $ra
    /* 6ABC4 8007A3C4 00000000 */   nop
endlabel func_8007A370
