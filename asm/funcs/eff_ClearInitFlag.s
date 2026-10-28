glabel eff_ClearInitFlag
    /* 7474 80016C74 4A0680A3 */  sb         $zero, %gp_rel(g_eff_initialized)($gp)
    /* 7478 80016C78 0800E003 */  jr         $ra
    /* 747C 80016C7C 00000000 */   nop
endlabel eff_ClearInitFlag
