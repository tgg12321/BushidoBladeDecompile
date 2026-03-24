glabel func_8007A3C8
    /* 6ABC8 8007A3C8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6ABCC 8007A3CC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6ABD0 8007A3D0 6EE2010C */  jal        func_800789B8
    /* 6ABD4 8007A3D4 00000000 */   nop
    /* 6ABD8 8007A3D8 0EE9010C */  jal        func_8007A438
    /* 6ABDC 8007A3DC 00000000 */   nop
    /* 6ABE0 8007A3E0 96E2010C */  jal        func_80078A58
    /* 6ABE4 8007A3E4 21200000 */   addu      $a0, $zero, $zero
    /* 6ABE8 8007A3E8 72E2010C */  jal        func_800789C8
    /* 6ABEC 8007A3EC 00000000 */   nop
    /* 6ABF0 8007A3F0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6ABF4 8007A3F4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6ABF8 8007A3F8 0800E003 */  jr         $ra
    /* 6ABFC 8007A3FC 00000000 */   nop
endlabel func_8007A3C8
