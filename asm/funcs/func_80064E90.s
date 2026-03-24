glabel func_80064E90
    /* 55690 80064E90 B003838F */  lw         $v1, %gp_rel(D_800A347C)($gp)
    /* 55694 80064E94 00000000 */  nop
    /* 55698 80064E98 0000628C */  lw         $v0, 0x0($v1)
    /* 5569C 80064E9C 0F80013C */  lui        $at, %hi(D_800F0CA0)
    /* 556A0 80064EA0 A00C22AC */  sw         $v0, %lo(D_800F0CA0)($at)
    /* 556A4 80064EA4 0400628C */  lw         $v0, 0x4($v1)
    /* 556A8 80064EA8 0F80013C */  lui        $at, %hi(D_800F0CA4)
    /* 556AC 80064EAC A40C22AC */  sw         $v0, %lo(D_800F0CA4)($at)
    /* 556B0 80064EB0 0800638C */  lw         $v1, 0x8($v1)
    /* 556B4 80064EB4 01000224 */  addiu      $v0, $zero, 0x1
    /* 556B8 80064EB8 0F80013C */  lui        $at, %hi(D_800F10E0)
    /* 556BC 80064EBC E01022AC */  sw         $v0, %lo(D_800F10E0)($at)
    /* 556C0 80064EC0 0F80013C */  lui        $at, %hi(D_800F0BA8)
    /* 556C4 80064EC4 A80B20A4 */  sh         $zero, %lo(D_800F0BA8)($at)
    /* 556C8 80064EC8 0F80013C */  lui        $at, %hi(D_800F0CA8)
    /* 556CC 80064ECC A80C23AC */  sw         $v1, %lo(D_800F0CA8)($at)
    /* 556D0 80064ED0 0800E003 */  jr         $ra
    /* 556D4 80064ED4 00000000 */   nop
endlabel func_80064E90
