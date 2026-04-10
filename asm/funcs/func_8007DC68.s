glabel func_8007DC68
    /* 6E468 8007DC68 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6E46C 8007DC6C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6E470 8007DC70 330A020C */  jal        sys_VSync
    /* 6E474 8007DC74 FFFF0424 */   addiu     $a0, $zero, -0x1
    /* 6E478 8007DC78 F0004224 */  addiu      $v0, $v0, 0xF0
    /* 6E47C 8007DC7C 0A80013C */  lui        $at, %hi(D_8009BF8C)
    /* 6E480 8007DC80 8CBF22AC */  sw         $v0, %lo(D_8009BF8C)($at)
    /* 6E484 8007DC84 0A80013C */  lui        $at, %hi(D_8009BF90)
    /* 6E488 8007DC88 90BF20AC */  sw         $zero, %lo(D_8009BF90)($at)
    /* 6E48C 8007DC8C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6E490 8007DC90 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6E494 8007DC94 0800E003 */  jr         $ra
    /* 6E498 8007DC98 00000000 */   nop
endlabel func_8007DC68
