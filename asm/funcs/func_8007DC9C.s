glabel func_8007DC9C
    /* 6E49C 8007DC9C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6E4A0 8007DCA0 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6E4A4 8007DCA4 330A020C */  jal        func_800828CC
    /* 6E4A8 8007DCA8 FFFF0424 */   addiu     $a0, $zero, -0x1
    /* 6E4AC 8007DCAC 0A80033C */  lui        $v1, %hi(D_8009BF8C)
    /* 6E4B0 8007DCB0 8CBF638C */  lw         $v1, %lo(D_8009BF8C)($v1)
    /* 6E4B4 8007DCB4 00000000 */  nop
    /* 6E4B8 8007DCB8 2A186200 */  slt        $v1, $v1, $v0
    /* 6E4BC 8007DCBC 0B006014 */  bnez       $v1, .L8007DCEC
    /* 6E4C0 8007DCC0 00000000 */   nop
    /* 6E4C4 8007DCC4 0A80033C */  lui        $v1, %hi(D_8009BF90)
    /* 6E4C8 8007DCC8 90BF638C */  lw         $v1, %lo(D_8009BF90)($v1)
    /* 6E4CC 8007DCCC 00000000 */  nop
    /* 6E4D0 8007DCD0 01006224 */  addiu      $v0, $v1, 0x1
    /* 6E4D4 8007DCD4 0A80013C */  lui        $at, %hi(D_8009BF90)
    /* 6E4D8 8007DCD8 90BF22AC */  sw         $v0, %lo(D_8009BF90)($at)
    /* 6E4DC 8007DCDC 0F00023C */  lui        $v0, (0xF0000 >> 16)
    /* 6E4E0 8007DCE0 2A104300 */  slt        $v0, $v0, $v1
    /* 6E4E4 8007DCE4 43004010 */  beqz       $v0, .L8007DDF4
    /* 6E4E8 8007DCE8 00000000 */   nop
  .L8007DCEC:
    /* 6E4EC 8007DCEC 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E4F0 8007DCF0 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E4F4 8007DCF4 0180043C */  lui        $a0, %hi(D_80016010)
    /* 6E4F8 8007DCF8 10608424 */  addiu      $a0, $a0, %lo(D_80016010)
    /* 6E4FC 8007DCFC 0000628C */  lw         $v0, 0x0($v1)
    /* 6E500 8007DD00 0A80053C */  lui        $a1, %hi(D_8009BF78)
    /* 6E504 8007DD04 78BFA58C */  lw         $a1, %lo(D_8009BF78)($a1)
    /* 6E508 8007DD08 0A80023C */  lui        $v0, %hi(D_8009BF4C)
    /* 6E50C 8007DD0C 4CBF428C */  lw         $v0, %lo(D_8009BF4C)($v0)
    /* 6E510 8007DD10 0A80083C */  lui        $t0, %hi(D_8009BF7C)
    /* 6E514 8007DD14 7CBF088D */  lw         $t0, %lo(D_8009BF7C)($t0)
    /* 6E518 8007DD18 0000428C */  lw         $v0, 0x0($v0)
    /* 6E51C 8007DD1C 2328A800 */  subu       $a1, $a1, $t0
    /* 6E520 8007DD20 1000A2AF */  sw         $v0, 0x10($sp)
    /* 6E524 8007DD24 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6E528 8007DD28 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6E52C 8007DD2C 0000668C */  lw         $a2, 0x0($v1)
    /* 6E530 8007DD30 0000478C */  lw         $a3, 0x0($v0)
    /* 6E534 8007DD34 82E4010C */  jal        func_80079208
    /* 6E538 8007DD38 3F00A530 */   andi      $a1, $a1, 0x3F
    /* 6E53C 8007DD3C 0A80023C */  lui        $v0, %hi(D_8009BF68)
    /* 6E540 8007DD40 68BF4224 */  addiu      $v0, $v0, %lo(D_8009BF68)
    /* 6E544 8007DD44 0000458C */  lw         $a1, 0x0($v0)
    /* 6E548 8007DD48 0A80063C */  lui        $a2, %hi(D_8009BF6C)
    /* 6E54C 8007DD4C 6CBFC68C */  lw         $a2, %lo(D_8009BF6C)($a2)
    /* 6E550 8007DD50 0A80073C */  lui        $a3, %hi(D_8009BF70)
    /* 6E554 8007DD54 70BFE78C */  lw         $a3, %lo(D_8009BF70)($a3)
    /* 6E558 8007DD58 0180043C */  lui        $a0, %hi(D_80016044)
    /* 6E55C 8007DD5C 44608424 */  addiu      $a0, $a0, %lo(D_80016044)
    /* 6E560 8007DD60 82E4010C */  jal        func_80079208
    /* 6E564 8007DD64 00000000 */   nop
    /* 6E568 8007DD68 0F0B020C */  jal        motion_make_table
    /* 6E56C 8007DD6C 21200000 */   addu      $a0, $zero, $zero
    /* 6E570 8007DD70 0A80013C */  lui        $at, %hi(D_8009BF7C)
    /* 6E574 8007DD74 7CBF20AC */  sw         $zero, %lo(D_8009BF7C)($at)
    /* 6E578 8007DD78 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6E57C 8007DD7C 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6E580 8007DD80 0A80013C */  lui        $at, %hi(D_8009BF88)
    /* 6E584 8007DD84 88BF22AC */  sw         $v0, %lo(D_8009BF88)($at)
    /* 6E588 8007DD88 0A80013C */  lui        $at, %hi(D_8009BF78)
    /* 6E58C 8007DD8C 78BF23AC */  sw         $v1, %lo(D_8009BF78)($at)
    /* 6E590 8007DD90 0A80033C */  lui        $v1, %hi(D_8009BF54)
    /* 6E594 8007DD94 54BF638C */  lw         $v1, %lo(D_8009BF54)($v1)
    /* 6E598 8007DD98 01040224 */  addiu      $v0, $zero, 0x401
    /* 6E59C 8007DD9C 000062AC */  sw         $v0, 0x0($v1)
    /* 6E5A0 8007DDA0 0A80033C */  lui        $v1, %hi(D_8009BF64)
    /* 6E5A4 8007DDA4 64BF638C */  lw         $v1, %lo(D_8009BF64)($v1)
    /* 6E5A8 8007DDA8 00000000 */  nop
    /* 6E5AC 8007DDAC 0000628C */  lw         $v0, 0x0($v1)
    /* 6E5B0 8007DDB0 00000000 */  nop
    /* 6E5B4 8007DDB4 00084234 */  ori        $v0, $v0, 0x800
    /* 6E5B8 8007DDB8 000062AC */  sw         $v0, 0x0($v1)
    /* 6E5BC 8007DDBC 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E5C0 8007DDC0 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E5C4 8007DDC4 0002023C */  lui        $v0, (0x2000000 >> 16)
    /* 6E5C8 8007DDC8 000062AC */  sw         $v0, 0x0($v1)
    /* 6E5CC 8007DDCC 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E5D0 8007DDD0 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E5D4 8007DDD4 0001023C */  lui        $v0, (0x1000000 >> 16)
    /* 6E5D8 8007DDD8 000062AC */  sw         $v0, 0x0($v1)
    /* 6E5DC 8007DDDC 0A80043C */  lui        $a0, %hi(D_8009BF88)
    /* 6E5E0 8007DDE0 88BF848C */  lw         $a0, %lo(D_8009BF88)($a0)
    /* 6E5E4 8007DDE4 0F0B020C */  jal        motion_make_table
    /* 6E5E8 8007DDE8 00000000 */   nop
    /* 6E5EC 8007DDEC 7EF70108 */  j          .L8007DDF8
    /* 6E5F0 8007DDF0 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8007DDF4:
    /* 6E5F4 8007DDF4 21100000 */  addu       $v0, $zero, $zero
  .L8007DDF8:
    /* 6E5F8 8007DDF8 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6E5FC 8007DDFC 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6E600 8007DE00 0800E003 */  jr         $ra
    /* 6E604 8007DE04 00000000 */   nop
endlabel func_8007DC9C
