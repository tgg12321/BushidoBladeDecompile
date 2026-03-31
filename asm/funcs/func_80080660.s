glabel func_80080660
    /* 70E60 80080660 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70E64 80080664 1000BFAF */  sw         $ra, 0x10($sp)
    /* 70E68 80080668 21288000 */  addu       $a1, $a0, $zero
    /* 70E6C 8008066C C80A020C */  jal        irq_AcknowledgeVblank
    /* 70E70 80080670 03000424 */   addiu     $a0, $zero, 0x3
    /* 70E74 80080674 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70E78 80080678 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 70E7C 8008067C 0800E003 */  jr         $ra
    /* 70E80 80080680 00000000 */   nop
endlabel func_80080660
