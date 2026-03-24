glabel func_800798CC
    /* 6A0CC 800798CC 21188000 */  addu       $v1, $a0, $zero
    /* 6A0D0 800798D0 FF006230 */  andi       $v0, $v1, 0xFF
    /* 6A0D4 800798D4 0A80013C */  lui        $at, %hi(D_8009BD8D)
    /* 6A0D8 800798D8 21082200 */  addu       $at, $at, $v0
    /* 6A0DC 800798DC 8DBD2290 */  lbu        $v0, %lo(D_8009BD8D)($at)
    /* 6A0E0 800798E0 00000000 */  nop
    /* 6A0E4 800798E4 02004230 */  andi       $v0, $v0, 0x2
    /* 6A0E8 800798E8 02004010 */  beqz       $v0, .L800798F4
    /* 6A0EC 800798EC 00000000 */   nop
    /* 6A0F0 800798F0 E0FF8324 */  addiu      $v1, $a0, -0x20
  .L800798F4:
    /* 6A0F4 800798F4 0800E003 */  jr         $ra
    /* 6A0F8 800798F8 FF006230 */   andi      $v0, $v1, 0xFF
endlabel func_800798CC
