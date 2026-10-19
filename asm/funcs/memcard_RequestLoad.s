glabel memcard_RequestLoad
    /* 28F7C 8003877C 04000224 */  addiu      $v0, $zero, 0x4
    /* 28F80 80038780 D20682A7 */  sh         $v0, %gp_rel(g_memcard_request_status)($gp)
    /* 28F84 80038784 01000224 */  addiu      $v0, $zero, 0x1
    /* 28F88 80038788 480780A7 */  sh         $zero, %gp_rel(D_800A3814)($gp)
    /* 28F8C 8003878C FC0680A7 */  sh         $zero, %gp_rel(g_memcard_request_operation)($gp)
    /* 28F90 80038790 280182AF */  sw         $v0, %gp_rel(g_memcard_request_state)($gp)
    /* 28F94 80038794 0800E003 */  jr         $ra
    /* 28F98 80038798 00000000 */   nop
endlabel memcard_RequestLoad
