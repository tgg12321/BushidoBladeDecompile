glabel func_800474D0
    /* 37CD0 800474D0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 37CD4 800474D4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 37CD8 800474D8 FF23010C */  jal        func_80048FFC
    /* 37CDC 800474DC 21200000 */   addu      $a0, $zero, $zero
    /* 37CE0 800474E0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 37CE4 800474E4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 37CE8 800474E8 0800E003 */  jr         $ra
    /* 37CEC 800474EC 00000000 */   nop
endlabel func_800474D0
