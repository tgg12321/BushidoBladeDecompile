glabel func_80046EDC
    /* 376DC 80046EDC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 376E0 80046EE0 1000B0AF */  sw         $s0, 0x10($sp)
    /* 376E4 80046EE4 1400BFAF */  sw         $ra, 0x14($sp)
    /* 376E8 80046EE8 3918010C */  jal        func_800460E4
    /* 376EC 80046EEC 21808000 */   addu      $s0, $a0, $zero
    /* 376F0 80046EF0 7208010C */  jal        func_800421C8
    /* 376F4 80046EF4 21200002 */   addu      $a0, $s0, $zero
    /* 376F8 80046EF8 38F8000C */  jal        func_8003E0E0
    /* 376FC 80046EFC 00000000 */   nop
    /* 37700 80046F00 1400BF8F */  lw         $ra, 0x14($sp)
    /* 37704 80046F04 1000B08F */  lw         $s0, 0x10($sp)
    /* 37708 80046F08 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 3770C 80046F0C 0800E003 */  jr         $ra
    /* 37710 80046F10 00000000 */   nop
endlabel func_80046EDC
