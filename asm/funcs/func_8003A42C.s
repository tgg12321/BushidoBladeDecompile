glabel func_8003A42C
    /* 2AC2C 8003A42C 0100023C */  lui        $v0, (0x10000 >> 16)
    /* 2AC30 8003A430 2B104500 */  sltu       $v0, $v0, $a1
    /* 2AC34 8003A434 03004014 */  bnez       $v0, .L8003A444
    /* 2AC38 8003A438 21100000 */   addu      $v0, $zero, $zero
    /* 2AC3C 8003A43C 12E90008 */  j          .L8003A448
    /* 2AC40 8003A440 01000224 */   addiu     $v0, $zero, 0x1
  .L8003A444:
    /* 2AC44 8003A444 600780A3 */  sb         $zero, %gp_rel(D_800A382C)($gp)
  .L8003A448:
    /* 2AC48 8003A448 0800E003 */  jr         $ra
    /* 2AC4C 8003A44C 00000000 */   nop
endlabel func_8003A42C
