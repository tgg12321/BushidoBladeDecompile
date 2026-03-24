glabel func_80066F5C
    /* 5775C 80066F5C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 57760 80066F60 01000424 */  addiu      $a0, $zero, 0x1
    /* 57764 80066F64 01000524 */  addiu      $a1, $zero, 0x1
    /* 57768 80066F68 1000BFAF */  sw         $ra, 0x10($sp)
    /* 5776C 80066F6C 809C010C */  jal        func_80067200
    /* 57770 80066F70 01000624 */   addiu     $a2, $zero, 0x1
    /* 57774 80066F74 02000324 */  addiu      $v1, $zero, 0x2
    /* 57778 80066F78 0F80013C */  lui        $at, %hi(D_800F10DC)
    /* 5777C 80066F7C DC1023AC */  sw         $v1, %lo(D_800F10DC)($at)
    /* 57780 80066F80 1000BF8F */  lw         $ra, 0x10($sp)
    /* 57784 80066F84 FF004230 */  andi       $v0, $v0, 0xFF
    /* 57788 80066F88 0800E003 */  jr         $ra
    /* 5778C 80066F8C 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80066F5C
