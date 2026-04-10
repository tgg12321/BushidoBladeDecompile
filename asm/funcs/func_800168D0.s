glabel func_800168D0
    /* 70D0 800168D0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70D4 800168D4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 70D8 800168D8 A8EC010C */  jal        gpu_SetDispMask
    /* 70DC 800168DC 01000424 */   addiu     $a0, $zero, 0x1
    /* 70E0 800168E0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70E4 800168E4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 70E8 800168E8 0800E003 */  jr         $ra
    /* 70EC 800168EC 00000000 */   nop
endlabel func_800168D0
