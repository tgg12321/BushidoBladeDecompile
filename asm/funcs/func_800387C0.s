glabel func_800387C0
    /* 28FC0 800387C0 01000224 */  addiu      $v0, $zero, 0x1
    /* 28FC4 800387C4 D20682A7 */  sh         $v0, %gp_rel(D_800A379E)($gp)
    /* 28FC8 800387C8 02000224 */  addiu      $v0, $zero, 0x2
    /* 28FCC 800387CC FC0682A7 */  sh         $v0, %gp_rel(D_800A37C8)($gp)
    /* 28FD0 800387D0 01000224 */  addiu      $v0, $zero, 0x1
    /* 28FD4 800387D4 000880A3 */  sb         $zero, %gp_rel(D_800A38CC)($gp)
    /* 28FD8 800387D8 480780A7 */  sh         $zero, %gp_rel(D_800A3814)($gp)
    /* 28FDC 800387DC 280182AF */  sw         $v0, %gp_rel(D_800A31F4)($gp)
    /* 28FE0 800387E0 0800E003 */  jr         $ra
    /* 28FE4 800387E4 00000000 */   nop
endlabel func_800387C0
