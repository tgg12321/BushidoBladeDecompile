glabel func_8003AA78
    /* 2B278 8003AA78 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2B27C 8003AA7C 01000224 */  addiu      $v0, $zero, 0x1
    /* 2B280 8003AA80 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2B284 8003AA84 A40782AF */  sw         $v0, %gp_rel(D_800A3870)($gp)
    /* 2B288 8003AA88 330A020C */  jal        sys_VSync
    /* 2B28C 8003AA8C 02000424 */   addiu     $a0, $zero, 0x2
    /* 2B290 8003AA90 92EA000C */  jal        func_8003AA48
    /* 2B294 8003AA94 00000000 */   nop
    /* 2B298 8003AA98 330A020C */  jal        sys_VSync
    /* 2B29C 8003AA9C 02000424 */   addiu     $a0, $zero, 0x2
    /* 2B2A0 8003AAA0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2B2A4 8003AAA4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2B2A8 8003AAA8 0800E003 */  jr         $ra
    /* 2B2AC 8003AAAC 00000000 */   nop
endlabel func_8003AA78
