glabel func_80064ED8
    /* 556D8 80064ED8 B003838F */  lw         $v1, %gp_rel(D_800A347C)($gp)
    /* 556DC 80064EDC 00000000 */  nop
    /* 556E0 80064EE0 0000628C */  lw         $v0, 0x0($v1)
    /* 556E4 80064EE4 0F80013C */  lui        $at, %hi(D_800F0CAC)
    /* 556E8 80064EE8 AC0C22AC */  sw         $v0, %lo(D_800F0CAC)($at)
    /* 556EC 80064EEC 0400628C */  lw         $v0, 0x4($v1)
    /* 556F0 80064EF0 0F80013C */  lui        $at, %hi(D_800F0CB0)
    /* 556F4 80064EF4 B00C22AC */  sw         $v0, %lo(D_800F0CB0)($at)
    /* 556F8 80064EF8 0800638C */  lw         $v1, 0x8($v1)
    /* 556FC 80064EFC 01000224 */  addiu      $v0, $zero, 0x1
    /* 55700 80064F00 0F80013C */  lui        $at, %hi(D_800F10E4)
    /* 55704 80064F04 E41022AC */  sw         $v0, %lo(D_800F10E4)($at)
    /* 55708 80064F08 0F80013C */  lui        $at, %hi(D_800F0BAA)
    /* 5570C 80064F0C AA0B20A4 */  sh         $zero, %lo(D_800F0BAA)($at)
    /* 55710 80064F10 0F80013C */  lui        $at, %hi(D_800F0CB4)
    /* 55714 80064F14 B40C23AC */  sw         $v1, %lo(D_800F0CB4)($at)
    /* 55718 80064F18 0800E003 */  jr         $ra
    /* 5571C 80064F1C 00000000 */   nop
endlabel func_80064ED8
