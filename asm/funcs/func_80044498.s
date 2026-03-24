glabel func_80044498
    /* 34C98 80044498 13000324 */  addiu      $v1, $zero, 0x13
    /* 34C9C 8004449C 1080023C */  lui        $v0, %hi(D_8010367E)
    /* 34CA0 800444A0 7E364224 */  addiu      $v0, $v0, %lo(D_8010367E)
  .L800444A4:
    /* 34CA4 800444A4 000040A4 */  sh         $zero, 0x0($v0)
    /* 34CA8 800444A8 FFFF6324 */  addiu      $v1, $v1, -0x1
    /* 34CAC 800444AC FDFF6104 */  bgez       $v1, .L800444A4
    /* 34CB0 800444B0 FEFF4224 */   addiu     $v0, $v0, -0x2
    /* 34CB4 800444B4 0800E003 */  jr         $ra
    /* 34CB8 800444B8 00000000 */   nop
endlabel func_80044498
