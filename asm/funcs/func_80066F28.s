glabel func_80066F28
    /* 57728 80066F28 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 5772C 80066F2C 01000424 */  addiu      $a0, $zero, 0x1
    /* 57730 80066F30 01000524 */  addiu      $a1, $zero, 0x1
    /* 57734 80066F34 1000BFAF */  sw         $ra, 0x10($sp)
    /* 57738 80066F38 809C010C */  jal        func_80067200
    /* 5773C 80066F3C 21300000 */   addu      $a2, $zero, $zero
    /* 57740 80066F40 01000324 */  addiu      $v1, $zero, 0x1
    /* 57744 80066F44 0F80013C */  lui        $at, %hi(D_800F10DC)
    /* 57748 80066F48 DC1023AC */  sw         $v1, %lo(D_800F10DC)($at)
    /* 5774C 80066F4C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 57750 80066F50 FF004230 */  andi       $v0, $v0, 0xFF
    /* 57754 80066F54 0800E003 */  jr         $ra
    /* 57758 80066F58 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80066F28
