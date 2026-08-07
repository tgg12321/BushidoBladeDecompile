glabel func_800836C8
    /* 73EC8 800836C8 2138C000 */  addu       $a3, $a2, $zero
    /* 73ECC 800836CC 2130A000 */  addu       $a2, $a1, $zero
    /* 73ED0 800836D0 21288000 */  addu       $a1, $a0, $zero
    /* 73ED4 800836D4 CD410000 */  break      0, 263
    /* 73ED8 800836D8 02004010 */  beqz       $v0, .L800836E4
    /* 73EDC 800836DC 21106000 */   addu      $v0, $v1, $zero
    /* 73EE0 800836E0 FFFF0224 */  addiu      $v0, $zero, -0x1
  .L800836E4:
    /* 73EE4 800836E4 0800E003 */  jr         $ra
    /* 73EE8 800836E8 00000000 */   nop
endlabel func_800836C8
