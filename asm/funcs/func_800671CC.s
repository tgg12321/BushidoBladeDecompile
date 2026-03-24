glabel func_800671CC
    /* 579CC 800671CC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 579D0 800671D0 07000424 */  addiu      $a0, $zero, 0x7
    /* 579D4 800671D4 02000524 */  addiu      $a1, $zero, 0x2
    /* 579D8 800671D8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 579DC 800671DC 809C010C */  jal        func_80067200
    /* 579E0 800671E0 01000624 */   addiu     $a2, $zero, 0x1
    /* 579E4 800671E4 02000324 */  addiu      $v1, $zero, 0x2
    /* 579E8 800671E8 0F80013C */  lui        $at, %hi(D_800F1134)
    /* 579EC 800671EC 341123AC */  sw         $v1, %lo(D_800F1134)($at)
    /* 579F0 800671F0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 579F4 800671F4 FF004230 */  andi       $v0, $v0, 0xFF
    /* 579F8 800671F8 0800E003 */  jr         $ra
    /* 579FC 800671FC 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_800671CC
