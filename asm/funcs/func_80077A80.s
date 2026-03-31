glabel func_80077A80
    /* 68280 80077A80 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 68284 80077A84 1C05868F */  lw         $a2, %gp_rel(D_800A35E8)($gp)
    /* 68288 80077A88 0A80053C */  lui        $a1, %hi(D_8009BD24)
    /* 6828C 80077A8C 24BDA524 */  addiu      $a1, $a1, %lo(D_8009BD24)
    /* 68290 80077A90 1000BFAF */  sw         $ra, 0x10($sp)
    /* 68294 80077A94 2EDC010C */  jal        func_800770B8
    /* 68298 80077A98 00000000 */   nop
    /* 6829C 80077A9C 01000424 */  addiu      $a0, $zero, 0x1
    /* 682A0 80077AA0 21280000 */  addu       $a1, $zero, $zero
    /* 682A4 80077AA4 21300000 */  addu       $a2, $zero, $zero
    /* 682A8 80077AA8 DA59000C */  jal        disp_SetFramebufferMode
    /* 682AC 80077AAC 21380000 */   addu      $a3, $zero, $zero
    /* 682B0 80077AB0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 682B4 80077AB4 01000224 */  addiu      $v0, $zero, 0x1
    /* 682B8 80077AB8 0800E003 */  jr         $ra
    /* 682BC 80077ABC 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80077A80
