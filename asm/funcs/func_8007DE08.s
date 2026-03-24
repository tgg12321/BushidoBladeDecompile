glabel func_8007DE08
    /* 6E608 8007DE08 0010033C */  lui        $v1, (0x10000007 >> 16)
    /* 6E60C 8007DE0C 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6E610 8007DE10 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6E614 8007DE14 07006334 */  ori        $v1, $v1, (0x10000007 & 0xFFFF)
    /* 6E618 8007DE18 000043AC */  sw         $v1, 0x0($v0)
    /* 6E61C 8007DE1C 0A80053C */  lui        $a1, %hi(D_8009BF44)
    /* 6E620 8007DE20 44BFA58C */  lw         $a1, %lo(D_8009BF44)($a1)
    /* 6E624 8007DE24 FF00033C */  lui        $v1, (0xFFFFFF >> 16)
    /* 6E628 8007DE28 0000A28C */  lw         $v0, 0x0($a1)
    /* 6E62C 8007DE2C FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 6E630 8007DE30 24104300 */  and        $v0, $v0, $v1
    /* 6E634 8007DE34 02000324 */  addiu      $v1, $zero, 0x2
    /* 6E638 8007DE38 1D004310 */  beq        $v0, $v1, .L8007DEB0
    /* 6E63C 8007DE3C 00E1033C */   lui       $v1, (0xE1001000 >> 16)
    /* 6E640 8007DE40 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6E644 8007DE44 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6E648 8007DE48 00000000 */  nop
    /* 6E64C 8007DE4C 0000428C */  lw         $v0, 0x0($v0)
    /* 6E650 8007DE50 00106334 */  ori        $v1, $v1, (0xE1001000 & 0xFFFF)
    /* 6E654 8007DE54 FF3F4230 */  andi       $v0, $v0, 0x3FFF
    /* 6E658 8007DE58 25104300 */  or         $v0, $v0, $v1
    /* 6E65C 8007DE5C 0000A2AC */  sw         $v0, 0x0($a1)
    /* 6E660 8007DE60 0A80023C */  lui        $v0, %hi(D_8009BF44)
    /* 6E664 8007DE64 44BF428C */  lw         $v0, %lo(D_8009BF44)($v0)
    /* 6E668 8007DE68 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E66C 8007DE6C 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E670 8007DE70 0000428C */  lw         $v0, 0x0($v0)
    /* 6E674 8007DE74 0000628C */  lw         $v0, 0x0($v1)
    /* 6E678 8007DE78 00000000 */  nop
    /* 6E67C 8007DE7C 00104230 */  andi       $v0, $v0, 0x1000
    /* 6E680 8007DE80 03004014 */  bnez       $v0, .L8007DE90
    /* 6E684 8007DE84 08008230 */   andi      $v0, $a0, 0x8
    /* 6E688 8007DE88 B7F70108 */  j          .L8007DEDC
    /* 6E68C 8007DE8C 21100000 */   addu      $v0, $zero, $zero
  .L8007DE90:
    /* 6E690 8007DE90 03004014 */  bnez       $v0, .L8007DEA0
    /* 6E694 8007DE94 0020023C */   lui       $v0, (0x20000504 >> 16)
    /* 6E698 8007DE98 B7F70108 */  j          .L8007DEDC
    /* 6E69C 8007DE9C 01000224 */   addiu     $v0, $zero, 0x1
  .L8007DEA0:
    /* 6E6A0 8007DEA0 04054234 */  ori        $v0, $v0, (0x20000504 & 0xFFFF)
    /* 6E6A4 8007DEA4 000062AC */  sw         $v0, 0x0($v1)
    /* 6E6A8 8007DEA8 B7F70108 */  j          .L8007DEDC
    /* 6E6AC 8007DEAC 02000224 */   addiu     $v0, $zero, 0x2
  .L8007DEB0:
    /* 6E6B0 8007DEB0 08008230 */  andi       $v0, $a0, 0x8
    /* 6E6B4 8007DEB4 08004010 */  beqz       $v0, .L8007DED8
    /* 6E6B8 8007DEB8 0009043C */   lui       $a0, (0x9000001 >> 16)
    /* 6E6BC 8007DEBC 01008434 */  ori        $a0, $a0, (0x9000001 & 0xFFFF)
    /* 6E6C0 8007DEC0 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E6C4 8007DEC4 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E6C8 8007DEC8 04000224 */  addiu      $v0, $zero, 0x4
    /* 6E6CC 8007DECC 000064AC */  sw         $a0, 0x0($v1)
    /* 6E6D0 8007DED0 B7F70108 */  j          .L8007DEDC
    /* 6E6D4 8007DED4 00000000 */   nop
  .L8007DED8:
    /* 6E6D8 8007DED8 03000224 */  addiu      $v0, $zero, 0x3
  .L8007DEDC:
    /* 6E6DC 8007DEDC 0800E003 */  jr         $ra
    /* 6E6E0 8007DEE0 00000000 */   nop
endlabel func_8007DE08
