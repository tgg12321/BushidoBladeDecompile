glabel SetInitPadFlag
    /* 693E0 80078BE0 0A80013C */  lui        $at, %hi(is_pad_init)
    /* 693E4 80078BE4 80BD24AC */  sw         $a0, %lo(is_pad_init)($at)
    /* 693E8 80078BE8 0800E003 */  jr         $ra
    /* 693EC 80078BEC 00000000 */   nop
endlabel SetInitPadFlag
