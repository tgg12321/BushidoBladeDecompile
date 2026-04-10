glabel cdrom_SetCallbackB
    /* 70A40 80080240 0A80023C */  lui        $v0, %hi(D_800A11B8)
    /* 70A44 80080244 B811428C */  lw         $v0, %lo(D_800A11B8)($v0)
    /* 70A48 80080248 0A80013C */  lui        $at, %hi(D_800A11B8)
    /* 70A4C 8008024C B81124AC */  sw         $a0, %lo(D_800A11B8)($at)
    /* 70A50 80080250 0800E003 */  jr         $ra
    /* 70A54 80080254 00000000 */   nop
endlabel cdrom_SetCallbackB
