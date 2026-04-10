glabel game_Cleanup
    /* 51978 80061178 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 5197C 8006117C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 51980 80061180 1883010C */  jal        func_80060C60
    /* 51984 80061184 00000000 */   nop
    /* 51988 80061188 6908010C */  jal        func_800421A4
    /* 5198C 8006118C 00000000 */   nop
    /* 51990 80061190 F00180AF */  sw         $zero, %gp_rel(D_800A32BC)($gp)
    /* 51994 80061194 1000BF8F */  lw         $ra, 0x10($sp)
    /* 51998 80061198 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 5199C 8006119C 0800E003 */  jr         $ra
    /* 519A0 800611A0 00000000 */   nop
endlabel game_Cleanup
