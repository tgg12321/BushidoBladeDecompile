glabel func_8003B56C
    /* 2BD6C 8003B56C 06000224 */  addiu      $v0, $zero, 0x6
    /* 2BD70 8003B570 0A80013C */  lui        $at, %hi(D_800A390C)
    /* 2BD74 8003B574 0C3924A0 */  sb         $a0, %lo(D_800A390C)($at)
    /* 2BD78 8003B578 80200400 */  sll        $a0, $a0, 2
    /* 2BD7C 8003B57C 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2BD80 8003B580 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2BD84 8003B584 0A80023C */  lui        $v0, %hi(D_800A385C)
    /* 2BD88 8003B588 5C38428C */  lw         $v0, %lo(D_800A385C)($v0)
    /* 2BD8C 8003B58C FCFF8424 */  addiu      $a0, $a0, -0x4
    /* 2BD90 8003B590 21104400 */  addu       $v0, $v0, $a0
    /* 2BD94 8003B594 0A80013C */  lui        $at, %hi(D_800A3878)
    /* 2BD98 8003B598 783822AC */  sw         $v0, %lo(D_800A3878)($at)
    /* 2BD9C 8003B59C 0800E003 */  jr         $ra
    /* 2BDA0 8003B5A0 00000000 */   nop
endlabel func_8003B56C
