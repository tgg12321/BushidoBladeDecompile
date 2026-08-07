glabel func_8003FFC4
    /* 307C4 8003FFC4 2400838C */  lw         $v1, 0x24($a0)
    /* 307C8 8003FFC8 00000000 */  nop
    /* 307CC 8003FFCC 02006010 */  beqz       $v1, .L8003FFD8
    /* 307D0 8003FFD0 01000224 */   addiu     $v0, $zero, 0x1
    /* 307D4 8003FFD4 060062A4 */  sh         $v0, 0x6($v1)
  .L8003FFD8:
    /* 307D8 8003FFD8 0800E003 */  jr         $ra
    /* 307DC 8003FFDC 00000000 */   nop
endlabel func_8003FFC4
