glabel func_80065590
    /* 55D90 80065590 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 55D94 80065594 1000BFAF */  sw         $ra, 0x10($sp)
    /* 55D98 80065598 0096010C */  jal        func_80065800
    /* 55D9C 8006559C 07000424 */   addiu     $a0, $zero, 0x7
    /* 55DA0 800655A0 0F80043C */  lui        $a0, %hi(D_800F0BB6)
    /* 55DA4 800655A4 B60B8424 */  addiu      $a0, $a0, %lo(D_800F0BB6)
    /* 55DA8 800655A8 00008394 */  lhu        $v1, 0x0($a0)
    /* 55DAC 800655AC 00000000 */  nop
    /* 55DB0 800655B0 32006324 */  addiu      $v1, $v1, 0x32
    /* 55DB4 800655B4 000083A4 */  sh         $v1, 0x0($a0)
    /* 55DB8 800655B8 001C0300 */  sll        $v1, $v1, 16
    /* 55DBC 800655BC 031C0300 */  sra        $v1, $v1, 16
    /* 55DC0 800655C0 00016328 */  slti       $v1, $v1, 0x100
    /* 55DC4 800655C4 02006014 */  bnez       $v1, .L800655D0
    /* 55DC8 800655C8 FF004230 */   andi      $v0, $v0, 0xFF
    /* 55DCC 800655CC 21100000 */  addu       $v0, $zero, $zero
  .L800655D0:
    /* 55DD0 800655D0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 55DD4 800655D4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 55DD8 800655D8 0800E003 */  jr         $ra
    /* 55DDC 800655DC 00000000 */   nop
endlabel func_80065590
