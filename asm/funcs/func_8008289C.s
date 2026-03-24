glabel func_8008289C
    /* 7309C 8008289C 0A80023C */  lui        $v0, %hi(D_800A14CC)
    /* 730A0 800828A0 CC14428C */  lw         $v0, %lo(D_800A14CC)($v0)
    /* 730A4 800828A4 0A80013C */  lui        $at, %hi(D_800A14CC)
    /* 730A8 800828A8 CC1424AC */  sw         $a0, %lo(D_800A14CC)($at)
    /* 730AC 800828AC 0800E003 */  jr         $ra
    /* 730B0 800828B0 00000000 */   nop
endlabel func_8008289C
