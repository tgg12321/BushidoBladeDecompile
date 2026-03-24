glabel func_80066FF8
    /* 577F8 80066FF8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 577FC 80066FFC 03000424 */  addiu      $a0, $zero, 0x3
    /* 57800 80067000 21280000 */  addu       $a1, $zero, $zero
    /* 57804 80067004 1000BFAF */  sw         $ra, 0x10($sp)
    /* 57808 80067008 809C010C */  jal        func_80067200
    /* 5780C 8006700C 21300000 */   addu      $a2, $zero, $zero
    /* 57810 80067010 01000324 */  addiu      $v1, $zero, 0x1
    /* 57814 80067014 0F80013C */  lui        $at, %hi(D_800F1124)
    /* 57818 80067018 241123AC */  sw         $v1, %lo(D_800F1124)($at)
    /* 5781C 8006701C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 57820 80067020 FF004230 */  andi       $v0, $v0, 0xFF
    /* 57824 80067024 0800E003 */  jr         $ra
    /* 57828 80067028 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80066FF8
