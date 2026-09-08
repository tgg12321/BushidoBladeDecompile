glabel CdLastPos
    /* 708CC 800800CC 0A80023C */  lui        $v0, %hi(CD_pos)
    /* 708D0 800800D0 D0114224 */  addiu      $v0, $v0, %lo(CD_pos)
    /* 708D4 800800D4 0800E003 */  jr         $ra
    /* 708D8 800800D8 00000000 */   nop
endlabel CdLastPos
