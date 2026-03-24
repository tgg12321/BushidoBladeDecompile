glabel func_800885AC
    /* 78DAC 800885AC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 78DB0 800885B0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 78DB4 800885B4 7321020C */  jal        func_800885CC
    /* 78DB8 800885B8 21200000 */   addu      $a0, $zero, $zero
    /* 78DBC 800885BC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 78DC0 800885C0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 78DC4 800885C4 0800E003 */  jr         $ra
    /* 78DC8 800885C8 00000000 */   nop
endlabel func_800885AC
