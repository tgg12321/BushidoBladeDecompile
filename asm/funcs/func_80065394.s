glabel func_80065394
    /* 55B94 80065394 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 55B98 80065398 1000BFAF */  sw         $ra, 0x10($sp)
    /* 55B9C 8006539C 0096010C */  jal        func_80065800
    /* 55BA0 800653A0 02000424 */   addiu     $a0, $zero, 0x2
    /* 55BA4 800653A4 0F80043C */  lui        $a0, %hi(D_800F0BAC)
    /* 55BA8 800653A8 AC0B8424 */  addiu      $a0, $a0, %lo(D_800F0BAC)
    /* 55BAC 800653AC 00008394 */  lhu        $v1, 0x0($a0)
    /* 55BB0 800653B0 00000000 */  nop
    /* 55BB4 800653B4 C6016324 */  addiu      $v1, $v1, 0x1C6
    /* 55BB8 800653B8 000083A4 */  sh         $v1, 0x0($a0)
    /* 55BBC 800653BC 001C0300 */  sll        $v1, $v1, 16
    /* 55BC0 800653C0 031C0300 */  sra        $v1, $v1, 16
    /* 55BC4 800653C4 C8116328 */  slti       $v1, $v1, 0x11C8
    /* 55BC8 800653C8 02006014 */  bnez       $v1, .L800653D4
    /* 55BCC 800653CC FF004230 */   andi      $v0, $v0, 0xFF
    /* 55BD0 800653D0 21100000 */  addu       $v0, $zero, $zero
  .L800653D4:
    /* 55BD4 800653D4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 55BD8 800653D8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 55BDC 800653DC 0800E003 */  jr         $ra
    /* 55BE0 800653E0 00000000 */   nop
endlabel func_80065394
