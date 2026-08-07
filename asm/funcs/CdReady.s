glabel CdReady
    /* 70A08 80080208 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70A0C 8008020C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 70A10 80080210 0C04020C */  jal        CD_ready
    /* 70A14 80080214 00000000 */   nop
    /* 70A18 80080218 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70A1C 8008021C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 70A20 80080220 0800E003 */  jr         $ra
    /* 70A24 80080224 00000000 */   nop
endlabel CdReady
