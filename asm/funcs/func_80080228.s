glabel cdrom_SetCallbackA
    /* 70A28 80080228 0A80023C */  lui        $v0, %hi(D_800A11B4)
    /* 70A2C 8008022C B411428C */  lw         $v0, %lo(D_800A11B4)($v0)
    /* 70A30 80080230 0A80013C */  lui        $at, %hi(D_800A11B4)
    /* 70A34 80080234 B41124AC */  sw         $a0, %lo(D_800A11B4)($at)
    /* 70A38 80080238 0800E003 */  jr         $ra
    /* 70A3C 8008023C 00000000 */   nop
endlabel cdrom_SetCallbackA
