glabel func_80078EC0
    /* 696C0 80078EC0 0A80033C */  lui        $v1, %hi(D_8009BD88)
    /* 696C4 80078EC4 88BD638C */  lw         $v1, %lo(D_8009BD88)($v1)
    /* 696C8 80078EC8 00000000 */  nop
    /* 696CC 80078ECC 0400628C */  lw         $v0, 0x4($v1)
    /* 696D0 80078ED0 00000000 */  nop
    /* 696D4 80078ED4 01004230 */  andi       $v0, $v0, 0x1
    /* 696D8 80078ED8 07004010 */  beqz       $v0, .L80078EF8
    /* 696DC 80078EDC 21100000 */   addu      $v0, $zero, $zero
    /* 696E0 80078EE0 0000628C */  lw         $v0, 0x0($v1)
    /* 696E4 80078EE4 00000000 */  nop
    /* 696E8 80078EE8 01004230 */  andi       $v0, $v0, 0x1
    /* 696EC 80078EEC 02004014 */  bnez       $v0, .L80078EF8
    /* 696F0 80078EF0 01000224 */   addiu     $v0, $zero, 0x1
    /* 696F4 80078EF4 21100000 */  addu       $v0, $zero, $zero
  .L80078EF8:
    /* 696F8 80078EF8 0800E003 */  jr         $ra
    /* 696FC 80078EFC 00000000 */   nop
endlabel func_80078EC0
