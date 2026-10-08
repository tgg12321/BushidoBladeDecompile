glabel func_8003F7F4
    /* 2FFF4 8003F7F4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2FFF8 8003F7F8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2FFFC 8003F7FC C55D000C */  jal        func_80017714
    /* 30000 8003F800 00000000 */   nop
    /* 30004 8003F804 E45F000C */  jal        func_80017F90
    /* 30008 8003F808 00000000 */   nop
    /* 3000C 8003F80C A40280AF */  sw         $zero, %gp_rel(D_800A3370)($gp)
    /* 30010 8003F810 A80280AF */  sw         $zero, %gp_rel(D_800A3374)($gp)
    /* 30014 8003F814 1000BF8F */  lw         $ra, 0x10($sp)
    /* 30018 8003F818 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 3001C 8003F81C 0800E003 */  jr         $ra
    /* 30020 8003F820 00000000 */   nop
endlabel func_8003F7F4
