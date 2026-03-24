glabel func_800397A0
    /* 29FA0 800397A0 2C068493 */  lbu        $a0, %gp_rel(D_800A36F8)($gp)
    /* 29FA4 800397A4 77000224 */  addiu      $v0, $zero, 0x77
    /* 29FA8 800397A8 FF008330 */  andi       $v1, $a0, 0xFF
    /* 29FAC 800397AC 06006214 */  bne        $v1, $v0, .L800397C8
    /* 29FB0 800397B0 01008224 */   addiu     $v0, $a0, 0x1
    /* 29FB4 800397B4 01000224 */  addiu      $v0, $zero, 0x1
    /* 29FB8 800397B8 2C0680A3 */  sb         $zero, %gp_rel(D_800A36F8)($gp)
    /* 29FBC 800397BC B60682A3 */  sb         $v0, %gp_rel(D_800A3782)($gp)
    /* 29FC0 800397C0 F3E50008 */  j          .L800397CC
    /* 29FC4 800397C4 00000000 */   nop
  .L800397C8:
    /* 29FC8 800397C8 2C0682A3 */  sb         $v0, %gp_rel(D_800A36F8)($gp)
  .L800397CC:
    /* 29FCC 800397CC 0800E003 */  jr         $ra
    /* 29FD0 800397D0 00000000 */   nop
endlabel func_800397A0
