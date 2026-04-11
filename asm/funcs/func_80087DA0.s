glabel func_80087DA0
    /* 785A0 80087DA0 1080023C */  lui        $v0, %hi(D_80101BCC)
    /* 785A4 80087DA4 CC1B4290 */  lbu        $v0, %lo(D_80101BCC)($v0)
    /* 785A8 80087DA8 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 785AC 80087DAC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 785B0 80087DB0 21800000 */  addu       $s0, $zero, $zero
    /* 785B4 80087DB4 1800BFAF */  sw         $ra, 0x18($sp)
    /* 785B8 80087DB8 1A004010 */  beqz       $v0, .L80087E24
    /* 785BC 80087DBC 1400B1AF */   sw        $s1, 0x14($sp)
    /* 785C0 80087DC0 00140400 */  sll        $v0, $a0, 16
    /* 785C4 80087DC4 038C0200 */  sra        $s1, $v0, 16
    /* 785C8 80087DC8 FF000232 */  andi       $v0, $s0, 0xFF
  .L80087DCC:
    /* 785CC 80087DCC C0180200 */  sll        $v1, $v0, 3
    /* 785D0 80087DD0 23186200 */  subu       $v1, $v1, $v0
    /* 785D4 80087DD4 80180300 */  sll        $v1, $v1, 2
    /* 785D8 80087DD8 23186200 */  subu       $v1, $v1, $v0
    /* 785DC 80087DDC 40180300 */  sll        $v1, $v1, 1
    /* 785E0 80087DE0 0F80013C */  lui        $at, %hi(D_800F4E28)
    /* 785E4 80087DE4 21082300 */  addu       $at, $at, $v1
    /* 785E8 80087DE8 284E2284 */  lh         $v0, %lo(D_800F4E28)($at)
    /* 785EC 80087DEC 00000000 */  nop
    /* 785F0 80087DF0 05005114 */  bne        $v0, $s1, .L80087E08
    /* 785F4 80087DF4 FF000232 */   andi      $v0, $s0, 0xFF
    /* 785F8 80087DF8 1080013C */  lui        $at, %hi(D_8010280A)
    /* 785FC 80087DFC 0A2822A4 */  sh         $v0, %lo(D_8010280A)($at)
    /* 78600 80087E00 751C020C */  jal        motutil_GetAngTableNum
    /* 78604 80087E04 21200000 */   addu      $a0, $zero, $zero
  .L80087E08:
    /* 78608 80087E08 01001026 */  addiu      $s0, $s0, 0x1
    /* 7860C 80087E0C 1080033C */  lui        $v1, %hi(D_80101BCC)
    /* 78610 80087E10 CC1B6390 */  lbu        $v1, %lo(D_80101BCC)($v1)
    /* 78614 80087E14 FF000232 */  andi       $v0, $s0, 0xFF
    /* 78618 80087E18 2B104300 */  sltu       $v0, $v0, $v1
    /* 7861C 80087E1C EBFF4014 */  bnez       $v0, .L80087DCC
    /* 78620 80087E20 FF000232 */   andi      $v0, $s0, 0xFF
  .L80087E24:
    /* 78624 80087E24 1800BF8F */  lw         $ra, 0x18($sp)
    /* 78628 80087E28 1400B18F */  lw         $s1, 0x14($sp)
    /* 7862C 80087E2C 1000B08F */  lw         $s0, 0x10($sp)
    /* 78630 80087E30 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 78634 80087E34 0800E003 */  jr         $ra
    /* 78638 80087E38 00000000 */   nop
endlabel func_80087DA0
