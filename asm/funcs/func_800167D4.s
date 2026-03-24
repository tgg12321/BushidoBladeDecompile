glabel func_800167D4
    /* 6FD4 800167D4 1080023C */  lui        $v0, %hi(D_80106A73)
    /* 6FD8 800167D8 736A4290 */  lbu        $v0, %lo(D_80106A73)($v0)
    /* 6FDC 800167DC 00000000 */  nop
    /* 6FE0 800167E0 82100200 */  srl        $v0, $v0, 2
    /* 6FE4 800167E4 0800E003 */  jr         $ra
    /* 6FE8 800167E8 01004230 */   andi      $v0, $v0, 0x1
endlabel func_800167D4
