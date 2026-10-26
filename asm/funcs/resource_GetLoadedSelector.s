glabel resource_GetLoadedSelector
    /* 36F98 80046798 0A80023C */  lui        $v0, %hi(g_loaded_resource_selector)
    /* 36F9C 8004679C 78944284 */  lh         $v0, %lo(g_loaded_resource_selector)($v0)
    /* 36FA0 800467A0 0800E003 */  jr         $ra
    /* 36FA4 800467A4 00000000 */   nop
endlabel resource_GetLoadedSelector
