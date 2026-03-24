glabel func_800164F8
    /* 6CF8 800164F8 0F270224 */  addiu      $v0, $zero, 0x270F
  .L800164FC:
    /* 6CFC 800164FC 0D000100 */  break      1
    /* 6D00 80016500 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D04 80016504 FDFF4104 */  bgez       $v0, .L800164FC
    /* 6D08 80016508 00000000 */   nop
    /* 6D0C 8001650C 0800E003 */  jr         $ra
    /* 6D10 80016510 00000000 */   nop
endlabel func_800164F8
