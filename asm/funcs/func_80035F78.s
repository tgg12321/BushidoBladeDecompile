glabel func_80035F78
    /* 26778 80035F78 1000A28F */  lw         $v0, 0x10($sp)
    /* 2677C 80035F7C EC0585A3 */  sb         $a1, %gp_rel(D_800A36B8)($gp)
    /* 26780 80035F80 0A80013C */  lui        $at, %hi(D_800A36B9)
    /* 26784 80035F84 B93626A0 */  sb         $a2, %lo(D_800A36B9)($at)
    /* 26788 80035F88 0A80013C */  lui        $at, %hi(D_800A36BA)
    /* 2678C 80035F8C BA3627A0 */  sb         $a3, %lo(D_800A36BA)($at)
    /* 26790 80035F90 880784A7 */  sh         $a0, %gp_rel(D_800A3854)($gp)
    /* 26794 80035F94 740780A7 */  sh         $zero, %gp_rel(D_800A3840)($gp)
    /* 26798 80035F98 0A80013C */  lui        $at, %hi(D_800A36BB)
    /* 2679C 80035F9C BB3622A0 */  sb         $v0, %lo(D_800A36BB)($at)
    /* 267A0 80035FA0 0800E003 */  jr         $ra
    /* 267A4 80035FA4 00000000 */   nop
endlabel func_80035F78
