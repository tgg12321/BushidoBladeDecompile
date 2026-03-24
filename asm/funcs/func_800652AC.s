glabel func_800652AC
    /* 55AAC 800652AC B003838F */  lw         $v1, %gp_rel(D_800A347C)($gp)
    /* 55AB0 800652B0 00000000 */  nop
    /* 55AB4 800652B4 0000628C */  lw         $v0, 0x0($v1)
    /* 55AB8 800652B8 0F80013C */  lui        $at, %hi(D_800F0D6C)
    /* 55ABC 800652BC 6C0D22AC */  sw         $v0, %lo(D_800F0D6C)($at)
    /* 55AC0 800652C0 0400628C */  lw         $v0, 0x4($v1)
    /* 55AC4 800652C4 0F80013C */  lui        $at, %hi(D_800F0D70)
    /* 55AC8 800652C8 700D22AC */  sw         $v0, %lo(D_800F0D70)($at)
    /* 55ACC 800652CC 0800638C */  lw         $v1, 0x8($v1)
    /* 55AD0 800652D0 01000224 */  addiu      $v0, $zero, 0x1
    /* 55AD4 800652D4 0F80013C */  lui        $at, %hi(D_800F111C)
    /* 55AD8 800652D8 1C1122AC */  sw         $v0, %lo(D_800F111C)($at)
    /* 55ADC 800652DC 0F80013C */  lui        $at, %hi(D_800F0BCA)
    /* 55AE0 800652E0 CA0B20A4 */  sh         $zero, %lo(D_800F0BCA)($at)
    /* 55AE4 800652E4 0F80013C */  lui        $at, %hi(D_800F0D74)
    /* 55AE8 800652E8 740D23AC */  sw         $v1, %lo(D_800F0D74)($at)
    /* 55AEC 800652EC 0800E003 */  jr         $ra
    /* 55AF0 800652F0 00000000 */   nop
endlabel func_800652AC
