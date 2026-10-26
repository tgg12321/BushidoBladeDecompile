glabel gpu_ClearRaisedOtCutoffRequest
    /* 37644 80046E44 0F80013C */  lui        $at, %hi(g_gpu_raised_ot_cutoff_requested)
    /* 37648 80046E48 546620A4 */  sh         $zero, %lo(g_gpu_raised_ot_cutoff_requested)($at)
    /* 3764C 80046E4C 0800E003 */  jr         $ra
    /* 37650 80046E50 00000000 */   nop
endlabel gpu_ClearRaisedOtCutoffRequest
