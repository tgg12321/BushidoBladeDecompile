glabel cdrom_EnableSerialMixAndStartRepeatingAudio
    /* 25DE8 800355E8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 25DEC 800355EC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 25DF0 800355F0 EAD7000C */  jal        snd_SerialMixOn
    /* 25DF4 800355F4 00000000 */   nop
    /* 25DF8 800355F8 44DC000C */  jal        cdrom_StartAudioEntry
    /* 25DFC 800355FC 01000424 */   addiu     $a0, $zero, 0x1
    /* 25E00 80035600 7ADC000C */  jal        cdrom_SetAudioRepeat
    /* 25E04 80035604 01000424 */   addiu     $a0, $zero, 0x1
    /* 25E08 80035608 1000BF8F */  lw         $ra, 0x10($sp)
    /* 25E0C 8003560C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 25E10 80035610 0800E003 */  jr         $ra
    /* 25E14 80035614 00000000 */   nop
endlabel cdrom_EnableSerialMixAndStartRepeatingAudio
