glabel comb_RequestClose
    /* 2AC1C 8003A41C 01000224 */  addiu      $v0, $zero, 0x1
    /* 2AC20 8003A420 640682AF */  sw         $v0, %gp_rel(g_comb_close_request)($gp)
    /* 2AC24 8003A424 0800E003 */  jr         $ra
    /* 2AC28 8003A428 00000000 */   nop
endlabel comb_RequestClose
