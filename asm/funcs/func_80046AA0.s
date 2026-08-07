glabel func_80046AA0
    /* 372A0 80046AA0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 372A4 80046AA4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 372A8 80046AA8 7105010C */  jal        player_Destroy
    /* 372AC 80046AAC 21200000 */   addu      $a0, $zero, $zero
    /* 372B0 80046AB0 7105010C */  jal        player_Destroy
    /* 372B4 80046AB4 01000424 */   addiu     $a0, $zero, 0x1
    /* 372B8 80046AB8 A319010C */  jal        func_8004668C
    /* 372BC 80046ABC 00000000 */   nop
    /* 372C0 80046AC0 0818010C */  jal        func_80046020
    /* 372C4 80046AC4 00000000 */   nop
    /* 372C8 80046AC8 8727010C */  jal        func_80049E1C
    /* 372CC 80046ACC 00000000 */   nop
    /* 372D0 80046AD0 451A010C */  jal        snd_StopBgm
    /* 372D4 80046AD4 00000000 */   nop
    /* 372D8 80046AD8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 372DC 80046ADC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 372E0 80046AE0 0800E003 */  jr         $ra
    /* 372E4 80046AE4 00000000 */   nop
endlabel func_80046AA0
