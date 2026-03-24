glabel func_800657B0
    /* 55FB0 800657B0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 55FB4 800657B4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 55FB8 800657B8 0096010C */  jal        func_80065800
    /* 55FBC 800657BC 11000424 */   addiu     $a0, $zero, 0x11
    /* 55FC0 800657C0 0F80043C */  lui        $a0, %hi(D_800F0BCA)
    /* 55FC4 800657C4 CA0B8424 */  addiu      $a0, $a0, %lo(D_800F0BCA)
    /* 55FC8 800657C8 00008394 */  lhu        $v1, 0x0($a0)
    /* 55FCC 800657CC 00000000 */  nop
    /* 55FD0 800657D0 C6016324 */  addiu      $v1, $v1, 0x1C6
    /* 55FD4 800657D4 000083A4 */  sh         $v1, 0x0($a0)
    /* 55FD8 800657D8 001C0300 */  sll        $v1, $v1, 16
    /* 55FDC 800657DC 031C0300 */  sra        $v1, $v1, 16
    /* 55FE0 800657E0 C8116328 */  slti       $v1, $v1, 0x11C8
    /* 55FE4 800657E4 02006014 */  bnez       $v1, .L800657F0
    /* 55FE8 800657E8 FF004230 */   andi      $v0, $v0, 0xFF
    /* 55FEC 800657EC 21100000 */  addu       $v0, $zero, $zero
  .L800657F0:
    /* 55FF0 800657F0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 55FF4 800657F4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 55FF8 800657F8 0800E003 */  jr         $ra
    /* 55FFC 800657FC 00000000 */   nop
endlabel func_800657B0
