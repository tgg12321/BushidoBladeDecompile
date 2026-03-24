glabel func_80077984
    /* 68184 80077984 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 68188 80077988 1405858F */  lw         $a1, %gp_rel(D_800A35E0)($gp)
    /* 6818C 8007798C 1C05878F */  lw         $a3, %gp_rel(D_800A35E8)($gp)
    /* 68190 80077990 0A80063C */  lui        $a2, %hi(D_8009BD24)
    /* 68194 80077994 24BDC624 */  addiu      $a2, $a2, %lo(D_8009BD24)
    /* 68198 80077998 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6819C 8007799C 4DB9010C */  jal        func_8006E534
    /* 681A0 800779A0 00000000 */   nop
    /* 681A4 800779A4 01000424 */  addiu      $a0, $zero, 0x1
    /* 681A8 800779A8 21280000 */  addu       $a1, $zero, $zero
    /* 681AC 800779AC 21300000 */  addu       $a2, $zero, $zero
    /* 681B0 800779B0 DA59000C */  jal        func_80016768
    /* 681B4 800779B4 21380000 */   addu      $a3, $zero, $zero
    /* 681B8 800779B8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 681BC 800779BC 01000224 */  addiu      $v0, $zero, 0x1
    /* 681C0 800779C0 0800E003 */  jr         $ra
    /* 681C4 800779C4 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80077984
