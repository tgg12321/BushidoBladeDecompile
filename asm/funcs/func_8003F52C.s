glabel stage_SetCollision
    /* 2FD2C 8003F52C 40290500 */  sll        $a1, $a1, 5
    /* 2FD30 8003F530 2128A400 */  addu       $a1, $a1, $a0
    /* 2FD34 8003F534 0300C630 */  andi       $a2, $a2, 0x3
    /* 2FD38 8003F538 0B80013C */  lui        $at, %hi(D_800A8FB0)
    /* 2FD3C 8003F53C 21082500 */  addu       $at, $at, $a1
    /* 2FD40 8003F540 B08F26A0 */  sb         $a2, %lo(D_800A8FB0)($at)
    /* 2FD44 8003F544 0800E003 */  jr         $ra
    /* 2FD48 8003F548 00000000 */   nop
endlabel stage_SetCollision
