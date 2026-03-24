glabel func_8003A574
    /* 2AD74 8003A574 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2AD78 8003A578 0A80053C */  lui        $a1, %hi(D_800A3688)
    /* 2AD7C 8003A57C 8836A524 */  addiu      $a1, $a1, %lo(D_800A3688)
    /* 2AD80 8003A580 6806848F */  lw         $a0, %gp_rel(D_800A3734)($gp)
    /* 2AD84 8003A584 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2AD88 8003A588 7EE2010C */  jal        func_800789F8
    /* 2AD8C 8003A58C 08000624 */   addiu     $a2, $zero, 0x8
    /* 2AD90 8003A590 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2AD94 8003A594 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2AD98 8003A598 0800E003 */  jr         $ra
    /* 2AD9C 8003A59C 00000000 */   nop
endlabel func_8003A574
