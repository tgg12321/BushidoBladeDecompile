glabel gpu_GetTrackedFighterViewFlag
    /* 2FA68 8003F268 6001828F */  lw         $v0, %gp_rel(g_gpu_tracked_fighter_view_active)($gp)
    /* 2FA6C 8003F26C 0800E003 */  jr         $ra
    /* 2FA70 8003F270 00000000 */   nop
endlabel gpu_GetTrackedFighterViewFlag
