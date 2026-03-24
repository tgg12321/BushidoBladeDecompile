glabel func_8003A360
    /* 2AB60 8003A360 4407848F */  lw         $a0, %gp_rel(D_800A3810)($gp)
    /* 2AB64 8003A364 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2AB68 8003A368 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2AB6C 8003A36C 6AE2010C */  jal        func_800789A8
    /* 2AB70 8003A370 00000000 */   nop
    /* 2AB74 8003A374 6C06848F */  lw         $a0, %gp_rel(D_800A3738)($gp)
    /* 2AB78 8003A378 6AE2010C */  jal        func_800789A8
    /* 2AB7C 8003A37C 00000000 */   nop
    /* 2AB80 8003A380 01000224 */  addiu      $v0, $zero, 0x1
    /* 2AB84 8003A384 400182A3 */  sb         $v0, %gp_rel(D_800A320C)($gp)
    /* 2AB88 8003A388 640680AF */  sw         $zero, %gp_rel(D_800A3730)($gp)
    /* 2AB8C 8003A38C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2AB90 8003A390 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2AB94 8003A394 0800E003 */  jr         $ra
    /* 2AB98 8003A398 00000000 */   nop
endlabel func_8003A360
