glabel gpu_GetSavedGrayscaleFlag
    /* 6FAC 800167AC 1080023C */  lui        $v0, %hi(g_saved_record + 0x23)
    /* 6FB0 800167B0 736A4290 */  lbu        $v0, %lo(g_saved_record + 0x23)($v0)
    /* 6FB4 800167B4 0800E003 */  jr         $ra
    /* 6FB8 800167B8 01004230 */   andi      $v0, $v0, 0x1
endlabel gpu_GetSavedGrayscaleFlag
