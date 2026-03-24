glabel func_8007AC84
    /* 6B484 8007AC84 03008290 */  lbu        $v0, 0x3($a0)
    /* 6B488 8007AC88 0300A390 */  lbu        $v1, 0x3($a1)
    /* 6B48C 8007AC8C 00000000 */  nop
    /* 6B490 8007AC90 21104300 */  addu       $v0, $v0, $v1
    /* 6B494 8007AC94 01004324 */  addiu      $v1, $v0, 0x1
    /* 6B498 8007AC98 11006228 */  slti       $v0, $v1, 0x11
    /* 6B49C 8007AC9C 04004010 */  beqz       $v0, .L8007ACB0
    /* 6B4A0 8007ACA0 21100000 */   addu      $v0, $zero, $zero
    /* 6B4A4 8007ACA4 030083A0 */  sb         $v1, 0x3($a0)
    /* 6B4A8 8007ACA8 2DEB0108 */  j          .L8007ACB4
    /* 6B4AC 8007ACAC 0000A0AC */   sw        $zero, 0x0($a1)
  .L8007ACB0:
    /* 6B4B0 8007ACB0 FFFF0224 */  addiu      $v0, $zero, -0x1
  .L8007ACB4:
    /* 6B4B4 8007ACB4 0800E003 */  jr         $ra
    /* 6B4B8 8007ACB8 00000000 */   nop
endlabel func_8007AC84
