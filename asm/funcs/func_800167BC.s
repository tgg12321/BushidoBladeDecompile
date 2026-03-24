glabel func_800167BC
    /* 6FBC 800167BC 1080023C */  lui        $v0, %hi(D_80106A73)
    /* 6FC0 800167C0 736A4290 */  lbu        $v0, %lo(D_80106A73)($v0)
    /* 6FC4 800167C4 00000000 */  nop
    /* 6FC8 800167C8 42100200 */  srl        $v0, $v0, 1
    /* 6FCC 800167CC 0800E003 */  jr         $ra
    /* 6FD0 800167D0 01004230 */   andi      $v0, $v0, 0x1
endlabel func_800167BC
