glabel func_8007A28C
    /* 6AA8C 8007A28C 2B108500 */  sltu       $v0, $a0, $a1
    /* 6AA90 8007A290 0E004014 */  bnez       $v0, .L8007A2CC
    /* 6AA94 8007A294 2110C000 */   addu      $v0, $a2, $zero
    /* 6AA98 8007A298 15004018 */  blez       $v0, .L8007A2F0
    /* 6AA9C 8007A29C FFFFC624 */   addiu     $a2, $a2, -0x1
    /* 6AAA0 8007A2A0 2138C400 */  addu       $a3, $a2, $a0
    /* 6AAA4 8007A2A4 2128C500 */  addu       $a1, $a2, $a1
  .L8007A2A8:
    /* 6AAA8 8007A2A8 0000A290 */  lbu        $v0, 0x0($a1)
    /* 6AAAC 8007A2AC FFFFA524 */  addiu      $a1, $a1, -0x1
    /* 6AAB0 8007A2B0 2118C000 */  addu       $v1, $a2, $zero
    /* 6AAB4 8007A2B4 FFFFC624 */  addiu      $a2, $a2, -0x1
    /* 6AAB8 8007A2B8 0000E2A0 */  sb         $v0, 0x0($a3)
    /* 6AABC 8007A2BC FAFF601C */  bgtz       $v1, .L8007A2A8
    /* 6AAC0 8007A2C0 FFFFE724 */   addiu     $a3, $a3, -0x1
    /* 6AAC4 8007A2C4 BCE80108 */  j          .L8007A2F0
    /* 6AAC8 8007A2C8 00000000 */   nop
  .L8007A2CC:
    /* 6AACC 8007A2CC 08004018 */  blez       $v0, .L8007A2F0
    /* 6AAD0 8007A2D0 FFFFC624 */   addiu     $a2, $a2, -0x1
  .L8007A2D4:
    /* 6AAD4 8007A2D4 0000A290 */  lbu        $v0, 0x0($a1)
    /* 6AAD8 8007A2D8 0100A524 */  addiu      $a1, $a1, 0x1
    /* 6AADC 8007A2DC 2118C000 */  addu       $v1, $a2, $zero
    /* 6AAE0 8007A2E0 FFFFC624 */  addiu      $a2, $a2, -0x1
    /* 6AAE4 8007A2E4 000082A0 */  sb         $v0, 0x0($a0)
    /* 6AAE8 8007A2E8 FAFF601C */  bgtz       $v1, .L8007A2D4
    /* 6AAEC 8007A2EC 01008424 */   addiu     $a0, $a0, 0x1
  .L8007A2F0:
    /* 6AAF0 8007A2F0 0800E003 */  jr         $ra
    /* 6AAF4 8007A2F4 21108000 */   addu      $v0, $a0, $zero
endlabel func_8007A28C
