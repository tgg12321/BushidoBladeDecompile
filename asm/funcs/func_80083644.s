glabel func_80083644
    /* 73E44 80083644 F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 73E48 80083648 0600A010 */  beqz       $a1, .L80083664
    /* 73E4C 8008364C FFFFA224 */   addiu     $v0, $a1, -0x1
    /* 73E50 80083650 FFFF0324 */  addiu      $v1, $zero, -0x1
  .L80083654:
    /* 73E54 80083654 000080AC */  sw         $zero, 0x0($a0)
    /* 73E58 80083658 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 73E5C 8008365C FDFF4314 */  bne        $v0, $v1, .L80083654
    /* 73E60 80083660 04008424 */   addiu     $a0, $a0, 0x4
  .L80083664:
    /* 73E64 80083664 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 73E68 80083668 0800E003 */  jr         $ra
    /* 73E6C 8008366C 00000000 */   nop
endlabel func_80083644
