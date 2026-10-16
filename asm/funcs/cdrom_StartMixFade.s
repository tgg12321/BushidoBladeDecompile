glabel cdrom_StartMixFade
    /* 26778 80035F78 1000A28F */  lw         $v0, 0x10($sp)
    /* 2677C 80035F7C EC0585A3 */  sb         $a1, %gp_rel(g_cd_atv_target)($gp)
    /* 26780 80035F80 0A80013C */  lui        $at, %hi(g_cd_atv_target + 0x1)
    /* 26784 80035F84 B93626A0 */  sb         $a2, %lo(g_cd_atv_target + 0x1)($at)
    /* 26788 80035F88 0A80013C */  lui        $at, %hi(g_cd_atv_target + 0x2)
    /* 2678C 80035F8C BA3627A0 */  sb         $a3, %lo(g_cd_atv_target + 0x2)($at)
    /* 26790 80035F90 880784A7 */  sh         $a0, %gp_rel(g_cd_fade_len)($gp)
    /* 26794 80035F94 740780A7 */  sh         $zero, %gp_rel(g_cd_fade_step)($gp)
    /* 26798 80035F98 0A80013C */  lui        $at, %hi(g_cd_atv_target + 0x3)
    /* 2679C 80035F9C BB3622A0 */  sb         $v0, %lo(g_cd_atv_target + 0x3)($at)
    /* 267A0 80035FA0 0800E003 */  jr         $ra
    /* 267A4 80035FA4 00000000 */   nop
endlabel cdrom_StartMixFade
