glabel cdrom_GetFileIndex
    /* 276A8 80036EA8 40200400 */  sll        $a0, $a0, 1
    /* 276AC 80036EAC 0980013C */  lui        $at, %hi(g_cdrom_file_group_bases)
    /* 276B0 80036EB0 21082400 */  addu       $at, $at, $a0
    /* 276B4 80036EB4 2CF12284 */  lh         $v0, %lo(g_cdrom_file_group_bases)($at)
    /* 276B8 80036EB8 0800E003 */  jr         $ra
    /* 276BC 80036EBC 21104500 */   addu      $v0, $v0, $a1
endlabel cdrom_GetFileIndex
