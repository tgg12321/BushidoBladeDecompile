glabel func_8007C7A0
    /* 6CFA0 8007C7A0 21388000 */  addu       $a3, $a0, $zero
    /* 6CFA4 8007C7A4 00240400 */  sll        $a0, $a0, 16
    /* 6CFA8 8007C7A8 03240400 */  sra        $a0, $a0, 16
    /* 6CFAC 8007C7AC 0B008004 */  bltz       $a0, .L8007C7DC
    /* 6CFB0 8007C7B0 F0FFBD27 */   addiu     $sp, $sp, -0x10
    /* 6CFB4 8007C7B4 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6CFB8 8007C7B8 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6CFBC 8007C7BC 00000000 */  nop
    /* 6CFC0 8007C7C0 21304000 */  addu       $a2, $v0, $zero
    /* 6CFC4 8007C7C4 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6CFC8 8007C7C8 2A104400 */  slt        $v0, $v0, $a0
    /* 6CFCC 8007C7CC 04004014 */  bnez       $v0, .L8007C7E0
    /* 6CFD0 8007C7D0 FFFFC224 */   addiu     $v0, $a2, -0x1
    /* 6CFD4 8007C7D4 F8F10108 */  j          .L8007C7E0
    /* 6CFD8 8007C7D8 2110E000 */   addu      $v0, $a3, $zero
  .L8007C7DC:
    /* 6CFDC 8007C7DC 21100000 */  addu       $v0, $zero, $zero
  .L8007C7E0:
    /* 6CFE0 8007C7E0 21384000 */  addu       $a3, $v0, $zero
    /* 6CFE4 8007C7E4 00140500 */  sll        $v0, $a1, 16
    /* 6CFE8 8007C7E8 03340200 */  sra        $a2, $v0, 16
    /* 6CFEC 8007C7EC 0B00C004 */  bltz       $a2, .L8007C81C
    /* 6CFF0 8007C7F0 00000000 */   nop
    /* 6CFF4 8007C7F4 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6CFF8 8007C7F8 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6CFFC 8007C7FC 00000000 */  nop
    /* 6D000 8007C800 21204000 */  addu       $a0, $v0, $zero
    /* 6D004 8007C804 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D008 8007C808 2A104600 */  slt        $v0, $v0, $a2
    /* 6D00C 8007C80C 04004010 */  beqz       $v0, .L8007C820
    /* 6D010 8007C810 00000000 */   nop
    /* 6D014 8007C814 08F20108 */  j          .L8007C820
    /* 6D018 8007C818 FFFF8524 */   addiu     $a1, $a0, -0x1
  .L8007C81C:
    /* 6D01C 8007C81C 21280000 */  addu       $a1, $zero, $zero
  .L8007C820:
    /* 6D020 8007C820 0A80023C */  lui        $v0, %hi(D_8009BE74)
    /* 6D024 8007C824 74BE4290 */  lbu        $v0, %lo(D_8009BE74)($v0)
    /* 6D028 8007C828 00000000 */  nop
    /* 6D02C 8007C82C FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D030 8007C830 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6D034 8007C834 05004014 */  bnez       $v0, .L8007C84C
    /* 6D038 8007C838 FF0FA330 */   andi      $v1, $a1, 0xFFF
    /* 6D03C 8007C83C FF03A330 */  andi       $v1, $a1, 0x3FF
    /* 6D040 8007C840 801A0300 */  sll        $v1, $v1, 10
    /* 6D044 8007C844 15F20108 */  j          .L8007C854
    /* 6D048 8007C848 FF03E230 */   andi      $v0, $a3, 0x3FF
  .L8007C84C:
    /* 6D04C 8007C84C 001B0300 */  sll        $v1, $v1, 12
    /* 6D050 8007C850 FF0FE230 */  andi       $v0, $a3, 0xFFF
  .L8007C854:
    /* 6D054 8007C854 00E3043C */  lui        $a0, (0xE3000000 >> 16)
    /* 6D058 8007C858 25104400 */  or         $v0, $v0, $a0
    /* 6D05C 8007C85C 25106200 */  or         $v0, $v1, $v0
    /* 6D060 8007C860 1000BD27 */  addiu      $sp, $sp, 0x10
    /* 6D064 8007C864 0800E003 */  jr         $ra
    /* 6D068 8007C868 00000000 */   nop
endlabel func_8007C7A0
