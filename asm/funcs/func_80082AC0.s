glabel func_80082AC0
    /* 732C0 80082AC0 0A80023C */  lui        $v0, %hi(D_800A2600)
    /* 732C4 80082AC4 0026428C */  lw         $v0, %lo(D_800A2600)($v0)
    /* 732C8 80082AC8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 732CC 80082ACC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 732D0 80082AD0 0C00428C */  lw         $v0, 0xC($v0)
    /* 732D4 80082AD4 00000000 */  nop
    /* 732D8 80082AD8 09F84000 */  jalr       $v0
    /* 732DC 80082ADC 00000000 */   nop
    /* 732E0 80082AE0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 732E4 80082AE4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 732E8 80082AE8 0800E003 */  jr         $ra
    /* 732EC 80082AEC 00000000 */   nop
endlabel func_80082AC0
