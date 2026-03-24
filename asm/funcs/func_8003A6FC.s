glabel func_8003A6FC
    /* 2AEFC 8003A6FC 21280000 */  addu       $a1, $zero, $zero
    /* 2AF00 8003A700 21180000 */  addu       $v1, $zero, $zero
  .L8003A704:
    /* 2AF04 8003A704 06106400 */  srlv       $v0, $a0, $v1
    /* 2AF08 8003A708 01004230 */  andi       $v0, $v0, 0x1
    /* 2AF0C 8003A70C 2128A200 */  addu       $a1, $a1, $v0
    /* 2AF10 8003A710 01006324 */  addiu      $v1, $v1, 0x1
    /* 2AF14 8003A714 20006228 */  slti       $v0, $v1, 0x20
    /* 2AF18 8003A718 FAFF4014 */  bnez       $v0, .L8003A704
    /* 2AF1C 8003A71C 00000000 */   nop
    /* 2AF20 8003A720 0800E003 */  jr         $ra
    /* 2AF24 8003A724 2110A000 */   addu      $v0, $a1, $zero
endlabel func_8003A6FC
