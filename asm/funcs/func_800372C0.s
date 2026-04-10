glabel func_800372C0
    /* 27AC0 800372C0 1080023C */  lui        $v0, %hi(D_80101E62)
    /* 27AC4 800372C4 621E4284 */  lh         $v0, %lo(D_80101E62)($v0)
    /* 27AC8 800372C8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 27ACC 800372CC 03004010 */  beqz       $v0, .L800372DC
    /* 27AD0 800372D0 1000BFAF */   sw        $ra, 0x10($sp)
    /* 27AD4 800372D4 B0DB000C */  jal        game_FrameInit
    /* 27AD8 800372D8 00000000 */   nop
  .L800372DC:
    /* 27ADC 800372DC D0DB000C */  jal        game_FrameLoop
    /* 27AE0 800372E0 00000000 */   nop
    /* 27AE4 800372E4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 27AE8 800372E8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 27AEC 800372EC 0800E003 */  jr         $ra
    /* 27AF0 800372F0 00000000 */   nop
endlabel func_800372C0
