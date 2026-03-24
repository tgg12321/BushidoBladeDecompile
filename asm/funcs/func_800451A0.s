glabel func_800451A0
    /* 359A0 800451A0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 359A4 800451A4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 359A8 800451A8 01000424 */  addiu      $a0, $zero, 0x1
    /* 359AC 800451AC 0980053C */  lui        $a1, %hi(D_800963EC)
    /* 359B0 800451B0 EC63A524 */  addiu      $a1, $a1, %lo(D_800963EC)
    /* 359B4 800451B4 21300000 */  addu       $a2, $zero, $zero
    /* 359B8 800451B8 8DDB000C */  jal        func_80036E34
    /* 359BC 800451BC 02000724 */   addiu     $a3, $zero, 0x2
    /* 359C0 800451C0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 359C4 800451C4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 359C8 800451C8 0800E003 */  jr         $ra
    /* 359CC 800451CC 00000000 */   nop
endlabel func_800451A0
