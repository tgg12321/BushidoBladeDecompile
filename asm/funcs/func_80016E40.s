glabel func_80016E40
    /* 7640 80016E40 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7644 80016E44 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7648 80016E48 CFEC010C */  jal        func_8007B33C
    /* 764C 80016E4C 21200000 */   addu      $a0, $zero, $zero
    /* 7650 80016E50 1000BF8F */  lw         $ra, 0x10($sp)
    /* 7654 80016E54 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7658 80016E58 0800E003 */  jr         $ra
    /* 765C 80016E5C 00000000 */   nop
endlabel func_80016E40
