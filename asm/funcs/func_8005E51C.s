glabel func_8005E51C
    /* 4ED1C 8005E51C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 4ED20 8005E520 1000BFAF */  sw         $ra, 0x10($sp)
    /* 4ED24 8005E524 2110A000 */  addu       $v0, $a1, $zero
    /* 4ED28 8005E528 2138C000 */  addu       $a3, $a2, $zero
    /* 4ED2C 8005E52C FFFF8524 */  addiu      $a1, $a0, -0x1
    /* 4ED30 8005E530 FFFF0424 */  addiu      $a0, $zero, -0x1
    /* 4ED34 8005E534 2678010C */  jal        func_8005E098
    /* 4ED38 8005E538 21304000 */   addu      $a2, $v0, $zero
    /* 4ED3C 8005E53C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 4ED40 8005E540 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 4ED44 8005E544 0800E003 */  jr         $ra
    /* 4ED48 8005E548 00000000 */   nop
endlabel func_8005E51C
