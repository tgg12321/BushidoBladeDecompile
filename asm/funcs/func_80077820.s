glabel func_80077820
    /* 68020 80077820 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 68024 80077824 0A80053C */  lui        $a1, %hi(D_8009BD24)
    /* 68028 80077828 24BDA524 */  addiu      $a1, $a1, %lo(D_8009BD24)
    /* 6802C 8007782C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 68030 80077830 DCA3010C */  jal        func_80068F70
    /* 68034 80077834 00000000 */   nop
    /* 68038 80077838 01000424 */  addiu      $a0, $zero, 0x1
    /* 6803C 8007783C 21280000 */  addu       $a1, $zero, $zero
    /* 68040 80077840 21300000 */  addu       $a2, $zero, $zero
    /* 68044 80077844 DA59000C */  jal        disp_SetFramebufferMode
    /* 68048 80077848 21380000 */   addu      $a3, $zero, $zero
    /* 6804C 8007784C 180580AF */  sw         $zero, %gp_rel(D_800A35E4)($gp)
    /* 68050 80077850 1000BF8F */  lw         $ra, 0x10($sp)
    /* 68054 80077854 01000224 */  addiu      $v0, $zero, 0x1
    /* 68058 80077858 0800E003 */  jr         $ra
    /* 6805C 8007785C 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80077820
