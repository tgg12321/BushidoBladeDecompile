glabel func_8004939C
    /* 39B9C 8004939C FFFF0424 */  addiu      $a0, $zero, -0x1
    /* 39BA0 800493A0 39000324 */  addiu      $v1, $zero, 0x39
    /* 39BA4 800493A4 0F80023C */  lui        $v0, %hi(D_800EF9F2)
    /* 39BA8 800493A8 F2F94224 */  addiu      $v0, $v0, %lo(D_800EF9F2)
  .L800493AC:
    /* 39BAC 800493AC 000044A4 */  sh         $a0, 0x0($v0)
    /* 39BB0 800493B0 FFFF6324 */  addiu      $v1, $v1, -0x1
    /* 39BB4 800493B4 FDFF6104 */  bgez       $v1, .L800493AC
    /* 39BB8 800493B8 FEFF4224 */   addiu     $v0, $v0, -0x2
    /* 39BBC 800493BC FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 39BC0 800493C0 0F80013C */  lui        $at, %hi(D_800EF9F4)
    /* 39BC4 800493C4 F4F922A4 */  sh         $v0, %lo(D_800EF9F4)($at)
    /* 39BC8 800493C8 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 39BCC 800493CC 1E0382A7 */  sh         $v0, %gp_rel(D_800A33EA)($gp)
    /* 39BD0 800493D0 1C0382A7 */  sh         $v0, %gp_rel(D_800A33E8)($gp)
    /* 39BD4 800493D4 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 39BD8 800493D8 200382AF */  sw         $v0, %gp_rel(D_800A33EC)($gp)
    /* 39BDC 800493DC 0800E003 */  jr         $ra
    /* 39BE0 800493E0 00000000 */   nop
endlabel func_8004939C
