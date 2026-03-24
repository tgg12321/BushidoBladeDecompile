glabel func_80035FA8
    /* 267A8 80035FA8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 267AC 80035FAC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 267B0 80035FB0 21200000 */  addu       $a0, $zero, $zero
    /* 267B4 80035FB4 21280000 */  addu       $a1, $zero, $zero
    /* 267B8 80035FB8 D40E020C */  jal        func_80083B50
    /* 267BC 80035FBC 01000624 */   addiu     $a2, $zero, 0x1
    /* 267C0 80035FC0 21200000 */  addu       $a0, $zero, $zero
    /* 267C4 80035FC4 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 267C8 80035FC8 1215020C */  jal        func_80085448
    /* 267CC 80035FCC 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 267D0 80035FD0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 267D4 80035FD4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 267D8 80035FD8 0800E003 */  jr         $ra
    /* 267DC 80035FDC 00000000 */   nop
endlabel func_80035FA8
