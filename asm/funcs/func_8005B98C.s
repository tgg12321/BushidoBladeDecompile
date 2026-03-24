glabel func_8005B98C
    /* 4C18C 8005B98C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 4C190 8005B990 1000B0AF */  sw         $s0, 0x10($sp)
    /* 4C194 8005B994 21808000 */  addu       $s0, $a0, $zero
    /* 4C198 8005B998 1400BFAF */  sw         $ra, 0x14($sp)
    /* 4C19C 8005B99C 3071010C */  jal        func_8005C4C0
    /* 4C1A0 8005B9A0 08000524 */   addiu     $a1, $zero, 0x8
    /* 4C1A4 8005B9A4 21200002 */  addu       $a0, $s0, $zero
    /* 4C1A8 8005B9A8 3071010C */  jal        func_8005C4C0
    /* 4C1AC 8005B9AC 04000524 */   addiu     $a1, $zero, 0x4
    /* 4C1B0 8005B9B0 1400BF8F */  lw         $ra, 0x14($sp)
    /* 4C1B4 8005B9B4 1000B08F */  lw         $s0, 0x10($sp)
    /* 4C1B8 8005B9B8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 4C1BC 8005B9BC 0800E003 */  jr         $ra
    /* 4C1C0 8005B9C0 00000000 */   nop
endlabel func_8005B98C
