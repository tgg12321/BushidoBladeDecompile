glabel memcard_RequestSave
    /* 28F9C 8003879C 01000224 */  addiu      $v0, $zero, 0x1
    /* 28FA0 800387A0 D20682A7 */  sh         $v0, %gp_rel(g_memcard_request_status)($gp)
    /* 28FA4 800387A4 FC0682A7 */  sh         $v0, %gp_rel(g_memcard_request_operation)($gp)
    /* 28FA8 800387A8 01000224 */  addiu      $v0, $zero, 0x1
    /* 28FAC 800387AC 000880A3 */  sb         $zero, %gp_rel(g_memcard_request_write_started)($gp)
    /* 28FB0 800387B0 480780A7 */  sh         $zero, %gp_rel(g_memcard_request_retry_count)($gp)
    /* 28FB4 800387B4 280182AF */  sw         $v0, %gp_rel(g_memcard_request_state)($gp)
    /* 28FB8 800387B8 0800E003 */  jr         $ra
    /* 28FBC 800387BC 00000000 */   nop
endlabel memcard_RequestSave
