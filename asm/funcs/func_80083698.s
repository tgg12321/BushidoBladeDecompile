glabel func_80083698
    /* 73E98 80083698 2130A000 */  addu       $a2, $a1, $zero
    /* 73E9C 8008369C 21288000 */  addu       $a1, $a0, $zero
    /* 73EA0 800836A0 CD400000 */  break      0, 259
    /* 73EA4 800836A4 02004010 */  beqz       $v0, .L800836B0
    /* 73EA8 800836A8 21106000 */   addu      $v0, $v1, $zero
    /* 73EAC 800836AC FFFF0224 */  addiu      $v0, $zero, -0x1
  .L800836B0:
    /* 73EB0 800836B0 0800E003 */  jr         $ra
    /* 73EB4 800836B4 00000000 */   nop
endlabel func_80083698
