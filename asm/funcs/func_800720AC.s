glabel func_800720AC
    /* 628AC 800720AC DC04828F */  lw         $v0, %gp_rel(D_800A35A8)($gp)
    /* 628B0 800720B0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 628B4 800720B4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 628B8 800720B8 6C00458C */  lw         $a1, 0x6C($v0)
    /* 628BC 800720BC 3FC8010C */  jal        func_800720FC
    /* 628C0 800720C0 01000624 */   addiu     $a2, $zero, 0x1
    /* 628C4 800720C4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 628C8 800720C8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 628CC 800720CC 0800E003 */  jr         $ra
    /* 628D0 800720D0 00000000 */   nop
endlabel func_800720AC
