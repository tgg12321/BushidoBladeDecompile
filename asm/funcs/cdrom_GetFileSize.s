glabel cdrom_GetFileSize
    /* 27728 80036F28 C0200400 */  sll        $a0, $a0, 3
    /* 2772C 80036F2C 0980013C */  lui        $at, %hi(g_cd_file_table_plus_0x4)
    /* 27730 80036F30 21082400 */  addu       $at, $at, $a0
    /* 27734 80036F34 38EC228C */  lw         $v0, %lo(g_cd_file_table_plus_0x4)($at)
    /* 27738 80036F38 0800E003 */  jr         $ra
    /* 2773C 80036F3C 00000000 */   nop
endlabel cdrom_GetFileSize
