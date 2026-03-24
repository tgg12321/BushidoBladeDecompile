glabel func_80064FB4
    /* 557B4 80064FB4 B003838F */  lw         $v1, %gp_rel(D_800A347C)($gp)
    /* 557B8 80064FB8 00000000 */  nop
    /* 557BC 80064FBC 0000628C */  lw         $v0, 0x0($v1)
    /* 557C0 80064FC0 0F80013C */  lui        $at, %hi(D_800F0CD0)
    /* 557C4 80064FC4 D00C22AC */  sw         $v0, %lo(D_800F0CD0)($at)
    /* 557C8 80064FC8 0400628C */  lw         $v0, 0x4($v1)
    /* 557CC 80064FCC 0F80013C */  lui        $at, %hi(D_800F0CD4)
    /* 557D0 80064FD0 D40C22AC */  sw         $v0, %lo(D_800F0CD4)($at)
    /* 557D4 80064FD4 0800638C */  lw         $v1, 0x8($v1)
    /* 557D8 80064FD8 01000224 */  addiu      $v0, $zero, 0x1
    /* 557DC 80064FDC 0F80013C */  lui        $at, %hi(D_800F10F8)
    /* 557E0 80064FE0 F81022AC */  sw         $v0, %lo(D_800F10F8)($at)
    /* 557E4 80064FE4 40000224 */  addiu      $v0, $zero, 0x40
    /* 557E8 80064FE8 0F80013C */  lui        $at, %hi(D_800F0BB0)
    /* 557EC 80064FEC B00B22A4 */  sh         $v0, %lo(D_800F0BB0)($at)
    /* 557F0 80064FF0 0F80013C */  lui        $at, %hi(D_800F0CD8)
    /* 557F4 80064FF4 D80C23AC */  sw         $v1, %lo(D_800F0CD8)($at)
    /* 557F8 80064FF8 0800E003 */  jr         $ra
    /* 557FC 80064FFC 01000224 */   addiu     $v0, $zero, 0x1
endlabel func_80064FB4
