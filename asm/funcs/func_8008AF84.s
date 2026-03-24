glabel func_8008AF84
    /* 7B784 8008AF84 0A80023C */  lui        $v0, %hi(D_800A2D10)
    /* 7B788 8008AF88 102D428C */  lw         $v0, %lo(D_800A2D10)($v0)
    /* 7B78C 8008AF8C 00000000 */  nop
    /* 7B790 8008AF90 01004238 */  xori       $v0, $v0, 0x1
    /* 7B794 8008AF94 0800E003 */  jr         $ra
    /* 7B798 8008AF98 2B100200 */   sltu      $v0, $zero, $v0
endlabel func_8008AF84
