glabel func_8008339C
    /* 73B9C 8008339C F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 73BA0 800833A0 0600A010 */  beqz       $a1, .L800833BC
    /* 73BA4 800833A4 FFFFA224 */   addiu     $v0, $a1, -0x1
    /* 73BA8 800833A8 FFFF0324 */  addiu      $v1, $zero, -0x1
  .L800833AC:
    /* 73BAC 800833AC 000080AC */  sw         $zero, 0x0($a0)
    /* 73BB0 800833B0 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 73BB4 800833B4 FDFF4314 */  bne        $v0, $v1, .L800833AC
    /* 73BB8 800833B8 04008424 */   addiu     $a0, $a0, 0x4
  .L800833BC:
    /* 73BBC 800833BC 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 73BC0 800833C0 0800E003 */  jr         $ra
    /* 73BC4 800833C4 00000000 */   nop
endlabel func_8008339C
