glabel motion_SavePreCalcData_8008003C
    /* 7083C 8008003C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70840 80080040 1000BFAF */  sw         $ra, 0x10($sp)
    /* 70844 80080044 00F0043C */  lui        $a0, (0xF0000003 >> 16)
    /* 70848 80080048 03008434 */  ori        $a0, $a0, (0xF0000003 & 0xFFFF)
    /* 7084C 8008004C 2300020C */  jal        bios_DeliverEvent
    /* 70850 80080050 40000524 */   addiu     $a1, $zero, 0x40
    /* 70854 80080054 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70858 80080058 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7085C 8008005C 0800E003 */  jr         $ra
    /* 70860 80080060 00000000 */   nop
endlabel motion_SavePreCalcData_8008003C
