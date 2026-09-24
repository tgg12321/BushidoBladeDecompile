glabel CdReadCallback
    /* 7309C 8008289C 0A80023C */  lui        $v0, %hi(g_CdReadCallback_func)
    /* 730A0 800828A0 CC14428C */  lw         $v0, %lo(g_CdReadCallback_func)($v0)
    /* 730A4 800828A4 0A80013C */  lui        $at, %hi(g_CdReadCallback_func)
    /* 730A8 800828A8 CC1424AC */  sw         $a0, %lo(g_CdReadCallback_func)($at)
    /* 730AC 800828AC 0800E003 */  jr         $ra
    /* 730B0 800828B0 00000000 */   nop
endlabel CdReadCallback
