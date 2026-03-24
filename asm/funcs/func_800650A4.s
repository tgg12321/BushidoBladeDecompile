glabel func_800650A4
    /* 558A4 800650A4 B003838F */  lw         $v1, %gp_rel(D_800A347C)($gp)
    /* 558A8 800650A8 00000000 */  nop
    /* 558AC 800650AC 0000628C */  lw         $v0, 0x0($v1)
    /* 558B0 800650B0 0F80013C */  lui        $at, %hi(D_800F0CF4)
    /* 558B4 800650B4 F40C22AC */  sw         $v0, %lo(D_800F0CF4)($at)
    /* 558B8 800650B8 0400628C */  lw         $v0, 0x4($v1)
    /* 558BC 800650BC 0F80013C */  lui        $at, %hi(D_800F0CF8)
    /* 558C0 800650C0 F80C22AC */  sw         $v0, %lo(D_800F0CF8)($at)
    /* 558C4 800650C4 0800638C */  lw         $v1, 0x8($v1)
    /* 558C8 800650C8 01000224 */  addiu      $v0, $zero, 0x1
    /* 558CC 800650CC 0F80013C */  lui        $at, %hi(D_800F1104)
    /* 558D0 800650D0 041122AC */  sw         $v0, %lo(D_800F1104)($at)
    /* 558D4 800650D4 0F80013C */  lui        $at, %hi(D_800F0BB6)
    /* 558D8 800650D8 B60B20A4 */  sh         $zero, %lo(D_800F0BB6)($at)
    /* 558DC 800650DC 0F80013C */  lui        $at, %hi(D_800F0CFC)
    /* 558E0 800650E0 FC0C23AC */  sw         $v1, %lo(D_800F0CFC)($at)
    /* 558E4 800650E4 0800E003 */  jr         $ra
    /* 558E8 800650E8 00000000 */   nop
endlabel func_800650A4
