glabel func_8007C86C
    /* 6D06C 8007C86C 21388000 */  addu       $a3, $a0, $zero
    /* 6D070 8007C870 00240400 */  sll        $a0, $a0, 16
    /* 6D074 8007C874 03240400 */  sra        $a0, $a0, 16
    /* 6D078 8007C878 0B008004 */  bltz       $a0, .L8007C8A8
    /* 6D07C 8007C87C F0FFBD27 */   addiu     $sp, $sp, -0x10
    /* 6D080 8007C880 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6D084 8007C884 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6D088 8007C888 00000000 */  nop
    /* 6D08C 8007C88C 21304000 */  addu       $a2, $v0, $zero
    /* 6D090 8007C890 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D094 8007C894 2A104400 */  slt        $v0, $v0, $a0
    /* 6D098 8007C898 04004014 */  bnez       $v0, .L8007C8AC
    /* 6D09C 8007C89C FFFFC224 */   addiu     $v0, $a2, -0x1
    /* 6D0A0 8007C8A0 2BF20108 */  j          .L8007C8AC
    /* 6D0A4 8007C8A4 2110E000 */   addu      $v0, $a3, $zero
  .L8007C8A8:
    /* 6D0A8 8007C8A8 21100000 */  addu       $v0, $zero, $zero
  .L8007C8AC:
    /* 6D0AC 8007C8AC 21384000 */  addu       $a3, $v0, $zero
    /* 6D0B0 8007C8B0 00140500 */  sll        $v0, $a1, 16
    /* 6D0B4 8007C8B4 03340200 */  sra        $a2, $v0, 16
    /* 6D0B8 8007C8B8 0B00C004 */  bltz       $a2, .L8007C8E8
    /* 6D0BC 8007C8BC 00000000 */   nop
    /* 6D0C0 8007C8C0 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6D0C4 8007C8C4 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6D0C8 8007C8C8 00000000 */  nop
    /* 6D0CC 8007C8CC 21204000 */  addu       $a0, $v0, $zero
    /* 6D0D0 8007C8D0 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D0D4 8007C8D4 2A104600 */  slt        $v0, $v0, $a2
    /* 6D0D8 8007C8D8 04004010 */  beqz       $v0, .L8007C8EC
    /* 6D0DC 8007C8DC 00000000 */   nop
    /* 6D0E0 8007C8E0 3BF20108 */  j          .L8007C8EC
    /* 6D0E4 8007C8E4 FFFF8524 */   addiu     $a1, $a0, -0x1
  .L8007C8E8:
    /* 6D0E8 8007C8E8 21280000 */  addu       $a1, $zero, $zero
  .L8007C8EC:
    /* 6D0EC 8007C8EC 0A80023C */  lui        $v0, %hi(D_8009BE74)
    /* 6D0F0 8007C8F0 74BE4290 */  lbu        $v0, %lo(D_8009BE74)($v0)
    /* 6D0F4 8007C8F4 00000000 */  nop
    /* 6D0F8 8007C8F8 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D0FC 8007C8FC 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6D100 8007C900 05004014 */  bnez       $v0, .L8007C918
    /* 6D104 8007C904 FF0FA330 */   andi      $v1, $a1, 0xFFF
    /* 6D108 8007C908 FF03A330 */  andi       $v1, $a1, 0x3FF
    /* 6D10C 8007C90C 801A0300 */  sll        $v1, $v1, 10
    /* 6D110 8007C910 48F20108 */  j          .L8007C920
    /* 6D114 8007C914 FF03E230 */   andi      $v0, $a3, 0x3FF
  .L8007C918:
    /* 6D118 8007C918 001B0300 */  sll        $v1, $v1, 12
    /* 6D11C 8007C91C FF0FE230 */  andi       $v0, $a3, 0xFFF
  .L8007C920:
    /* 6D120 8007C920 00E4043C */  lui        $a0, (0xE4000000 >> 16)
    /* 6D124 8007C924 25104400 */  or         $v0, $v0, $a0
    /* 6D128 8007C928 25106200 */  or         $v0, $v1, $v0
    /* 6D12C 8007C92C 1000BD27 */  addiu      $sp, $sp, 0x10
    /* 6D130 8007C930 0800E003 */  jr         $ra
    /* 6D134 8007C934 00000000 */   nop
endlabel func_8007C86C
