glabel motion_SavePreCalcData_80080064
    /* 70864 80080064 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70868 80080068 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7086C 8008006C 00F0043C */  lui        $a0, (0xF0000003 >> 16)
    /* 70870 80080070 03008434 */  ori        $a0, $a0, (0xF0000003 & 0xFFFF)
    /* 70874 80080074 2300020C */  jal        bios_DeliverEvent
    /* 70878 80080078 40000524 */   addiu     $a1, $zero, 0x40
    /* 7087C 8008007C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70880 80080080 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 70884 80080084 0800E003 */  jr         $ra
    /* 70888 80080088 00000000 */   nop
endlabel motion_SavePreCalcData_80080064
