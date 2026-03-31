glabel func_8005B9FC
    /* 4C1FC 8005B9FC E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 4C200 8005BA00 1000B0AF */  sw         $s0, 0x10($sp)
    /* 4C204 8005BA04 21808000 */  addu       $s0, $a0, $zero
    /* 4C208 8005BA08 1800BFAF */  sw         $ra, 0x18($sp)
    /* 4C20C 8005BA0C 716E010C */  jal        func_8005B9C4
    /* 4C210 8005BA10 1400B1AF */   sw        $s1, 0x14($sp)
    /* 4C214 8005BA14 02000424 */  addiu      $a0, $zero, 0x2
    /* 4C218 8005BA18 AADB000C */  jal        func_80036EA8
    /* 4C21C 8005BA1C 08000524 */   addiu     $a1, $zero, 0x8
    /* 4C220 8005BA20 D0DB000C */  jal        func_80036F40
    /* 4C224 8005BA24 21884000 */   addu      $s1, $v0, $zero
    /* 4C228 8005BA28 21202002 */  addu       $a0, $s1, $zero
    /* 4C22C 8005BA2C 66DB000C */  jal        replay_camera_Init
    /* 4C230 8005BA30 21280002 */   addu      $a1, $s0, $zero
    /* 4C234 8005BA34 CADB000C */  jal        func_80036F28
    /* 4C238 8005BA38 21202002 */   addu      $a0, $s1, $zero
    /* 4C23C 8005BA3C D0DB000C */  jal        func_80036F40
    /* 4C240 8005BA40 21884000 */   addu      $s1, $v0, $zero
    /* 4C244 8005BA44 21200002 */  addu       $a0, $s0, $zero
    /* 4C248 8005BA48 09000524 */  addiu      $a1, $zero, 0x9
    /* 4C24C 8005BA4C AA70010C */  jal        func_8005C2A8
    /* 4C250 8005BA50 21309100 */   addu      $a2, $a0, $s1
    /* 4C254 8005BA54 1800BF8F */  lw         $ra, 0x18($sp)
    /* 4C258 8005BA58 1400B18F */  lw         $s1, 0x14($sp)
    /* 4C25C 8005BA5C 1000B08F */  lw         $s0, 0x10($sp)
    /* 4C260 8005BA60 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 4C264 8005BA64 0800E003 */  jr         $ra
    /* 4C268 8005BA68 00000000 */   nop
endlabel func_8005B9FC
