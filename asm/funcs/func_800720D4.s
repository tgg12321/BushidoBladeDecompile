glabel func_800720D4
    /* 628D4 800720D4 DC04828F */  lw         $v0, %gp_rel(D_800A35A8)($gp)
    /* 628D8 800720D8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 628DC 800720DC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 628E0 800720E0 7000458C */  lw         $a1, 0x70($v0)
    /* 628E4 800720E4 3FC8010C */  jal        func_800720FC
    /* 628E8 800720E8 02000624 */   addiu     $a2, $zero, 0x2
    /* 628EC 800720EC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 628F0 800720F0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 628F4 800720F4 0800E003 */  jr         $ra
    /* 628F8 800720F8 00000000 */   nop
endlabel func_800720D4
