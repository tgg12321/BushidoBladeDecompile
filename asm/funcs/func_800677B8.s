glabel func_800677B8
    /* 57FB8 800677B8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 57FBC 800677BC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 57FC0 800677C0 04000424 */  addiu      $a0, $zero, 0x4
    /* 57FC4 800677C4 2A9E010C */  jal        func_800678A8
    /* 57FC8 800677C8 02000524 */   addiu     $a1, $zero, 0x2
    /* 57FCC 800677CC 04000424 */  addiu      $a0, $zero, 0x4
    /* 57FD0 800677D0 459F010C */  jal        func_80067D14
    /* 57FD4 800677D4 02000524 */   addiu     $a1, $zero, 0x2
    /* 57FD8 800677D8 04000424 */  addiu      $a0, $zero, 0x4
    /* 57FDC 800677DC 62A3010C */  jal        func_80068D88
    /* 57FE0 800677E0 02000524 */   addiu     $a1, $zero, 0x2
    /* 57FE4 800677E4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 57FE8 800677E8 FF004230 */  andi       $v0, $v0, 0xFF
    /* 57FEC 800677EC 0800E003 */  jr         $ra
    /* 57FF0 800677F0 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_800677B8
