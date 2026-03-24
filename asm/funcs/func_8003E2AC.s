glabel func_8003E2AC
    /* 2EAAC 8003E2AC 0F80033C */  lui        $v1, %hi(D_800F6656)
    /* 2EAB0 8003E2B0 56666324 */  addiu      $v1, $v1, %lo(D_800F6656)
    /* 2EAB4 8003E2B4 00006294 */  lhu        $v0, 0x0($v1)
    /* 2EAB8 8003E2B8 00000000 */  nop
    /* 2EABC 8003E2BC FDFF4230 */  andi       $v0, $v0, 0xFFFD
    /* 2EAC0 8003E2C0 0800E003 */  jr         $ra
    /* 2EAC4 8003E2C4 000062A4 */   sh        $v0, 0x0($v1)
endlabel func_8003E2AC
