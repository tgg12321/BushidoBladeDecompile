glabel func_8007D2CC
    /* 6DACC 8007D2CC 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6DAD0 8007D2D0 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6DAD4 8007D2D4 00000000 */  nop
    /* 6DAD8 8007D2D8 000044AC */  sw         $a0, 0x0($v0)
    /* 6DADC 8007D2DC 02160400 */  srl        $v0, $a0, 24
    /* 6DAE0 8007D2E0 0F80013C */  lui        $at, %hi(D_800F189C)
    /* 6DAE4 8007D2E4 21082200 */  addu       $at, $at, $v0
    /* 6DAE8 8007D2E8 9C1824A0 */  sb         $a0, %lo(D_800F189C)($at)
    /* 6DAEC 8007D2EC 0800E003 */  jr         $ra
    /* 6DAF0 8007D2F0 00000000 */   nop
endlabel func_8007D2CC
