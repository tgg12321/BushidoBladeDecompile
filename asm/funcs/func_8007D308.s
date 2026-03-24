glabel func_8007D308
    /* 6DB08 8007D308 F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 6DB0C 8007D30C FFFFA624 */  addiu      $a2, $a1, -0x1
    /* 6DB10 8007D310 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6DB14 8007D314 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6DB18 8007D318 0004023C */  lui        $v0, (0x4000000 >> 16)
    /* 6DB1C 8007D31C 000062AC */  sw         $v0, 0x0($v1)
    /* 6DB20 8007D320 0A00A010 */  beqz       $a1, .L8007D34C
    /* 6DB24 8007D324 21100000 */   addu      $v0, $zero, $zero
    /* 6DB28 8007D328 FFFF0524 */  addiu      $a1, $zero, -0x1
  .L8007D32C:
    /* 6DB2C 8007D32C 0000838C */  lw         $v1, 0x0($a0)
    /* 6DB30 8007D330 04008424 */  addiu      $a0, $a0, 0x4
    /* 6DB34 8007D334 0A80023C */  lui        $v0, %hi(D_8009BF44)
    /* 6DB38 8007D338 44BF428C */  lw         $v0, %lo(D_8009BF44)($v0)
    /* 6DB3C 8007D33C FFFFC624 */  addiu      $a2, $a2, -0x1
    /* 6DB40 8007D340 000043AC */  sw         $v1, 0x0($v0)
    /* 6DB44 8007D344 F9FFC514 */  bne        $a2, $a1, .L8007D32C
    /* 6DB48 8007D348 21100000 */   addu      $v0, $zero, $zero
  .L8007D34C:
    /* 6DB4C 8007D34C 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 6DB50 8007D350 0800E003 */  jr         $ra
    /* 6DB54 8007D354 00000000 */   nop
endlabel func_8007D308
