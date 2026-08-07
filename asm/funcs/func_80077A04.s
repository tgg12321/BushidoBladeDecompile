glabel func_80077A04
    /* 68204 80077A04 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 68208 80077A08 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6820C 80077A0C 180580AF */  sw         $zero, %gp_rel(D_800A35E4)($gp)
    /* 68210 80077A10 D3B5010C */  jal        func_8006D74C
    /* 68214 80077A14 00000000 */   nop
    /* 68218 80077A18 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6821C 80077A1C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 68220 80077A20 0800E003 */  jr         $ra
    /* 68224 80077A24 00000000 */   nop
endlabel func_80077A04
