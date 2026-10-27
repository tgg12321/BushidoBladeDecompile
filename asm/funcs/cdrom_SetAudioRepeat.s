glabel cdrom_SetAudioRepeat
    /* 279E8 800371E8 1080013C */  lui        $at, %hi(g_cdrom_state + 0x12)
    /* 279EC 800371EC 6A1E24A4 */  sh         $a0, %lo(g_cdrom_state + 0x12)($at)
    /* 279F0 800371F0 0800E003 */  jr         $ra
    /* 279F4 800371F4 00000000 */   nop
endlabel cdrom_SetAudioRepeat
