glabel arena_GetBase
    /* 36014 80045814 0B80023C */  lui        $v0, %hi(g_arena)
    /* 36018 80045818 109D4224 */  addiu      $v0, $v0, %lo(g_arena)
    /* 3601C 8004581C 0800E003 */  jr         $ra
    /* 36020 80045820 00000000 */   nop
endlabel arena_GetBase
