glabel func_80066EC0
    /* 576C0 80066EC0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 576C4 80066EC4 21200000 */  addu       $a0, $zero, $zero
    /* 576C8 80066EC8 21280000 */  addu       $a1, $zero, $zero
    /* 576CC 80066ECC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 576D0 80066ED0 809C010C */  jal        func_80067200
    /* 576D4 80066ED4 21300000 */   addu      $a2, $zero, $zero
    /* 576D8 80066ED8 01000324 */  addiu      $v1, $zero, 0x1
    /* 576DC 80066EDC 0F80013C */  lui        $at, %hi(D_800F10D8)
    /* 576E0 80066EE0 D81023AC */  sw         $v1, %lo(D_800F10D8)($at)
    /* 576E4 80066EE4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 576E8 80066EE8 FF004230 */  andi       $v0, $v0, 0xFF
    /* 576EC 80066EEC 0800E003 */  jr         $ra
    /* 576F0 80066EF0 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80066EC0
