glabel func_80016888
    /* 7088 80016888 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 708C 8001688C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7090 80016890 A8EC010C */  jal        gpu_SetDispMask
    /* 7094 80016894 21200000 */   addu      $a0, $zero, $zero
    /* 7098 80016898 9FEB010C */  jal        gpu_SetMode
    /* 709C 8001689C 01000424 */   addiu     $a0, $zero, 0x1
    /* 70A0 800168A0 0A80043C */  lui        $a0, %hi(D_800A30CC)
    /* 70A4 800168A4 CC308424 */  addiu      $a0, $a0, %lo(D_800A30CC)
    /* 70A8 800168A8 21280000 */  addu       $a1, $zero, $zero
    /* 70AC 800168AC 21300000 */  addu       $a2, $zero, $zero
    /* 70B0 800168B0 34ED010C */  jal        func_8007B4D0
    /* 70B4 800168B4 21380000 */   addu      $a3, $zero, $zero
    /* 70B8 800168B8 CFEC010C */  jal        gpu_DrawSync
    /* 70BC 800168BC 21200000 */   addu      $a0, $zero, $zero
    /* 70C0 800168C0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70C4 800168C4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 70C8 800168C8 0800E003 */  jr         $ra
    /* 70CC 800168CC 00000000 */   nop
endlabel func_80016888
