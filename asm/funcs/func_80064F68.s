glabel func_80064F68
    /* 55768 80064F68 B003838F */  lw         $v1, %gp_rel(D_800A347C)($gp)
    /* 5576C 80064F6C 00000000 */  nop
    /* 55770 80064F70 0000628C */  lw         $v0, 0x0($v1)
    /* 55774 80064F74 0F80013C */  lui        $at, %hi(D_800F0CC4)
    /* 55778 80064F78 C40C22AC */  sw         $v0, %lo(D_800F0CC4)($at)
    /* 5577C 80064F7C 0400628C */  lw         $v0, 0x4($v1)
    /* 55780 80064F80 0F80013C */  lui        $at, %hi(D_800F0CC8)
    /* 55784 80064F84 C80C22AC */  sw         $v0, %lo(D_800F0CC8)($at)
    /* 55788 80064F88 0800638C */  lw         $v1, 0x8($v1)
    /* 5578C 80064F8C 01000224 */  addiu      $v0, $zero, 0x1
    /* 55790 80064F90 0F80013C */  lui        $at, %hi(D_800F10F4)
    /* 55794 80064F94 F41022AC */  sw         $v0, %lo(D_800F10F4)($at)
    /* 55798 80064F98 40000224 */  addiu      $v0, $zero, 0x40
    /* 5579C 80064F9C 0F80013C */  lui        $at, %hi(D_800F0BAE)
    /* 557A0 80064FA0 AE0B22A4 */  sh         $v0, %lo(D_800F0BAE)($at)
    /* 557A4 80064FA4 0F80013C */  lui        $at, %hi(D_800F0CCC)
    /* 557A8 80064FA8 CC0C23AC */  sw         $v1, %lo(D_800F0CCC)($at)
    /* 557AC 80064FAC 0800E003 */  jr         $ra
    /* 557B0 80064FB0 01000224 */   addiu     $v0, $zero, 0x1
endlabel func_80064F68
