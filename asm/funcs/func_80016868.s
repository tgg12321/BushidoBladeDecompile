glabel func_80016868
    /* 7068 80016868 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 706C 8001686C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7070 80016870 9FEB010C */  jal        gpu_SetMode
    /* 7074 80016874 01000424 */   addiu     $a0, $zero, 0x1
    /* 7078 80016878 1000BF8F */  lw         $ra, 0x10($sp)
    /* 707C 8001687C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7080 80016880 0800E003 */  jr         $ra
    /* 7084 80016884 00000000 */   nop
endlabel func_80016868
