glabel func_8007C97C
    /* 6D17C 8007C97C 03008014 */  bnez       $a0, .L8007C98C
    /* 6D180 8007C980 F0FFBD27 */   addiu     $sp, $sp, -0x10
    /* 6D184 8007C984 7DF20108 */  j          .L8007C9F4
    /* 6D188 8007C988 21100000 */   addu      $v0, $zero, $zero
  .L8007C98C:
    /* 6D18C 8007C98C 00008590 */  lbu        $a1, 0x0($a0)
    /* 6D190 8007C990 00000000 */  nop
    /* 6D194 8007C994 C2280500 */  srl        $a1, $a1, 3
    /* 6D198 8007C998 0000A5AF */  sw         $a1, 0x0($sp)
    /* 6D19C 8007C99C 04008684 */  lh         $a2, 0x4($a0)
    /* 6D1A0 8007C9A0 00000000 */  nop
    /* 6D1A4 8007C9A4 23300600 */  negu       $a2, $a2
    /* 6D1A8 8007C9A8 FF00C630 */  andi       $a2, $a2, 0xFF
    /* 6D1AC 8007C9AC C3300600 */  sra        $a2, $a2, 3
    /* 6D1B0 8007C9B0 0800A6AF */  sw         $a2, 0x8($sp)
    /* 6D1B4 8007C9B4 02008290 */  lbu        $v0, 0x2($a0)
    /* 6D1B8 8007C9B8 802A0500 */  sll        $a1, $a1, 10
    /* 6D1BC 8007C9BC C2100200 */  srl        $v0, $v0, 3
    /* 6D1C0 8007C9C0 0400A2AF */  sw         $v0, 0x4($sp)
    /* 6D1C4 8007C9C4 C0130200 */  sll        $v0, $v0, 15
    /* 6D1C8 8007C9C8 06008384 */  lh         $v1, 0x6($a0)
    /* 6D1CC 8007C9CC 00E2043C */  lui        $a0, (0xE2000000 >> 16)
    /* 6D1D0 8007C9D0 2528A400 */  or         $a1, $a1, $a0
    /* 6D1D4 8007C9D4 25104500 */  or         $v0, $v0, $a1
    /* 6D1D8 8007C9D8 23180300 */  negu       $v1, $v1
    /* 6D1DC 8007C9DC FF006330 */  andi       $v1, $v1, 0xFF
    /* 6D1E0 8007C9E0 C3180300 */  sra        $v1, $v1, 3
    /* 6D1E4 8007C9E4 40210300 */  sll        $a0, $v1, 5
    /* 6D1E8 8007C9E8 25104400 */  or         $v0, $v0, $a0
    /* 6D1EC 8007C9EC 25104600 */  or         $v0, $v0, $a2
    /* 6D1F0 8007C9F0 0C00A3AF */  sw         $v1, 0xC($sp)
  .L8007C9F4:
    /* 6D1F4 8007C9F4 1000BD27 */  addiu      $sp, $sp, 0x10
    /* 6D1F8 8007C9F8 0800E003 */  jr         $ra
    /* 6D1FC 8007C9FC 00000000 */   nop
endlabel func_8007C97C
