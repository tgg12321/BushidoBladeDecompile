glabel font_BeginFrame
    /* 2DAF4 8003D2F4 F000023C */  lui        $v0, (0xF0F0F0 >> 16)
    /* 2DAF8 8003D2F8 F0F04234 */  ori        $v0, $v0, (0xF0F0F0 & 0xFFFF)
    /* 2DAFC 8003D2FC 980282AF */  sw         $v0, %gp_rel(g_font_color)($gp)
    /* 2DB00 8003D300 4C01828F */  lw         $v0, %gp_rel(g_font_buf_index)($gp)
    /* 2DB04 8003D304 5001838F */  lw         $v1, %gp_rel(g_font_enabled)($gp)
    /* 2DB08 8003D308 8C0280AF */  sw         $zero, %gp_rel(g_font_sprite_count)($gp)
    /* 2DB0C 8003D30C 940280AF */  sw         $zero, %gp_rel(g_font_row)($gp)
    /* 2DB10 8003D310 900280AF */  sw         $zero, %gp_rel(g_font_col)($gp)
    /* 2DB14 8003D314 0100422C */  sltiu      $v0, $v0, 0x1
    /* 2DB18 8003D318 4C0182AF */  sw         $v0, %gp_rel(g_font_buf_index)($gp)
    /* 2DB1C 8003D31C 02006014 */  bnez       $v1, .L8003D328
    /* 2DB20 8003D320 20000224 */   addiu     $v0, $zero, 0x20
    /* 2DB24 8003D324 8C0282AF */  sw         $v0, %gp_rel(g_font_sprite_count)($gp)
  .L8003D328:
    /* 2DB28 8003D328 0800E003 */  jr         $ra
    /* 2DB2C 8003D32C 00000000 */   nop
endlabel font_BeginFrame
