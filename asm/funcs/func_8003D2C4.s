glabel func_8003D2C4
    /* 2DAC4 8003D2C4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2DAC8 8003D2C8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2DACC 8003D2CC 0A80043C */  lui        $a0, %hi(D_800A3220)
    /* 2DAD0 8003D2D0 20328424 */  addiu      $a0, $a0, %lo(D_800A3220)
    /* 2DAD4 8003D2D4 0980053C */  lui        $a1, %hi(D_80090178)
    /* 2DAD8 8003D2D8 7801A524 */  addiu      $a1, $a1, %lo(D_80090178)
    /* 2DADC 8003D2DC 80ED010C */  jal        gpu_LoadImage
    /* 2DAE0 8003D2E0 00000000 */   nop
    /* 2DAE4 8003D2E4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2DAE8 8003D2E8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2DAEC 8003D2EC 0800E003 */  jr         $ra
    /* 2DAF0 8003D2F0 00000000 */   nop
endlabel func_8003D2C4
