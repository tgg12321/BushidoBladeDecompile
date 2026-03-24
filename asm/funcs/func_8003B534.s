glabel func_8003B534
    /* 2BD34 8003B534 06000224 */  addiu      $v0, $zero, 0x6
    /* 2BD38 8003B538 0A80013C */  lui        $at, %hi(D_800A37B0)
    /* 2BD3C 8003B53C B03724A0 */  sb         $a0, %lo(D_800A37B0)($at)
    /* 2BD40 8003B540 80200400 */  sll        $a0, $a0, 2
    /* 2BD44 8003B544 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2BD48 8003B548 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2BD4C 8003B54C 0A80023C */  lui        $v0, %hi(D_800A3894)
    /* 2BD50 8003B550 9438428C */  lw         $v0, %lo(D_800A3894)($v0)
    /* 2BD54 8003B554 01008424 */  addiu      $a0, $a0, 0x1
    /* 2BD58 8003B558 21104400 */  addu       $v0, $v0, $a0
    /* 2BD5C 8003B55C 0A80013C */  lui        $at, %hi(D_800A3878)
    /* 2BD60 8003B560 783822AC */  sw         $v0, %lo(D_800A3878)($at)
    /* 2BD64 8003B564 0800E003 */  jr         $ra
    /* 2BD68 8003B568 00000000 */   nop
endlabel func_8003B534
