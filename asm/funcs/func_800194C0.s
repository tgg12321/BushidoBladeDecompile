glabel func_800194C0
    /* 9CC0 800194C0 0F008230 */  andi       $v0, $a0, 0xF
    /* 9CC4 800194C4 0A80013C */  lui        $at, %hi(D_800A3912)
    /* 9CC8 800194C8 123922A0 */  sb         $v0, %lo(D_800A3912)($at)
    /* 9CCC 800194CC 03110400 */  sra        $v0, $a0, 4
    /* 9CD0 800194D0 0F004230 */  andi       $v0, $v0, 0xF
    /* 9CD4 800194D4 03220400 */  sra        $a0, $a0, 8
    /* 9CD8 800194D8 0F008430 */  andi       $a0, $a0, 0xF
    /* 9CDC 800194DC 0A80013C */  lui        $at, %hi(D_800A3913)
    /* 9CE0 800194E0 133922A0 */  sb         $v0, %lo(D_800A3913)($at)
    /* 9CE4 800194E4 0A80013C */  lui        $at, %hi(D_800A3914)
    /* 9CE8 800194E8 143924A0 */  sb         $a0, %lo(D_800A3914)($at)
    /* 9CEC 800194EC 0800E003 */  jr         $ra
    /* 9CF0 800194F0 00000000 */   nop
endlabel func_800194C0
