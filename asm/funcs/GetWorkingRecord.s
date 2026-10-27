glabel GetWorkingRecord
    /* 68500 80077D00 0A80023C */  lui        $v0, %hi(g_working_record)
    /* 68504 80077D04 24BD4224 */  addiu      $v0, $v0, %lo(g_working_record)
    /* 68508 80077D08 0800E003 */  jr         $ra
    /* 6850C 80077D0C 00000000 */   nop
endlabel GetWorkingRecord
