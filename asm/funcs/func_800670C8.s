glabel func_800670C8
    /* 578C8 800670C8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 578CC 800670CC 05000424 */  addiu      $a0, $zero, 0x5
    /* 578D0 800670D0 03000524 */  addiu      $a1, $zero, 0x3
    /* 578D4 800670D4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 578D8 800670D8 809C010C */  jal        func_80067200
    /* 578DC 800670DC 21300000 */   addu      $a2, $zero, $zero
    /* 578E0 800670E0 01000324 */  addiu      $v1, $zero, 0x1
    /* 578E4 800670E4 0F80013C */  lui        $at, %hi(D_800F112C)
    /* 578E8 800670E8 2C1123AC */  sw         $v1, %lo(D_800F112C)($at)
    /* 578EC 800670EC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 578F0 800670F0 FF004230 */  andi       $v0, $v0, 0xFF
    /* 578F4 800670F4 0800E003 */  jr         $ra
    /* 578F8 800670F8 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_800670C8
