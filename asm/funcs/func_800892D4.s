glabel func_800892D4
    /* 79AD4 800892D4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 79AD8 800892D8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 79ADC 800892DC 21288000 */  addu       $a1, $a0, $zero
    /* 79AE0 800892E0 C80A020C */  jal        irq_AcknowledgeVblank
    /* 79AE4 800892E4 04000424 */   addiu     $a0, $zero, 0x4
    /* 79AE8 800892E8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 79AEC 800892EC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 79AF0 800892F0 0800E003 */  jr         $ra
    /* 79AF4 800892F4 00000000 */   nop
endlabel func_800892D4
