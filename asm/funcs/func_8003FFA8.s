glabel func_8003FFA8
    /* 307A8 8003FFA8 03008230 */  andi       $v0, $a0, 0x3
    /* 307AC 8003FFAC 03004010 */  beqz       $v0, .L8003FFBC
    /* 307B0 8003FFB0 03008324 */   addiu     $v1, $a0, 0x3
    /* 307B4 8003FFB4 FCFF0224 */  addiu      $v0, $zero, -0x4
    /* 307B8 8003FFB8 24206200 */  and        $a0, $v1, $v0
  .L8003FFBC:
    /* 307BC 8003FFBC 0800E003 */  jr         $ra
    /* 307C0 8003FFC0 21108000 */   addu      $v0, $a0, $zero
endlabel func_8003FFA8
