glabel CdSyncCallback
    /* 70A28 80080228 0A80023C */  lui        $v0, %hi(CD_cbsync)
    /* 70A2C 8008022C B411428C */  lw         $v0, %lo(CD_cbsync)($v0)
    /* 70A30 80080230 0A80013C */  lui        $at, %hi(CD_cbsync)
    /* 70A34 80080234 B41124AC */  sw         $a0, %lo(CD_cbsync)($at)
    /* 70A38 80080238 0800E003 */  jr         $ra
    /* 70A3C 8008023C 00000000 */   nop
endlabel CdSyncCallback
