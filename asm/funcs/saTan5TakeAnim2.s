glabel saTan5TakeAnim2
    /* 1DD18 8002D518 2148A000 */  addu       $t1, $a1, $zero
    /* 1DD1C 8002D51C 0000C58C */  lw         $a1, 0x0($a2)
    /* 1DD20 8002D520 0000E38C */  lw         $v1, 0x0($a3)
    /* 1DD24 8002D524 00000000 */  nop
    /* 1DD28 8002D528 2A10A300 */  slt        $v0, $a1, $v1
    /* 1DD2C 8002D52C 0A004010 */  beqz       $v0, .L8002D558
    /* 1DD30 8002D530 F8FFBD27 */   addiu     $sp, $sp, -0x8
    /* 1DD34 8002D534 23100400 */  negu       $v0, $a0
    /* 1DD38 8002D538 2A106200 */  slt        $v0, $v1, $v0
    /* 1DD3C 8002D53C 8D004014 */  bnez       $v0, .L8002D774
    /* 1DD40 8002D540 21100000 */   addu      $v0, $zero, $zero
    /* 1DD44 8002D544 2A108500 */  slt        $v0, $a0, $a1
    /* 1DD48 8002D548 0A004010 */  beqz       $v0, .L8002D574
    /* 1DD4C 8002D54C 21100000 */   addu      $v0, $zero, $zero
    /* 1DD50 8002D550 DDB50008 */  j          .L8002D774
    /* 1DD54 8002D554 00000000 */   nop
  .L8002D558:
    /* 1DD58 8002D558 23100400 */  negu       $v0, $a0
    /* 1DD5C 8002D55C 2A10A200 */  slt        $v0, $a1, $v0
    /* 1DD60 8002D560 84004014 */  bnez       $v0, .L8002D774
    /* 1DD64 8002D564 21100000 */   addu      $v0, $zero, $zero
    /* 1DD68 8002D568 2A108300 */  slt        $v0, $a0, $v1
    /* 1DD6C 8002D56C 81004014 */  bnez       $v0, .L8002D774
    /* 1DD70 8002D570 21100000 */   addu      $v0, $zero, $zero
  .L8002D574:
    /* 1DD74 8002D574 0400C58C */  lw         $a1, 0x4($a2)
    /* 1DD78 8002D578 0400E38C */  lw         $v1, 0x4($a3)
    /* 1DD7C 8002D57C 00000000 */  nop
    /* 1DD80 8002D580 2A10A300 */  slt        $v0, $a1, $v1
    /* 1DD84 8002D584 09004010 */  beqz       $v0, .L8002D5AC
    /* 1DD88 8002D588 23100400 */   negu      $v0, $a0
    /* 1DD8C 8002D58C 2A106200 */  slt        $v0, $v1, $v0
    /* 1DD90 8002D590 78004014 */  bnez       $v0, .L8002D774
    /* 1DD94 8002D594 21100000 */   addu      $v0, $zero, $zero
    /* 1DD98 8002D598 2A108500 */  slt        $v0, $a0, $a1
    /* 1DD9C 8002D59C 09004010 */  beqz       $v0, .L8002D5C4
    /* 1DDA0 8002D5A0 21100000 */   addu      $v0, $zero, $zero
    /* 1DDA4 8002D5A4 DDB50008 */  j          .L8002D774
    /* 1DDA8 8002D5A8 00000000 */   nop
  .L8002D5AC:
    /* 1DDAC 8002D5AC 2A10A200 */  slt        $v0, $a1, $v0
    /* 1DDB0 8002D5B0 70004014 */  bnez       $v0, .L8002D774
    /* 1DDB4 8002D5B4 21100000 */   addu      $v0, $zero, $zero
    /* 1DDB8 8002D5B8 2A108300 */  slt        $v0, $a0, $v1
    /* 1DDBC 8002D5BC 6D004014 */  bnez       $v0, .L8002D774
    /* 1DDC0 8002D5C0 21100000 */   addu      $v0, $zero, $zero
  .L8002D5C4:
    /* 1DDC4 8002D5C4 0000E48C */  lw         $a0, 0x0($a3)
    /* 1DDC8 8002D5C8 0000C58C */  lw         $a1, 0x0($a2)
    /* 1DDCC 8002D5CC 00000000 */  nop
    /* 1DDD0 8002D5D0 23208500 */  subu       $a0, $a0, $a1
    /* 1DDD4 8002D5D4 18008400 */  mult       $a0, $a0
    /* 1DDD8 8002D5D8 0400E28C */  lw         $v0, 0x4($a3)
    /* 1DDDC 8002D5DC 0400C38C */  lw         $v1, 0x4($a2)
    /* 1DDE0 8002D5E0 12400000 */  mflo       $t0
    /* 1DDE4 8002D5E4 23104300 */  subu       $v0, $v0, $v1
    /* 1DDE8 8002D5E8 00000000 */  nop
    /* 1DDEC 8002D5EC 18004200 */  mult       $v0, $v0
    /* 1DDF0 8002D5F0 12300000 */  mflo       $a2
    /* 1DDF4 8002D5F4 00000000 */  nop
    /* 1DDF8 8002D5F8 00000000 */  nop
    /* 1DDFC 8002D5FC 18008500 */  mult       $a0, $a1
    /* 1DE00 8002D600 12200000 */  mflo       $a0
    /* 1DE04 8002D604 00000000 */  nop
    /* 1DE08 8002D608 00000000 */  nop
    /* 1DE0C 8002D60C 18004300 */  mult       $v0, $v1
    /* 1DE10 8002D610 12580000 */  mflo       $t3
    /* 1DE14 8002D614 00000000 */  nop
    /* 1DE18 8002D618 00000000 */  nop
    /* 1DE1C 8002D61C 1800A500 */  mult       $a1, $a1
    /* 1DE20 8002D620 12280000 */  mflo       $a1
    /* 1DE24 8002D624 00000000 */  nop
    /* 1DE28 8002D628 00000000 */  nop
    /* 1DE2C 8002D62C 18006300 */  mult       $v1, $v1
    /* 1DE30 8002D630 21108B00 */  addu       $v0, $a0, $t3
    /* 1DE34 8002D634 12180000 */  mflo       $v1
    /* 1DE38 8002D638 40380200 */  sll        $a3, $v0, 1
    /* 1DE3C 8002D63C 43120700 */  sra        $v0, $a3, 9
    /* 1DE40 8002D640 18004200 */  mult       $v0, $v0
    /* 1DE44 8002D644 2110A300 */  addu       $v0, $a1, $v1
    /* 1DE48 8002D648 23104900 */  subu       $v0, $v0, $t1
    /* 1DE4C 8002D64C 43120200 */  sra        $v0, $v0, 9
    /* 1DE50 8002D650 21280601 */  addu       $a1, $t0, $a2
    /* 1DE54 8002D654 12200000 */  mflo       $a0
    /* 1DE58 8002D658 431A0500 */  sra        $v1, $a1, 9
    /* 1DE5C 8002D65C 80100200 */  sll        $v0, $v0, 2
    /* 1DE60 8002D660 18006200 */  mult       $v1, $v0
    /* 1DE64 8002D664 12180000 */  mflo       $v1
    /* 1DE68 8002D668 23308300 */  subu       $a2, $a0, $v1
    /* 1DE6C 8002D66C 0300C104 */  bgez       $a2, .L8002D67C
    /* 1DE70 8002D670 0004C22C */   sltiu     $v0, $a2, 0x400
    /* 1DE74 8002D674 DDB50008 */  j          .L8002D774
    /* 1DE78 8002D678 21100000 */   addu      $v0, $zero, $zero
  .L8002D67C:
    /* 1DE7C 8002D67C 06004010 */  beqz       $v0, .L8002D698
    /* 1DE80 8002D680 2120C000 */   addu      $a0, $a2, $zero
    /* 1DE84 8002D684 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1DE88 8002D688 21082600 */  addu       $at, $at, $a2
    /* 1DE8C 8002D68C 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1DE90 8002D690 BCB50008 */  j          .L8002D6F0
    /* 1DE94 8002D694 C2300200 */   srl       $a2, $v0, 3
  .L8002D698:
    /* 1DE98 8002D698 0800C004 */  bltz       $a2, .L8002D6BC
    /* 1DE9C 8002D69C 21180000 */   addu      $v1, $zero, $zero
    /* 1DEA0 8002D6A0 21608000 */  addu       $t4, $a0, $zero
    /* 1DEA4 8002D6A4 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1DEA8 8002D6A8 00000000 */  nop
    /* 1DEAC 8002D6AC 00000000 */  nop
    /* 1DEB0 8002D6B0 2160A003 */  addu       $t4, $sp, $zero
    /* 1DEB4 8002D6B4 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1DEB8 8002D6B8 0000A38F */  lw         $v1, 0x0($sp)
  .L8002D6BC:
    /* 1DEBC 8002D6BC FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1DEC0 8002D6C0 24106200 */  and        $v0, $v1, $v0
    /* 1DEC4 8002D6C4 16000324 */  addiu      $v1, $zero, 0x16
    /* 1DEC8 8002D6C8 23186200 */  subu       $v1, $v1, $v0
    /* 1DECC 8002D6CC 06106400 */  srlv       $v0, $a0, $v1
    /* 1DED0 8002D6D0 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1DED4 8002D6D4 21082200 */  addu       $at, $at, $v0
    /* 1DED8 8002D6D8 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1DEDC 8002D6DC 42180300 */  srl        $v1, $v1, 1
    /* 1DEE0 8002D6E0 13000224 */  addiu      $v0, $zero, 0x13
    /* 1DEE4 8002D6E4 23104300 */  subu       $v0, $v0, $v1
    /* 1DEE8 8002D6E8 00240400 */  sll        $a0, $a0, 16
    /* 1DEEC 8002D6EC 06304400 */  srlv       $a2, $a0, $v0
  .L8002D6F0:
    /* 1DEF0 8002D6F0 40320600 */  sll        $a2, $a2, 9
    /* 1DEF4 8002D6F4 23100700 */  negu       $v0, $a3
    /* 1DEF8 8002D6F8 21204600 */  addu       $a0, $v0, $a2
    /* 1DEFC 8002D6FC 00220400 */  sll        $a0, $a0, 8
    /* 1DF00 8002D700 40180500 */  sll        $v1, $a1, 1
    /* 1DF04 8002D704 1A008300 */  div        $zero, $a0, $v1
    /* 1DF08 8002D708 02006014 */  bnez       $v1, .L8002D714
    /* 1DF0C 8002D70C 00000000 */   nop
    /* 1DF10 8002D710 0D000700 */  break      7
  .L8002D714:
    /* 1DF14 8002D714 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1DF18 8002D718 04006114 */  bne        $v1, $at, .L8002D72C
    /* 1DF1C 8002D71C 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1DF20 8002D720 02008114 */  bne        $a0, $at, .L8002D72C
    /* 1DF24 8002D724 00000000 */   nop
    /* 1DF28 8002D728 0D000600 */  break      6
  .L8002D72C:
    /* 1DF2C 8002D72C 12200000 */  mflo       $a0
    /* 1DF30 8002D730 23104600 */  subu       $v0, $v0, $a2
    /* 1DF34 8002D734 00120200 */  sll        $v0, $v0, 8
    /* 1DF38 8002D738 1A004300 */  div        $zero, $v0, $v1
    /* 1DF3C 8002D73C 02006014 */  bnez       $v1, .L8002D748
    /* 1DF40 8002D740 00000000 */   nop
    /* 1DF44 8002D744 0D000700 */  break      7
  .L8002D748:
    /* 1DF48 8002D748 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1DF4C 8002D74C 04006114 */  bne        $v1, $at, .L8002D760
    /* 1DF50 8002D750 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1DF54 8002D754 02004114 */  bne        $v0, $at, .L8002D760
    /* 1DF58 8002D758 00000000 */   nop
    /* 1DF5C 8002D75C 0D000600 */  break      6
  .L8002D760:
    /* 1DF60 8002D760 12180000 */  mflo       $v1
    /* 1DF64 8002D764 02008004 */  bltz       $a0, .L8002D770
    /* 1DF68 8002D768 21280000 */   addu      $a1, $zero, $zero
    /* 1DF6C 8002D76C 01016528 */  slti       $a1, $v1, 0x101
  .L8002D770:
    /* 1DF70 8002D770 2110A000 */  addu       $v0, $a1, $zero
  .L8002D774:
    /* 1DF74 8002D774 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 1DF78 8002D778 0800E003 */  jr         $ra
    /* 1DF7C 8002D77C 00000000 */   nop
endlabel saTan5TakeAnim2
