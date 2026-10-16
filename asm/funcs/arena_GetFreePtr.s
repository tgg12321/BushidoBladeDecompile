glabel arena_GetFreePtr
    /* 35FFC 800457FC D402828F */  lw         $v0, %gp_rel(g_arena_free_ptr)($gp)
    /* 36000 80045800 0800E003 */  jr         $ra
    /* 36004 80045804 00000000 */   nop
endlabel arena_GetFreePtr
