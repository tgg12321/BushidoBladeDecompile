glabel func_80037F08
    /* 28708 80037F08 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2870C 80037F0C 1800BFAF */  sw         $ra, 0x18($sp)
    /* 28710 80037F10 21308000 */  addu       $a2, $a0, $zero
    /* 28714 80037F14 2138A000 */  addu       $a3, $a1, $zero
    /* 28718 80037F18 0180053C */  lui        $a1, %hi(D_800109C8)
    /* 2871C 80037F1C C809A524 */  addiu      $a1, $a1, %lo(D_800109C8)
    /* 28720 80037F20 8CE6010C */  jal        func_80079A30
    /* 28724 80037F24 1000A427 */   addiu     $a0, $sp, 0x10
    /* 28728 80037F28 8AE2010C */  jal        func_80078A28
    /* 2872C 80037F2C 1000A427 */   addiu     $a0, $sp, 0x10
    /* 28730 80037F30 1800BF8F */  lw         $ra, 0x18($sp)
    /* 28734 80037F34 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 28738 80037F38 0800E003 */  jr         $ra
    /* 2873C 80037F3C 00000000 */   nop
endlabel func_80037F08
