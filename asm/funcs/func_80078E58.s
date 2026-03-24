glabel func_80078E58
    /* 69658 80078E58 0A80023C */  lui        $v0, %hi(D_8009BD84)
    /* 6965C 80078E5C 84BD428C */  lw         $v0, %lo(D_8009BD84)($v0)
    /* 69660 80078E60 F0FFBD27 */  addiu      $sp, $sp, -0x10
    /* 69664 80078E64 0A0040A4 */  sh         $zero, 0xA($v0)
    /* 69668 80078E68 0A000224 */  addiu      $v0, $zero, 0xA
    /* 6966C 80078E6C 0000A2AF */  sw         $v0, 0x0($sp)
    /* 69670 80078E70 0000A28F */  lw         $v0, 0x0($sp)
    /* 69674 80078E74 00000000 */  nop
    /* 69678 80078E78 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6967C 80078E7C 0000A2AF */  sw         $v0, 0x0($sp)
    /* 69680 80078E80 0000A38F */  lw         $v1, 0x0($sp)
    /* 69684 80078E84 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 69688 80078E88 0A006210 */  beq        $v1, $v0, .L80078EB4
    /* 6968C 80078E8C 21100000 */   addu      $v0, $zero, $zero
    /* 69690 80078E90 FFFF0324 */  addiu      $v1, $zero, -0x1
  .L80078E94:
    /* 69694 80078E94 0000A28F */  lw         $v0, 0x0($sp)
    /* 69698 80078E98 00000000 */  nop
    /* 6969C 80078E9C FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 696A0 80078EA0 0000A2AF */  sw         $v0, 0x0($sp)
    /* 696A4 80078EA4 0000A28F */  lw         $v0, 0x0($sp)
    /* 696A8 80078EA8 00000000 */  nop
    /* 696AC 80078EAC F9FF4314 */  bne        $v0, $v1, .L80078E94
    /* 696B0 80078EB0 21100000 */   addu      $v0, $zero, $zero
  .L80078EB4:
    /* 696B4 80078EB4 1000BD27 */  addiu      $sp, $sp, 0x10
    /* 696B8 80078EB8 0800E003 */  jr         $ra
    /* 696BC 80078EBC 00000000 */   nop
endlabel func_80078E58
