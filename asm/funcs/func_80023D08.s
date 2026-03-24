glabel func_80023D08
    /* 14508 80023D08 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 1450C 80023D0C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 14510 80023D10 2D8F000C */  jal        func_80023CB4
    /* 14514 80023D14 00020524 */   addiu     $a1, $zero, 0x200
    /* 14518 80023D18 1000BF8F */  lw         $ra, 0x10($sp)
    /* 1451C 80023D1C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 14520 80023D20 0800E003 */  jr         $ra
    /* 14524 80023D24 00000000 */   nop
endlabel func_80023D08
