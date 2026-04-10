glabel func_80080620
    /* 70E20 80080620 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70E24 80080624 1000BFAF */  sw         $ra, 0x10($sp)
    /* 70E28 80080628 4707020C */  jal        cdrom_DmaToRam
    /* 70E2C 8008062C 00000000 */   nop
    /* 70E30 80080630 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70E34 80080634 0100422C */  sltiu      $v0, $v0, 0x1
    /* 70E38 80080638 0800E003 */  jr         $ra
    /* 70E3C 8008063C 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80080620
