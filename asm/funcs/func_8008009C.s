glabel CdStatus
    /* 7089C 8008009C 0A80023C */  lui        $v0, %hi(CD_status)
    /* 708A0 800800A0 C4114290 */  lbu        $v0, %lo(CD_status)($v0)
    /* 708A4 800800A4 0800E003 */  jr         $ra
    /* 708A8 800800A8 00000000 */   nop
endlabel CdStatus
