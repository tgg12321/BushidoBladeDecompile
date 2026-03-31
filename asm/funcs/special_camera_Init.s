glabel special_camera_Init
    /* 1D558 8002CD58 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 1D55C 8002CD5C 2400B1AF */  sw         $s1, 0x24($sp)
    /* 1D560 8002CD60 21888000 */  addu       $s1, $a0, $zero
    /* 1D564 8002CD64 2800BFAF */  sw         $ra, 0x28($sp)
    /* 1D568 8002CD68 2000B0AF */  sw         $s0, 0x20($sp)
    /* 1D56C 8002CD6C 6400228E */  lw         $v0, 0x64($s1)
    /* 1D570 8002CD70 6000238E */  lw         $v1, 0x60($s1)
    /* 1D574 8002CD74 0000428C */  lw         $v0, 0x0($v0)
    /* 1D578 8002CD78 0000638C */  lw         $v1, 0x0($v1)
    /* 1D57C 8002CD7C 00000000 */  nop
    /* 1D580 8002CD80 23104300 */  subu       $v0, $v0, $v1
    /* 1D584 8002CD84 A80022AE */  sw         $v0, 0xA8($s1)
    /* 1D588 8002CD88 6400228E */  lw         $v0, 0x64($s1)
    /* 1D58C 8002CD8C 6000238E */  lw         $v1, 0x60($s1)
    /* 1D590 8002CD90 0400428C */  lw         $v0, 0x4($v0)
    /* 1D594 8002CD94 0400638C */  lw         $v1, 0x4($v1)
    /* 1D598 8002CD98 00000000 */  nop
    /* 1D59C 8002CD9C 23104300 */  subu       $v0, $v0, $v1
    /* 1D5A0 8002CDA0 AC0022AE */  sw         $v0, 0xAC($s1)
    /* 1D5A4 8002CDA4 6400228E */  lw         $v0, 0x64($s1)
    /* 1D5A8 8002CDA8 6000238E */  lw         $v1, 0x60($s1)
    /* 1D5AC 8002CDAC 0800428C */  lw         $v0, 0x8($v0)
    /* 1D5B0 8002CDB0 0800638C */  lw         $v1, 0x8($v1)
    /* 1D5B4 8002CDB4 00000000 */  nop
    /* 1D5B8 8002CDB8 23104300 */  subu       $v0, $v0, $v1
    /* 1D5BC 8002CDBC B00022AE */  sw         $v0, 0xB0($s1)
    /* 1D5C0 8002CDC0 6800228E */  lw         $v0, 0x68($s1)
    /* 1D5C4 8002CDC4 6000238E */  lw         $v1, 0x60($s1)
    /* 1D5C8 8002CDC8 0000428C */  lw         $v0, 0x0($v0)
    /* 1D5CC 8002CDCC 0000638C */  lw         $v1, 0x0($v1)
    /* 1D5D0 8002CDD0 00000000 */  nop
    /* 1D5D4 8002CDD4 23104300 */  subu       $v0, $v0, $v1
    /* 1D5D8 8002CDD8 B80022AE */  sw         $v0, 0xB8($s1)
    /* 1D5DC 8002CDDC 6800228E */  lw         $v0, 0x68($s1)
    /* 1D5E0 8002CDE0 6000238E */  lw         $v1, 0x60($s1)
    /* 1D5E4 8002CDE4 0400428C */  lw         $v0, 0x4($v0)
    /* 1D5E8 8002CDE8 0400638C */  lw         $v1, 0x4($v1)
    /* 1D5EC 8002CDEC 00000000 */  nop
    /* 1D5F0 8002CDF0 23104300 */  subu       $v0, $v0, $v1
    /* 1D5F4 8002CDF4 BC0022AE */  sw         $v0, 0xBC($s1)
    /* 1D5F8 8002CDF8 6800228E */  lw         $v0, 0x68($s1)
    /* 1D5FC 8002CDFC 6000238E */  lw         $v1, 0x60($s1)
    /* 1D600 8002CE00 0800428C */  lw         $v0, 0x8($v0)
    /* 1D604 8002CE04 0800638C */  lw         $v1, 0x8($v1)
    /* 1D608 8002CE08 00000000 */  nop
    /* 1D60C 8002CE0C 23104300 */  subu       $v0, $v0, $v1
    /* 1D610 8002CE10 C00022AE */  sw         $v0, 0xC0($s1)
    /* 1D614 8002CE14 A8002226 */  addiu      $v0, $s1, 0xA8
    /* 1D618 8002CE18 21604000 */  addu       $t4, $v0, $zero
    /* 1D61C 8002CE1C 00008D8D */  lw         $t5, 0x0($t4)
    /* 1D620 8002CE20 04008E8D */  lw         $t6, 0x4($t4)
    /* 1D624 8002CE24 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 1D628 8002CE28 08008F8D */  lw         $t7, 0x8($t4)
    /* 1D62C 8002CE2C 0010CE48 */  ctc2       $t6, $2 /* handwritten instruction */
    /* 1D630 8002CE30 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 1D634 8002CE34 B8002226 */  addiu      $v0, $s1, 0xB8
    /* 1D638 8002CE38 21604000 */  addu       $t4, $v0, $zero
    /* 1D63C 8002CE3C 08008BC9 */  lwc2       $11, 0x8($t4)
    /* 1D640 8002CE40 000089C9 */  lwc2       $9, 0x0($t4)
    /* 1D644 8002CE44 04008AC9 */  lwc2       $10, 0x4($t4)
    /* 1D648 8002CE48 00000000 */  nop
    /* 1D64C 8002CE4C 00000000 */  nop
    /* 1D650 8002CE50 0C00704B */  op         0
    /* 1D654 8002CE54 C8002226 */  addiu      $v0, $s1, 0xC8
    /* 1D658 8002CE58 21604000 */  addu       $t4, $v0, $zero
    /* 1D65C 8002CE5C 000099E9 */  swc2       $25, 0x0($t4)
    /* 1D660 8002CE60 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1D664 8002CE64 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1D668 8002CE68 C800228E */  lw         $v0, 0xC8($s1)
    /* 1D66C 8002CE6C 00000000 */  nop
    /* 1D670 8002CE70 FF3F4224 */  addiu      $v0, $v0, 0x3FFF
    /* 1D674 8002CE74 FF7F422C */  sltiu      $v0, $v0, 0x7FFF
    /* 1D678 8002CE78 AB004010 */  beqz       $v0, .L8002D128
    /* 1D67C 8002CE7C 00000000 */   nop
    /* 1D680 8002CE80 CC00228E */  lw         $v0, 0xCC($s1)
    /* 1D684 8002CE84 00000000 */  nop
    /* 1D688 8002CE88 FF3F4224 */  addiu      $v0, $v0, 0x3FFF
    /* 1D68C 8002CE8C FF7F422C */  sltiu      $v0, $v0, 0x7FFF
    /* 1D690 8002CE90 A5004010 */  beqz       $v0, .L8002D128
    /* 1D694 8002CE94 00000000 */   nop
    /* 1D698 8002CE98 D000228E */  lw         $v0, 0xD0($s1)
    /* 1D69C 8002CE9C 00000000 */  nop
    /* 1D6A0 8002CEA0 FF3F4224 */  addiu      $v0, $v0, 0x3FFF
    /* 1D6A4 8002CEA4 FF7F422C */  sltiu      $v0, $v0, 0x7FFF
    /* 1D6A8 8002CEA8 9F004010 */  beqz       $v0, .L8002D128
    /* 1D6AC 8002CEAC 00000000 */   nop
    /* 1D6B0 8002CEB0 00000000 */  nop
    /* 1D6B4 8002CEB4 00000000 */  nop
    /* 1D6B8 8002CEB8 2804A04A */  sqr        0
    /* 1D6BC 8002CEBC 00012226 */  addiu      $v0, $s1, 0x100
    /* 1D6C0 8002CEC0 21604000 */  addu       $t4, $v0, $zero
    /* 1D6C4 8002CEC4 000099E9 */  swc2       $25, 0x0($t4)
    /* 1D6C8 8002CEC8 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1D6CC 8002CECC 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1D6D0 8002CED0 0001228E */  lw         $v0, 0x100($s1)
    /* 1D6D4 8002CED4 0401238E */  lw         $v1, 0x104($s1)
    /* 1D6D8 8002CED8 0801248E */  lw         $a0, 0x108($s1)
    /* 1D6DC 8002CEDC 21104300 */  addu       $v0, $v0, $v1
    /* 1D6E0 8002CEE0 21204400 */  addu       $a0, $v0, $a0
    /* 1D6E4 8002CEE4 0004822C */  sltiu      $v0, $a0, 0x400
    /* 1D6E8 8002CEE8 06004010 */  beqz       $v0, .L8002CF04
    /* 1D6EC 8002CEEC 00000000 */   nop
    /* 1D6F0 8002CEF0 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1D6F4 8002CEF4 21082400 */  addu       $at, $at, $a0
    /* 1D6F8 8002CEF8 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1D6FC 8002CEFC D8B30008 */  j          .L8002CF60
    /* 1D700 8002CF00 C2280200 */   srl       $a1, $v0, 3
  .L8002CF04:
    /* 1D704 8002CF04 09008004 */  bltz       $a0, .L8002CF2C
    /* 1D708 8002CF08 21180000 */   addu      $v1, $zero, $zero
    /* 1D70C 8002CF0C 21608000 */  addu       $t4, $a0, $zero
    /* 1D710 8002CF10 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1D714 8002CF14 00000000 */  nop
    /* 1D718 8002CF18 00000000 */  nop
    /* 1D71C 8002CF1C 1000A227 */  addiu      $v0, $sp, 0x10
    /* 1D720 8002CF20 21604000 */  addu       $t4, $v0, $zero
    /* 1D724 8002CF24 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1D728 8002CF28 1000A38F */  lw         $v1, 0x10($sp)
  .L8002CF2C:
    /* 1D72C 8002CF2C FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1D730 8002CF30 24106200 */  and        $v0, $v1, $v0
    /* 1D734 8002CF34 16000324 */  addiu      $v1, $zero, 0x16
    /* 1D738 8002CF38 23186200 */  subu       $v1, $v1, $v0
    /* 1D73C 8002CF3C 06106400 */  srlv       $v0, $a0, $v1
    /* 1D740 8002CF40 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1D744 8002CF44 21082200 */  addu       $at, $at, $v0
    /* 1D748 8002CF48 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1D74C 8002CF4C 42180300 */  srl        $v1, $v1, 1
    /* 1D750 8002CF50 13000224 */  addiu      $v0, $zero, 0x13
    /* 1D754 8002CF54 23104300 */  subu       $v0, $v0, $v1
    /* 1D758 8002CF58 00240400 */  sll        $a0, $a0, 16
    /* 1D75C 8002CF5C 06284400 */  srlv       $a1, $a0, $v0
  .L8002CF60:
    /* 1D760 8002CF60 0040A22C */  sltiu      $v0, $a1, 0x4000
    /* 1D764 8002CF64 70004010 */  beqz       $v0, .L8002D128
    /* 1D768 8002CF68 00000000 */   nop
    /* 1D76C 8002CF6C A800248E */  lw         $a0, 0xA8($s1)
    /* 1D770 8002CF70 B000258E */  lw         $a1, 0xB0($s1)
    /* 1D774 8002CF74 57FF010C */  jal        func_8007FD5C
    /* 1D778 8002CF78 00000000 */   nop
    /* 1D77C 8002CF7C A800238E */  lw         $v1, 0xA8($s1)
    /* 1D780 8002CF80 00000000 */  nop
    /* 1D784 8002CF84 18006300 */  mult       $v1, $v1
    /* 1D788 8002CF88 12200000 */  mflo       $a0
    /* 1D78C 8002CF8C B000238E */  lw         $v1, 0xB0($s1)
    /* 1D790 8002CF90 00000000 */  nop
    /* 1D794 8002CF94 18006300 */  mult       $v1, $v1
    /* 1D798 8002CF98 00080324 */  addiu      $v1, $zero, 0x800
    /* 1D79C 8002CF9C 23186200 */  subu       $v1, $v1, $v0
    /* 1D7A0 8002CFA0 12300000 */  mflo       $a2
    /* 1D7A4 8002CFA4 21208600 */  addu       $a0, $a0, $a2
    /* 1D7A8 8002CFA8 0004822C */  sltiu      $v0, $a0, 0x400
    /* 1D7AC 8002CFAC 06004010 */  beqz       $v0, .L8002CFC8
    /* 1D7B0 8002CFB0 FA0023A6 */   sh        $v1, 0xFA($s1)
    /* 1D7B4 8002CFB4 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1D7B8 8002CFB8 21082400 */  addu       $at, $at, $a0
    /* 1D7BC 8002CFBC 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1D7C0 8002CFC0 09B40008 */  j          .L8002D024
    /* 1D7C4 8002CFC4 C2280200 */   srl       $a1, $v0, 3
  .L8002CFC8:
    /* 1D7C8 8002CFC8 09008004 */  bltz       $a0, .L8002CFF0
    /* 1D7CC 8002CFCC 21180000 */   addu      $v1, $zero, $zero
    /* 1D7D0 8002CFD0 21608000 */  addu       $t4, $a0, $zero
    /* 1D7D4 8002CFD4 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1D7D8 8002CFD8 00000000 */  nop
    /* 1D7DC 8002CFDC 00000000 */  nop
    /* 1D7E0 8002CFE0 1400A227 */  addiu      $v0, $sp, 0x14
    /* 1D7E4 8002CFE4 21604000 */  addu       $t4, $v0, $zero
    /* 1D7E8 8002CFE8 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1D7EC 8002CFEC 1400A38F */  lw         $v1, 0x14($sp)
  .L8002CFF0:
    /* 1D7F0 8002CFF0 FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1D7F4 8002CFF4 24106200 */  and        $v0, $v1, $v0
    /* 1D7F8 8002CFF8 16000324 */  addiu      $v1, $zero, 0x16
    /* 1D7FC 8002CFFC 23186200 */  subu       $v1, $v1, $v0
    /* 1D800 8002D000 06106400 */  srlv       $v0, $a0, $v1
    /* 1D804 8002D004 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1D808 8002D008 21082200 */  addu       $at, $at, $v0
    /* 1D80C 8002D00C 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1D810 8002D010 42180300 */  srl        $v1, $v1, 1
    /* 1D814 8002D014 13000224 */  addiu      $v0, $zero, 0x13
    /* 1D818 8002D018 23104300 */  subu       $v0, $v0, $v1
    /* 1D81C 8002D01C 00240400 */  sll        $a0, $a0, 16
    /* 1D820 8002D020 06284400 */  srlv       $a1, $a0, $v0
  .L8002D024:
    /* 1D824 8002D024 AC00248E */  lw         $a0, 0xAC($s1)
    /* 1D828 8002D028 57FF010C */  jal        func_8007FD5C
    /* 1D82C 8002D02C D8003026 */   addiu     $s0, $s1, 0xD8
    /* 1D830 8002D030 21280002 */  addu       $a1, $s0, $zero
    /* 1D834 8002D034 00080324 */  addiu      $v1, $zero, 0x800
    /* 1D838 8002D038 23186200 */  subu       $v1, $v1, $v0
    /* 1D83C 8002D03C FA002486 */  lh         $a0, 0xFA($s1)
    /* 1D840 8002D040 00100224 */  addiu      $v0, $zero, 0x1000
    /* 1D844 8002D044 F80023A6 */  sh         $v1, 0xF8($s1)
    /* 1D848 8002D048 D80022A6 */  sh         $v0, 0xD8($s1)
    /* 1D84C 8002D04C DA0020A6 */  sh         $zero, 0xDA($s1)
    /* 1D850 8002D050 DC0020A6 */  sh         $zero, 0xDC($s1)
    /* 1D854 8002D054 DE0020A6 */  sh         $zero, 0xDE($s1)
    /* 1D858 8002D058 E00022A6 */  sh         $v0, 0xE0($s1)
    /* 1D85C 8002D05C E20020A6 */  sh         $zero, 0xE2($s1)
    /* 1D860 8002D060 E40020A6 */  sh         $zero, 0xE4($s1)
    /* 1D864 8002D064 E60020A6 */  sh         $zero, 0xE6($s1)
    /* 1D868 8002D068 87FE010C */  jal        func_8007FA1C
    /* 1D86C 8002D06C E80022A6 */   sh        $v0, 0xE8($s1)
    /* 1D870 8002D070 F8002486 */  lh         $a0, 0xF8($s1)
    /* 1D874 8002D074 1FFE010C */  jal        func_8007F87C
    /* 1D878 8002D078 21280002 */   addu      $a1, $s0, $zero
    /* 1D87C 8002D07C 21600002 */  addu       $t4, $s0, $zero
    /* 1D880 8002D080 00008D8D */  lw         $t5, 0x0($t4)
    /* 1D884 8002D084 04008E8D */  lw         $t6, 0x4($t4)
    /* 1D888 8002D088 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 1D88C 8002D08C 0008CE48 */  ctc2       $t6, $1 /* handwritten instruction */
    /* 1D890 8002D090 08008D8D */  lw         $t5, 0x8($t4)
    /* 1D894 8002D094 0C008E8D */  lw         $t6, 0xC($t4)
    /* 1D898 8002D098 10008F8D */  lw         $t7, 0x10($t4)
    /* 1D89C 8002D09C 0010CD48 */  ctc2       $t5, $2 /* handwritten instruction */
    /* 1D8A0 8002D0A0 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 1D8A4 8002D0A4 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 1D8A8 8002D0A8 A8002226 */  addiu      $v0, $s1, 0xA8
    /* 1D8AC 8002D0AC 21604000 */  addu       $t4, $v0, $zero
    /* 1D8B0 8002D0B0 04008E95 */  lhu        $t6, 0x4($t4)
    /* 1D8B4 8002D0B4 00008D95 */  lhu        $t5, 0x0($t4)
    /* 1D8B8 8002D0B8 00740E00 */  sll        $t6, $t6, 16
    /* 1D8BC 8002D0BC 2568AE01 */  or         $t5, $t5, $t6
    /* 1D8C0 8002D0C0 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 1D8C4 8002D0C4 080081C9 */  lwc2       $1, 0x8($t4)
    /* 1D8C8 8002D0C8 00000000 */  nop
    /* 1D8CC 8002D0CC 00000000 */  nop
    /* 1D8D0 8002D0D0 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1D8D4 8002D0D4 21604000 */  addu       $t4, $v0, $zero
    /* 1D8D8 8002D0D8 000099E9 */  swc2       $25, 0x0($t4)
    /* 1D8DC 8002D0DC 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1D8E0 8002D0E0 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1D8E4 8002D0E4 B8002226 */  addiu      $v0, $s1, 0xB8
    /* 1D8E8 8002D0E8 21604000 */  addu       $t4, $v0, $zero
    /* 1D8EC 8002D0EC 04008E95 */  lhu        $t6, 0x4($t4)
    /* 1D8F0 8002D0F0 00008D95 */  lhu        $t5, 0x0($t4)
    /* 1D8F4 8002D0F4 00740E00 */  sll        $t6, $t6, 16
    /* 1D8F8 8002D0F8 2568AE01 */  or         $t5, $t5, $t6
    /* 1D8FC 8002D0FC 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 1D900 8002D100 080081C9 */  lwc2       $1, 0x8($t4)
    /* 1D904 8002D104 00000000 */  nop
    /* 1D908 8002D108 00000000 */  nop
    /* 1D90C 8002D10C 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1D910 8002D110 21604000 */  addu       $t4, $v0, $zero
    /* 1D914 8002D114 000099E9 */  swc2       $25, 0x0($t4)
    /* 1D918 8002D118 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1D91C 8002D11C 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1D920 8002D120 C2B40008 */  j          .L8002D308
    /* 1D924 8002D124 21100000 */   addu      $v0, $zero, $zero
  .L8002D128:
    /* 1D928 8002D128 C800248E */  lw         $a0, 0xC8($s1)
    /* 1D92C 8002D12C D000258E */  lw         $a1, 0xD0($s1)
    /* 1D930 8002D130 57FF010C */  jal        func_8007FD5C
    /* 1D934 8002D134 00000000 */   nop
    /* 1D938 8002D138 C800238E */  lw         $v1, 0xC8($s1)
    /* 1D93C 8002D13C 00000000 */  nop
    /* 1D940 8002D140 83190300 */  sra        $v1, $v1, 6
    /* 1D944 8002D144 C80023AE */  sw         $v1, 0xC8($s1)
    /* 1D948 8002D148 C800238E */  lw         $v1, 0xC8($s1)
    /* 1D94C 8002D14C 00000000 */  nop
    /* 1D950 8002D150 18006300 */  mult       $v1, $v1
    /* 1D954 8002D154 D000258E */  lw         $a1, 0xD0($s1)
    /* 1D958 8002D158 12300000 */  mflo       $a2
    /* 1D95C 8002D15C 83290500 */  sra        $a1, $a1, 6
    /* 1D960 8002D160 00000000 */  nop
    /* 1D964 8002D164 1800A500 */  mult       $a1, $a1
    /* 1D968 8002D168 CC00248E */  lw         $a0, 0xCC($s1)
    /* 1D96C 8002D16C 00000000 */  nop
    /* 1D970 8002D170 83210400 */  sra        $a0, $a0, 6
    /* 1D974 8002D174 CC0024AE */  sw         $a0, 0xCC($s1)
    /* 1D978 8002D178 D00025AE */  sw         $a1, 0xD0($s1)
    /* 1D97C 8002D17C 00080324 */  addiu      $v1, $zero, 0x800
    /* 1D980 8002D180 23186200 */  subu       $v1, $v1, $v0
    /* 1D984 8002D184 12400000 */  mflo       $t0
    /* 1D988 8002D188 2120C800 */  addu       $a0, $a2, $t0
    /* 1D98C 8002D18C 0004822C */  sltiu      $v0, $a0, 0x400
    /* 1D990 8002D190 06004010 */  beqz       $v0, .L8002D1AC
    /* 1D994 8002D194 FA0023A6 */   sh        $v1, 0xFA($s1)
    /* 1D998 8002D198 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1D99C 8002D19C 21082400 */  addu       $at, $at, $a0
    /* 1D9A0 8002D1A0 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1D9A4 8002D1A4 82B40008 */  j          .L8002D208
    /* 1D9A8 8002D1A8 C2280200 */   srl       $a1, $v0, 3
  .L8002D1AC:
    /* 1D9AC 8002D1AC 09008004 */  bltz       $a0, .L8002D1D4
    /* 1D9B0 8002D1B0 21180000 */   addu      $v1, $zero, $zero
    /* 1D9B4 8002D1B4 21608000 */  addu       $t4, $a0, $zero
    /* 1D9B8 8002D1B8 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1D9BC 8002D1BC 00000000 */  nop
    /* 1D9C0 8002D1C0 00000000 */  nop
    /* 1D9C4 8002D1C4 1800A227 */  addiu      $v0, $sp, 0x18
    /* 1D9C8 8002D1C8 21604000 */  addu       $t4, $v0, $zero
    /* 1D9CC 8002D1CC 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1D9D0 8002D1D0 1800A38F */  lw         $v1, 0x18($sp)
  .L8002D1D4:
    /* 1D9D4 8002D1D4 FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1D9D8 8002D1D8 24106200 */  and        $v0, $v1, $v0
    /* 1D9DC 8002D1DC 16000324 */  addiu      $v1, $zero, 0x16
    /* 1D9E0 8002D1E0 23186200 */  subu       $v1, $v1, $v0
    /* 1D9E4 8002D1E4 06106400 */  srlv       $v0, $a0, $v1
    /* 1D9E8 8002D1E8 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1D9EC 8002D1EC 21082200 */  addu       $at, $at, $v0
    /* 1D9F0 8002D1F0 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1D9F4 8002D1F4 42180300 */  srl        $v1, $v1, 1
    /* 1D9F8 8002D1F8 13000224 */  addiu      $v0, $zero, 0x13
    /* 1D9FC 8002D1FC 23104300 */  subu       $v0, $v0, $v1
    /* 1DA00 8002D200 00240400 */  sll        $a0, $a0, 16
    /* 1DA04 8002D204 06284400 */  srlv       $a1, $a0, $v0
  .L8002D208:
    /* 1DA08 8002D208 CC00248E */  lw         $a0, 0xCC($s1)
    /* 1DA0C 8002D20C 57FF010C */  jal        func_8007FD5C
    /* 1DA10 8002D210 D8003026 */   addiu     $s0, $s1, 0xD8
    /* 1DA14 8002D214 21280002 */  addu       $a1, $s0, $zero
    /* 1DA18 8002D218 00080324 */  addiu      $v1, $zero, 0x800
    /* 1DA1C 8002D21C 23186200 */  subu       $v1, $v1, $v0
    /* 1DA20 8002D220 FA002486 */  lh         $a0, 0xFA($s1)
    /* 1DA24 8002D224 00100224 */  addiu      $v0, $zero, 0x1000
    /* 1DA28 8002D228 F80023A6 */  sh         $v1, 0xF8($s1)
    /* 1DA2C 8002D22C D80022A6 */  sh         $v0, 0xD8($s1)
    /* 1DA30 8002D230 DA0020A6 */  sh         $zero, 0xDA($s1)
    /* 1DA34 8002D234 DC0020A6 */  sh         $zero, 0xDC($s1)
    /* 1DA38 8002D238 DE0020A6 */  sh         $zero, 0xDE($s1)
    /* 1DA3C 8002D23C E00022A6 */  sh         $v0, 0xE0($s1)
    /* 1DA40 8002D240 E20020A6 */  sh         $zero, 0xE2($s1)
    /* 1DA44 8002D244 E40020A6 */  sh         $zero, 0xE4($s1)
    /* 1DA48 8002D248 E60020A6 */  sh         $zero, 0xE6($s1)
    /* 1DA4C 8002D24C 87FE010C */  jal        func_8007FA1C
    /* 1DA50 8002D250 E80022A6 */   sh        $v0, 0xE8($s1)
    /* 1DA54 8002D254 F8002486 */  lh         $a0, 0xF8($s1)
    /* 1DA58 8002D258 1FFE010C */  jal        func_8007F87C
    /* 1DA5C 8002D25C 21280002 */   addu      $a1, $s0, $zero
    /* 1DA60 8002D260 21600002 */  addu       $t4, $s0, $zero
    /* 1DA64 8002D264 00008D8D */  lw         $t5, 0x0($t4)
    /* 1DA68 8002D268 04008E8D */  lw         $t6, 0x4($t4)
    /* 1DA6C 8002D26C 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 1DA70 8002D270 0008CE48 */  ctc2       $t6, $1 /* handwritten instruction */
    /* 1DA74 8002D274 08008D8D */  lw         $t5, 0x8($t4)
    /* 1DA78 8002D278 0C008E8D */  lw         $t6, 0xC($t4)
    /* 1DA7C 8002D27C 10008F8D */  lw         $t7, 0x10($t4)
    /* 1DA80 8002D280 0010CD48 */  ctc2       $t5, $2 /* handwritten instruction */
    /* 1DA84 8002D284 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 1DA88 8002D288 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 1DA8C 8002D28C A8002226 */  addiu      $v0, $s1, 0xA8
    /* 1DA90 8002D290 21604000 */  addu       $t4, $v0, $zero
    /* 1DA94 8002D294 04008E95 */  lhu        $t6, 0x4($t4)
    /* 1DA98 8002D298 00008D95 */  lhu        $t5, 0x0($t4)
    /* 1DA9C 8002D29C 00740E00 */  sll        $t6, $t6, 16
    /* 1DAA0 8002D2A0 2568AE01 */  or         $t5, $t5, $t6
    /* 1DAA4 8002D2A4 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 1DAA8 8002D2A8 080081C9 */  lwc2       $1, 0x8($t4)
    /* 1DAAC 8002D2AC 00000000 */  nop
    /* 1DAB0 8002D2B0 00000000 */  nop
    /* 1DAB4 8002D2B4 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1DAB8 8002D2B8 21604000 */  addu       $t4, $v0, $zero
    /* 1DABC 8002D2BC 000099E9 */  swc2       $25, 0x0($t4)
    /* 1DAC0 8002D2C0 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1DAC4 8002D2C4 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1DAC8 8002D2C8 B8002226 */  addiu      $v0, $s1, 0xB8
    /* 1DACC 8002D2CC 21604000 */  addu       $t4, $v0, $zero
    /* 1DAD0 8002D2D0 04008E95 */  lhu        $t6, 0x4($t4)
    /* 1DAD4 8002D2D4 00008D95 */  lhu        $t5, 0x0($t4)
    /* 1DAD8 8002D2D8 00740E00 */  sll        $t6, $t6, 16
    /* 1DADC 8002D2DC 2568AE01 */  or         $t5, $t5, $t6
    /* 1DAE0 8002D2E0 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 1DAE4 8002D2E4 080081C9 */  lwc2       $1, 0x8($t4)
    /* 1DAE8 8002D2E8 00000000 */  nop
    /* 1DAEC 8002D2EC 00000000 */  nop
    /* 1DAF0 8002D2F0 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1DAF4 8002D2F4 21604000 */  addu       $t4, $v0, $zero
    /* 1DAF8 8002D2F8 000099E9 */  swc2       $25, 0x0($t4)
    /* 1DAFC 8002D2FC 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1DB00 8002D300 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1DB04 8002D304 01000224 */  addiu      $v0, $zero, 0x1
  .L8002D308:
    /* 1DB08 8002D308 2800BF8F */  lw         $ra, 0x28($sp)
    /* 1DB0C 8002D30C 2400B18F */  lw         $s1, 0x24($sp)
    /* 1DB10 8002D310 2000B08F */  lw         $s0, 0x20($sp)
    /* 1DB14 8002D314 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 1DB18 8002D318 0800E003 */  jr         $ra
    /* 1DB1C 8002D31C 00000000 */   nop
endlabel special_camera_Init
