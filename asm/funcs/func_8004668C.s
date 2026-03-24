glabel func_8004668C
    /* 36E8C 8004668C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 36E90 80046690 1000BFAF */  sw         $ra, 0x10($sp)
    /* 36E94 80046694 F814010C */  jal        func_800453E0
    /* 36E98 80046698 07000424 */   addiu     $a0, $zero, 0x7
    /* 36E9C 8004669C FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 36EA0 800466A0 0A80013C */  lui        $at, %hi(D_80099478)
    /* 36EA4 800466A4 789422A4 */  sh         $v0, %lo(D_80099478)($at)
    /* 36EA8 800466A8 0A80013C */  lui        $at, %hi(D_8009947A)
    /* 36EAC 800466AC 7A9420A4 */  sh         $zero, %lo(D_8009947A)($at)
    /* 36EB0 800466B0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 36EB4 800466B4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 36EB8 800466B8 0800E003 */  jr         $ra
    /* 36EBC 800466BC 00000000 */   nop
endlabel func_8004668C
