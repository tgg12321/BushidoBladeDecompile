glabel func_8003D2F4
    /* 2DAF4 8003D2F4 F000023C */  lui        $v0, (0xF0F0F0 >> 16)
    /* 2DAF8 8003D2F8 F0F04234 */  ori        $v0, $v0, (0xF0F0F0 & 0xFFFF)
    /* 2DAFC 8003D2FC 980282AF */  sw         $v0, %gp_rel(D_800A3364)($gp)
    /* 2DB00 8003D300 4C01828F */  lw         $v0, %gp_rel(D_800A3218)($gp)
    /* 2DB04 8003D304 5001838F */  lw         $v1, %gp_rel(D_800A321C)($gp)
    /* 2DB08 8003D308 8C0280AF */  sw         $zero, %gp_rel(D_800A3358)($gp)
    /* 2DB0C 8003D30C 940280AF */  sw         $zero, %gp_rel(D_800A3360)($gp)
    /* 2DB10 8003D310 900280AF */  sw         $zero, %gp_rel(D_800A335C)($gp)
    /* 2DB14 8003D314 0100422C */  sltiu      $v0, $v0, 0x1
    /* 2DB18 8003D318 4C0182AF */  sw         $v0, %gp_rel(D_800A3218)($gp)
    /* 2DB1C 8003D31C 02006014 */  bnez       $v1, .L8003D328
    /* 2DB20 8003D320 20000224 */   addiu     $v0, $zero, 0x20
    /* 2DB24 8003D324 8C0282AF */  sw         $v0, %gp_rel(D_800A3358)($gp)
  .L8003D328:
    /* 2DB28 8003D328 0800E003 */  jr         $ra
    /* 2DB2C 8003D32C 00000000 */   nop
endlabel func_8003D2F4
