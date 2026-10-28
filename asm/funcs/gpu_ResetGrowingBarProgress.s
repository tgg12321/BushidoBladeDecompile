glabel gpu_ResetGrowingBarProgress
    /* 50F58 80060758 EA0180A7 */  sh         $zero, %gp_rel(g_gpu_fixed_bar_progress)($gp)
    /* 50F5C 8006075C E80180A7 */  sh         $zero, %gp_rel(g_gpu_highlighted_row_bar_progress)($gp)
    /* 50F60 80060760 0800E003 */  jr         $ra
    /* 50F64 80060764 00000000 */   nop
endlabel gpu_ResetGrowingBarProgress
