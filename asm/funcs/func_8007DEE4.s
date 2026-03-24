glabel func_8007DEE4
    /* 6E6E4 8007DEE4 F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 6E6E8 8007DEE8 0600C010 */  beqz       $a2, .L8007DF04
    /* 6E6EC 8007DEEC FFFFC224 */   addiu     $v0, $a2, -0x1
    /* 6E6F0 8007DEF0 FFFF0324 */  addiu      $v1, $zero, -0x1
  .L8007DEF4:
    /* 6E6F4 8007DEF4 000085A0 */  sb         $a1, 0x0($a0)
    /* 6E6F8 8007DEF8 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6E6FC 8007DEFC FDFF4314 */  bne        $v0, $v1, .L8007DEF4
    /* 6E700 8007DF00 01008424 */   addiu     $a0, $a0, 0x1
  .L8007DF04:
    /* 6E704 8007DF04 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 6E708 8007DF08 0800E003 */  jr         $ra
    /* 6E70C 8007DF0C 00000000 */   nop
endlabel func_8007DEE4
