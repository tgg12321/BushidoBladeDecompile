glabel gpu_GetLastFogOtIndex
    /* 2EAC8 8003E2C8 0980023C */  lui        $v0, %hi(g_gpu_last_fog_ot_index)
    /* 2EACC 8003E2CC F805428C */  lw         $v0, %lo(g_gpu_last_fog_ot_index)($v0)
    /* 2EAD0 8003E2D0 0800E003 */  jr         $ra
    /* 2EAD4 8003E2D4 00000000 */   nop
endlabel gpu_GetLastFogOtIndex
