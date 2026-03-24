glabel func_8001A67C
    /* AE7C 8001A67C D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* AE80 8001A680 2400B3AF */  sw         $s3, 0x24($sp)
    /* AE84 8001A684 21988000 */  addu       $s3, $a0, $zero
    /* AE88 8001A688 2000B2AF */  sw         $s2, 0x20($sp)
    /* AE8C 8001A68C 2190C000 */  addu       $s2, $a2, $zero
    /* AE90 8001A690 2800BFAF */  sw         $ra, 0x28($sp)
    /* AE94 8001A694 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* AE98 8001A698 1800B0AF */  sw         $s0, 0x18($sp)
    /* AE9C 8001A69C 0000A68C */  lw         $a2, 0x0($a1)
    /* AEA0 8001A6A0 0000438E */  lw         $v1, 0x0($s2)
    /* AEA4 8001A6A4 0800A48C */  lw         $a0, 0x8($a1)
    /* AEA8 8001A6A8 0800428E */  lw         $v0, 0x8($s2)
    /* AEAC 8001A6AC 2330C300 */  subu       $a2, $a2, $v1
    /* AEB0 8001A6B0 B3690008 */  j          .L8001A6CC
    /* AEB4 8001A6B4 23888200 */   subu      $s1, $a0, $v0
  .L8001A6B8:
    /* AEB8 8001A6B8 2110C200 */  addu       $v0, $a2, $v0
    /* AEBC 8001A6BC 43300200 */  sra        $a2, $v0, 1
    /* AEC0 8001A6C0 C2171100 */  srl        $v0, $s1, 31
    /* AEC4 8001A6C4 21102202 */  addu       $v0, $s1, $v0
    /* AEC8 8001A6C8 43880200 */  sra        $s1, $v0, 1
  .L8001A6CC:
    /* AECC 8001A6CC 0040C224 */  addiu      $v0, $a2, 0x4000
    /* AED0 8001A6D0 00800334 */  ori        $v1, $zero, 0x8000
    /* AED4 8001A6D4 2B106200 */  sltu       $v0, $v1, $v0
    /* AED8 8001A6D8 F7FF4014 */  bnez       $v0, .L8001A6B8
    /* AEDC 8001A6DC C2170600 */   srl       $v0, $a2, 31
    /* AEE0 8001A6E0 00402226 */  addiu      $v0, $s1, 0x4000
    /* AEE4 8001A6E4 2B106200 */  sltu       $v0, $v1, $v0
    /* AEE8 8001A6E8 F3FF4014 */  bnez       $v0, .L8001A6B8
    /* AEEC 8001A6EC C2170600 */   srl       $v0, $a2, 31
    /* AEF0 8001A6F0 1800C600 */  mult       $a2, $a2
    /* AEF4 8001A6F4 12100000 */  mflo       $v0
    /* AEF8 8001A6F8 00000000 */  nop
    /* AEFC 8001A6FC 00000000 */  nop
    /* AF00 8001A700 18003102 */  mult       $s1, $s1
    /* AF04 8001A704 12180000 */  mflo       $v1
    /* AF08 8001A708 21204300 */  addu       $a0, $v0, $v1
    /* AF0C 8001A70C 0004822C */  sltiu      $v0, $a0, 0x400
    /* AF10 8001A710 06004010 */  beqz       $v0, .L8001A72C
    /* AF14 8001A714 00000000 */   nop
    /* AF18 8001A718 0980013C */  lui        $at, %hi(D_8008D118)
    /* AF1C 8001A71C 21082400 */  addu       $at, $at, $a0
    /* AF20 8001A720 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* AF24 8001A724 E0690008 */  j          .L8001A780
    /* AF28 8001A728 C2800200 */   srl       $s0, $v0, 3
  .L8001A72C:
    /* AF2C 8001A72C 21608000 */  addu       $t4, $a0, $zero
    /* AF30 8001A730 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* AF34 8001A734 00000000 */  nop
    /* AF38 8001A738 00000000 */  nop
    /* AF3C 8001A73C 1000A227 */  addiu      $v0, $sp, 0x10
    /* AF40 8001A740 21604000 */  addu       $t4, $v0, $zero
    /* AF44 8001A744 00009FE9 */  swc2       $31, 0x0($t4)
    /* AF48 8001A748 1000A38F */  lw         $v1, 0x10($sp)
    /* AF4C 8001A74C FEFF0224 */  addiu      $v0, $zero, -0x2
    /* AF50 8001A750 24106200 */  and        $v0, $v1, $v0
    /* AF54 8001A754 16000324 */  addiu      $v1, $zero, 0x16
    /* AF58 8001A758 23186200 */  subu       $v1, $v1, $v0
    /* AF5C 8001A75C 06106400 */  srlv       $v0, $a0, $v1
    /* AF60 8001A760 42180300 */  srl        $v1, $v1, 1
    /* AF64 8001A764 0980013C */  lui        $at, %hi(D_8008D118)
    /* AF68 8001A768 21082200 */  addu       $at, $at, $v0
    /* AF6C 8001A76C 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* AF70 8001A770 13000224 */  addiu      $v0, $zero, 0x13
    /* AF74 8001A774 23104300 */  subu       $v0, $v0, $v1
    /* AF78 8001A778 00240400 */  sll        $a0, $a0, 16
    /* AF7C 8001A77C 06804400 */  srlv       $s0, $a0, $v0
  .L8001A780:
    /* AF80 8001A780 80220600 */  sll        $a0, $a2, 10
    /* AF84 8001A784 1A009000 */  div        $zero, $a0, $s0
    /* AF88 8001A788 02000016 */  bnez       $s0, .L8001A794
    /* AF8C 8001A78C 00000000 */   nop
    /* AF90 8001A790 0D000700 */  break      7
  .L8001A794:
    /* AF94 8001A794 FFFF0124 */  addiu      $at, $zero, -0x1
    /* AF98 8001A798 04000116 */  bne        $s0, $at, .L8001A7AC
    /* AF9C 8001A79C 0080013C */   lui       $at, (0x80000000 >> 16)
    /* AFA0 8001A7A0 02008114 */  bne        $a0, $at, .L8001A7AC
    /* AFA4 8001A7A4 00000000 */   nop
    /* AFA8 8001A7A8 0D000600 */  break      6
  .L8001A7AC:
    /* AFAC 8001A7AC 12200000 */  mflo       $a0
    /* AFB0 8001A7B0 0000428E */  lw         $v0, 0x0($s2)
    /* AFB4 8001A7B4 8B69000C */  jal        func_8001A62C
    /* AFB8 8001A7B8 21204400 */   addu      $a0, $v0, $a0
    /* AFBC 8001A7BC 80221100 */  sll        $a0, $s1, 10
    /* AFC0 8001A7C0 1A009000 */  div        $zero, $a0, $s0
    /* AFC4 8001A7C4 02000016 */  bnez       $s0, .L8001A7D0
    /* AFC8 8001A7C8 00000000 */   nop
    /* AFCC 8001A7CC 0D000700 */  break      7
  .L8001A7D0:
    /* AFD0 8001A7D0 FFFF0124 */  addiu      $at, $zero, -0x1
    /* AFD4 8001A7D4 04000116 */  bne        $s0, $at, .L8001A7E8
    /* AFD8 8001A7D8 0080013C */   lui       $at, (0x80000000 >> 16)
    /* AFDC 8001A7DC 02008114 */  bne        $a0, $at, .L8001A7E8
    /* AFE0 8001A7E0 00000000 */   nop
    /* AFE4 8001A7E4 0D000600 */  break      6
  .L8001A7E8:
    /* AFE8 8001A7E8 12200000 */  mflo       $a0
    /* AFEC 8001A7EC 000062A6 */  sh         $v0, 0x0($s3)
    /* AFF0 8001A7F0 0800428E */  lw         $v0, 0x8($s2)
    /* AFF4 8001A7F4 8B69000C */  jal        func_8001A62C
    /* AFF8 8001A7F8 21204400 */   addu      $a0, $v0, $a0
    /* AFFC 8001A7FC 040062A6 */  sh         $v0, 0x4($s3)
    /* B000 8001A800 2800BF8F */  lw         $ra, 0x28($sp)
    /* B004 8001A804 2400B38F */  lw         $s3, 0x24($sp)
    /* B008 8001A808 2000B28F */  lw         $s2, 0x20($sp)
    /* B00C 8001A80C 1C00B18F */  lw         $s1, 0x1C($sp)
    /* B010 8001A810 1800B08F */  lw         $s0, 0x18($sp)
    /* B014 8001A814 3000BD27 */  addiu      $sp, $sp, 0x30
    /* B018 8001A818 0800E003 */  jr         $ra
    /* B01C 8001A81C 00000000 */   nop
endlabel func_8001A67C
