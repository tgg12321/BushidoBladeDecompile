glabel func_8007CE0C
    /* 6D60C 8007CE0C B0FFBD27 */  addiu      $sp, $sp, -0x50
    /* 6D610 8007CE10 3400B1AF */  sw         $s1, 0x34($sp)
    /* 6D614 8007CE14 21888000 */  addu       $s1, $a0, $zero
    /* 6D618 8007CE18 3800B2AF */  sw         $s2, 0x38($sp)
    /* 6D61C 8007CE1C 2190A000 */  addu       $s2, $a1, $zero
    /* 6D620 8007CE20 4800BFAF */  sw         $ra, 0x48($sp)
    /* 6D624 8007CE24 4400B5AF */  sw         $s5, 0x44($sp)
    /* 6D628 8007CE28 4000B4AF */  sw         $s4, 0x40($sp)
    /* 6D62C 8007CE2C 3C00B3AF */  sw         $s3, 0x3C($sp)
    /* 6D630 8007CE30 1AF7010C */  jal        func_8007DC68
    /* 6D634 8007CE34 3000B0AF */   sw        $s0, 0x30($sp)
    /* 6D638 8007CE38 04002586 */  lh         $a1, 0x4($s1)
    /* 6D63C 8007CE3C 21A80000 */  addu       $s5, $zero, $zero
    /* 6D640 8007CE40 0A00A004 */  bltz       $a1, .L8007CE6C
    /* 6D644 8007CE44 2118A000 */   addu      $v1, $a1, $zero
    /* 6D648 8007CE48 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6D64C 8007CE4C 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6D650 8007CE50 00000000 */  nop
    /* 6D654 8007CE54 21204000 */  addu       $a0, $v0, $zero
    /* 6D658 8007CE58 2A104500 */  slt        $v0, $v0, $a1
    /* 6D65C 8007CE5C 04004010 */  beqz       $v0, .L8007CE70
    /* 6D660 8007CE60 00000000 */   nop
    /* 6D664 8007CE64 9CF30108 */  j          .L8007CE70
    /* 6D668 8007CE68 21188000 */   addu      $v1, $a0, $zero
  .L8007CE6C:
    /* 6D66C 8007CE6C 21180000 */  addu       $v1, $zero, $zero
  .L8007CE70:
    /* 6D670 8007CE70 06002586 */  lh         $a1, 0x6($s1)
    /* 6D674 8007CE74 040023A6 */  sh         $v1, 0x4($s1)
    /* 6D678 8007CE78 0A00A004 */  bltz       $a1, .L8007CEA4
    /* 6D67C 8007CE7C 2118A000 */   addu      $v1, $a1, $zero
    /* 6D680 8007CE80 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6D684 8007CE84 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6D688 8007CE88 21206000 */  addu       $a0, $v1, $zero
    /* 6D68C 8007CE8C 21184000 */  addu       $v1, $v0, $zero
    /* 6D690 8007CE90 2A104500 */  slt        $v0, $v0, $a1
    /* 6D694 8007CE94 05004010 */  beqz       $v0, .L8007CEAC
    /* 6D698 8007CE98 00140400 */   sll       $v0, $a0, 16
    /* 6D69C 8007CE9C AAF30108 */  j          .L8007CEA8
    /* 6D6A0 8007CEA0 21206000 */   addu      $a0, $v1, $zero
  .L8007CEA4:
    /* 6D6A4 8007CEA4 21200000 */  addu       $a0, $zero, $zero
  .L8007CEA8:
    /* 6D6A8 8007CEA8 00140400 */  sll        $v0, $a0, 16
  .L8007CEAC:
    /* 6D6AC 8007CEAC 04002386 */  lh         $v1, 0x4($s1)
    /* 6D6B0 8007CEB0 03140200 */  sra        $v0, $v0, 16
    /* 6D6B4 8007CEB4 18006200 */  mult       $v1, $v0
    /* 6D6B8 8007CEB8 060024A6 */  sh         $a0, 0x6($s1)
    /* 6D6BC 8007CEBC 12300000 */  mflo       $a2
    /* 6D6C0 8007CEC0 0100C324 */  addiu      $v1, $a2, 0x1
    /* 6D6C4 8007CEC4 C2170300 */  srl        $v0, $v1, 31
    /* 6D6C8 8007CEC8 21186200 */  addu       $v1, $v1, $v0
    /* 6D6CC 8007CECC 43200300 */  sra        $a0, $v1, 1
    /* 6D6D0 8007CED0 0300801C */  bgtz       $a0, .L8007CEE0
    /* 6D6D4 8007CED4 43810300 */   sra       $s0, $v1, 5
    /* 6D6D8 8007CED8 08F40108 */  j          .L8007D020
    /* 6D6DC 8007CEDC FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8007CEE0:
    /* 6D6E0 8007CEE0 21180002 */  addu       $v1, $s0, $zero
    /* 6D6E4 8007CEE4 00110300 */  sll        $v0, $v1, 4
    /* 6D6E8 8007CEE8 23808200 */  subu       $s0, $a0, $v0
    /* 6D6EC 8007CEEC 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D6F0 8007CEF0 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D6F4 8007CEF4 21A06000 */  addu       $s4, $v1, $zero
    /* 6D6F8 8007CEF8 0000428C */  lw         $v0, 0x0($v0)
    /* 6D6FC 8007CEFC 0004033C */  lui        $v1, (0x4000000 >> 16)
    /* 6D700 8007CF00 24104300 */  and        $v0, $v0, $v1
    /* 6D704 8007CF04 0E004014 */  bnez       $v0, .L8007CF40
    /* 6D708 8007CF08 00A0043C */   lui       $a0, (0xA0000000 >> 16)
    /* 6D70C 8007CF0C 0004133C */  lui        $s3, (0x4000000 >> 16)
  .L8007CF10:
    /* 6D710 8007CF10 27F7010C */  jal        func_8007DC9C
    /* 6D714 8007CF14 00000000 */   nop
    /* 6D718 8007CF18 41004014 */  bnez       $v0, .L8007D020
    /* 6D71C 8007CF1C FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 6D720 8007CF20 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D724 8007CF24 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D728 8007CF28 00000000 */  nop
    /* 6D72C 8007CF2C 0000428C */  lw         $v0, 0x0($v0)
    /* 6D730 8007CF30 00000000 */  nop
    /* 6D734 8007CF34 24105300 */  and        $v0, $v0, $s3
    /* 6D738 8007CF38 F5FF4010 */  beqz       $v0, .L8007CF10
    /* 6D73C 8007CF3C 00A0043C */   lui       $a0, (0xA0000000 >> 16)
  .L8007CF40:
    /* 6D740 8007CF40 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6D744 8007CF44 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6D748 8007CF48 0004023C */  lui        $v0, (0x4000000 >> 16)
    /* 6D74C 8007CF4C 000062AC */  sw         $v0, 0x0($v1)
    /* 6D750 8007CF50 0A80033C */  lui        $v1, %hi(D_8009BF44)
    /* 6D754 8007CF54 44BF638C */  lw         $v1, %lo(D_8009BF44)($v1)
    /* 6D758 8007CF58 0001023C */  lui        $v0, (0x1000000 >> 16)
    /* 6D75C 8007CF5C 000062AC */  sw         $v0, 0x0($v1)
    /* 6D760 8007CF60 0A80023C */  lui        $v0, %hi(D_8009BF44)
    /* 6D764 8007CF64 44BF428C */  lw         $v0, %lo(D_8009BF44)($v0)
    /* 6D768 8007CF68 0200A012 */  beqz       $s5, .L8007CF74
    /* 6D76C 8007CF6C 00000000 */   nop
    /* 6D770 8007CF70 00B0043C */  lui        $a0, (0xB0000000 >> 16)
  .L8007CF74:
    /* 6D774 8007CF74 000044AC */  sw         $a0, 0x0($v0)
    /* 6D778 8007CF78 0A80033C */  lui        $v1, %hi(D_8009BF44)
    /* 6D77C 8007CF7C 44BF638C */  lw         $v1, %lo(D_8009BF44)($v1)
    /* 6D780 8007CF80 0000228E */  lw         $v0, 0x0($s1)
    /* 6D784 8007CF84 00000000 */  nop
    /* 6D788 8007CF88 000062AC */  sw         $v0, 0x0($v1)
    /* 6D78C 8007CF8C 0A80033C */  lui        $v1, %hi(D_8009BF44)
    /* 6D790 8007CF90 44BF638C */  lw         $v1, %lo(D_8009BF44)($v1)
    /* 6D794 8007CF94 0400228E */  lw         $v0, 0x4($s1)
    /* 6D798 8007CF98 FFFF1026 */  addiu      $s0, $s0, -0x1
    /* 6D79C 8007CF9C 000062AC */  sw         $v0, 0x0($v1)
    /* 6D7A0 8007CFA0 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 6D7A4 8007CFA4 0A000212 */  beq        $s0, $v0, .L8007CFD0
    /* 6D7A8 8007CFA8 00000000 */   nop
    /* 6D7AC 8007CFAC FFFF0424 */  addiu      $a0, $zero, -0x1
  .L8007CFB0:
    /* 6D7B0 8007CFB0 0000438E */  lw         $v1, 0x0($s2)
    /* 6D7B4 8007CFB4 04005226 */  addiu      $s2, $s2, 0x4
    /* 6D7B8 8007CFB8 0A80023C */  lui        $v0, %hi(D_8009BF44)
    /* 6D7BC 8007CFBC 44BF428C */  lw         $v0, %lo(D_8009BF44)($v0)
    /* 6D7C0 8007CFC0 FFFF1026 */  addiu      $s0, $s0, -0x1
    /* 6D7C4 8007CFC4 000043AC */  sw         $v1, 0x0($v0)
    /* 6D7C8 8007CFC8 F9FF0416 */  bne        $s0, $a0, .L8007CFB0
    /* 6D7CC 8007CFCC 00000000 */   nop
  .L8007CFD0:
    /* 6D7D0 8007CFD0 12008012 */  beqz       $s4, .L8007D01C
    /* 6D7D4 8007CFD4 0004033C */   lui       $v1, (0x4000002 >> 16)
    /* 6D7D8 8007CFD8 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D7DC 8007CFDC 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D7E0 8007CFE0 02006334 */  ori        $v1, $v1, (0x4000002 & 0xFFFF)
    /* 6D7E4 8007CFE4 000043AC */  sw         $v1, 0x0($v0)
    /* 6D7E8 8007CFE8 0A80023C */  lui        $v0, %hi(D_8009BF4C)
    /* 6D7EC 8007CFEC 4CBF428C */  lw         $v0, %lo(D_8009BF4C)($v0)
    /* 6D7F0 8007CFF0 0001043C */  lui        $a0, (0x1000201 >> 16)
    /* 6D7F4 8007CFF4 000052AC */  sw         $s2, 0x0($v0)
    /* 6D7F8 8007CFF8 00141400 */  sll        $v0, $s4, 16
    /* 6D7FC 8007CFFC 0A80033C */  lui        $v1, %hi(D_8009BF50)
    /* 6D800 8007D000 50BF638C */  lw         $v1, %lo(D_8009BF50)($v1)
    /* 6D804 8007D004 10004234 */  ori        $v0, $v0, 0x10
    /* 6D808 8007D008 000062AC */  sw         $v0, 0x0($v1)
    /* 6D80C 8007D00C 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6D810 8007D010 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6D814 8007D014 01028434 */  ori        $a0, $a0, (0x1000201 & 0xFFFF)
    /* 6D818 8007D018 000044AC */  sw         $a0, 0x0($v0)
  .L8007D01C:
    /* 6D81C 8007D01C 21100000 */  addu       $v0, $zero, $zero
  .L8007D020:
    /* 6D820 8007D020 4800BF8F */  lw         $ra, 0x48($sp)
    /* 6D824 8007D024 4400B58F */  lw         $s5, 0x44($sp)
    /* 6D828 8007D028 4000B48F */  lw         $s4, 0x40($sp)
    /* 6D82C 8007D02C 3C00B38F */  lw         $s3, 0x3C($sp)
    /* 6D830 8007D030 3800B28F */  lw         $s2, 0x38($sp)
    /* 6D834 8007D034 3400B18F */  lw         $s1, 0x34($sp)
    /* 6D838 8007D038 3000B08F */  lw         $s0, 0x30($sp)
    /* 6D83C 8007D03C 5000BD27 */  addiu      $sp, $sp, 0x50
    /* 6D840 8007D040 0800E003 */  jr         $ra
    /* 6D844 8007D044 00000000 */   nop
endlabel func_8007CE0C
