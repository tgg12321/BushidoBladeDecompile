glabel func_8003CCCC
    /* 2D4CC 8003CCCC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2D4D0 8003CCD0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2D4D4 8003CCD4 225A000C */  jal        func_80016888
    /* 2D4D8 8003CCD8 00000000 */   nop
    /* 2D4DC 8003CCDC 5E84010C */  jal        func_80061178
    /* 2D4E0 8003CCE0 00000000 */   nop
    /* 2D4E4 8003CCE4 21000224 */  addiu      $v0, $zero, 0x21
    /* 2D4E8 8003CCE8 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2D4EC 8003CCEC B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* 2D4F0 8003CCF0 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2D4F4 8003CCF4 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2D4F8 8003CCF8 345A000C */  jal        func_800168D0
    /* 2D4FC 8003CCFC 00000000 */   nop
    /* 2D500 8003CD00 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2D504 8003CD04 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2D508 8003CD08 0800E003 */  jr         $ra
    /* 2D50C 8003CD0C 00000000 */   nop
endlabel func_8003CCCC
