glabel func_800457A0
    /* 35FA0 800457A0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 35FA4 800457A4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 35FA8 800457A8 D315010C */  jal        func_8004574C
    /* 35FAC 800457AC 00000000 */   nop
    /* 35FB0 800457B0 03004014 */  bnez       $v0, .L800457C0
    /* 35FB4 800457B4 00000000 */   nop
    /* 35FB8 800457B8 F1150108 */  j          .L800457C4
    /* 35FBC 800457BC 21100000 */   addu      $v0, $zero, $zero
  .L800457C0:
    /* 35FC0 800457C0 0400428C */  lw         $v0, 0x4($v0)
  .L800457C4:
    /* 35FC4 800457C4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 35FC8 800457C8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 35FCC 800457CC 0800E003 */  jr         $ra
    /* 35FD0 800457D0 00000000 */   nop
endlabel func_800457A0
