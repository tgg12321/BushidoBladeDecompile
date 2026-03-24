glabel func_800670FC
    /* 578FC 800670FC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 57900 80067100 05000424 */  addiu      $a0, $zero, 0x5
    /* 57904 80067104 03000524 */  addiu      $a1, $zero, 0x3
    /* 57908 80067108 1000BFAF */  sw         $ra, 0x10($sp)
    /* 5790C 8006710C 809C010C */  jal        func_80067200
    /* 57910 80067110 01000624 */   addiu     $a2, $zero, 0x1
    /* 57914 80067114 02000324 */  addiu      $v1, $zero, 0x2
    /* 57918 80067118 0F80013C */  lui        $at, %hi(D_800F112C)
    /* 5791C 8006711C 2C1123AC */  sw         $v1, %lo(D_800F112C)($at)
    /* 57920 80067120 1000BF8F */  lw         $ra, 0x10($sp)
    /* 57924 80067124 FF004230 */  andi       $v0, $v0, 0xFF
    /* 57928 80067128 0800E003 */  jr         $ra
    /* 5792C 8006712C 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_800670FC
