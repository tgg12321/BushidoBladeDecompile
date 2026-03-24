glabel func_80049E1C
    /* 3A61C 80049E1C FFFF0424 */  addiu      $a0, $zero, -0x1
    /* 3A620 80049E20 39000324 */  addiu      $v1, $zero, 0x39
    /* 3A624 80049E24 0A80023C */  lui        $v0, %hi(D_80099CC2)
    /* 3A628 80049E28 C29C4224 */  addiu      $v0, $v0, %lo(D_80099CC2)
  .L80049E2C:
    /* 3A62C 80049E2C 000044A4 */  sh         $a0, 0x0($v0)
    /* 3A630 80049E30 FFFF6324 */  addiu      $v1, $v1, -0x1
    /* 3A634 80049E34 FDFF6104 */  bgez       $v1, .L80049E2C
    /* 3A638 80049E38 FEFF4224 */   addiu     $v0, $v0, -0x2
    /* 3A63C 80049E3C FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 3A640 80049E40 800182AF */  sw         $v0, %gp_rel(D_800A324C)($gp)
    /* 3A644 80049E44 0800E003 */  jr         $ra
    /* 3A648 80049E48 00000000 */   nop
endlabel func_80049E1C
