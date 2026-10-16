glabel arena_GetFreeSize
    /* 35FDC 800457DC D802828F */  lw         $v0, %gp_rel(g_arena_free_size)($gp)
    /* 35FE0 800457E0 0800E003 */  jr         $ra
    /* 35FE4 800457E4 00000000 */   nop
endlabel arena_GetFreeSize
