glabel func_800474AC
    /* 37CAC 800474AC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 37CB0 800474B0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 37CB4 800474B4 21200000 */  addu       $a0, $zero, $zero
    /* 37CB8 800474B8 D623010C */  jal        func_80048F58
    /* 37CBC 800474BC 21280000 */   addu      $a1, $zero, $zero
    /* 37CC0 800474C0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 37CC4 800474C4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 37CC8 800474C8 0800E003 */  jr         $ra
    /* 37CCC 800474CC 00000000 */   nop
endlabel func_800474AC
