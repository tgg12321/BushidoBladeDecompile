glabel func_8006E534
    /* 5ED34 8006E534 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 5ED38 8006E538 1800B0AF */  sw         $s0, 0x18($sp)
    /* 5ED3C 8006E53C 21808000 */  addu       $s0, $a0, $zero
    /* 5ED40 8006E540 2000B2AF */  sw         $s2, 0x20($sp)
    /* 5ED44 8006E544 2190A000 */  addu       $s2, $a1, $zero
    /* 5ED48 8006E548 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 5ED4C 8006E54C 2188C000 */  addu       $s1, $a2, $zero
    /* 5ED50 8006E550 2400B3AF */  sw         $s3, 0x24($sp)
    /* 5ED54 8006E554 2198E000 */  addu       $s3, $a3, $zero
    /* 5ED58 8006E558 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5ED5C 8006E55C 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5ED60 8006E560 2800BFAF */  sw         $ra, 0x28($sp)
    /* 5ED64 8006E564 11EE010C */  jal        func_8007B844
    /* 5ED68 8006E568 08100524 */   addiu     $a1, $zero, 0x1008
    /* 5ED6C 8006E56C 7F000224 */  addiu      $v0, $zero, 0x7F
    /* 5ED70 8006E570 1C0282A3 */  sb         $v0, %gp_rel(D_800A32E8)($gp)
    /* 5ED74 8006E574 1400228E */  lw         $v0, 0x14($s1)
    /* 5ED78 8006E578 E00490AF */  sw         $s0, %gp_rel(D_800A35AC)($gp)
    /* 5ED7C 8006E57C 58001026 */  addiu      $s0, $s0, 0x58
    /* 5ED80 8006E580 9C0491AF */  sw         $s1, %gp_rel(D_800A3568)($gp)
    /* 5ED84 8006E584 8C0480A7 */  sh         $zero, %gp_rel(D_800A3558)($gp)
    /* 5ED88 8006E588 880480A7 */  sh         $zero, %gp_rel(D_800A3554)($gp)
    /* 5ED8C 8006E58C 1D0280A3 */  sb         $zero, %gp_rel(D_800A32E9)($gp)
    /* 5ED90 8006E590 E40492AF */  sw         $s2, %gp_rel(D_800A35B0)($gp)
    /* 5ED94 8006E594 A00490AF */  sw         $s0, %gp_rel(D_800A356C)($gp)
    /* 5ED98 8006E598 DC0490AF */  sw         $s0, %gp_rel(D_800A35A8)($gp)
    /* 5ED9C 8006E59C 0F004230 */  andi       $v0, $v0, 0xF
    /* 5EDA0 8006E5A0 F00482AF */  sw         $v0, %gp_rel(D_800A35BC)($gp)
    /* 5EDA4 8006E5A4 A81A010C */  jal        snd_StopAll
    /* 5EDA8 8006E5A8 00000000 */   nop
    /* 5EDAC 8006E5AC F004838F */  lw         $v1, %gp_rel(D_800A35BC)($gp)
    /* 5EDB0 8006E5B0 00000000 */  nop
    /* 5EDB4 8006E5B4 0700622C */  sltiu      $v0, $v1, 0x7
    /* 5EDB8 8006E5B8 1B004010 */  beqz       $v0, .L8006E628
    /* 5EDBC 8006E5BC 80100300 */   sll       $v0, $v1, 2
    /* 5EDC0 8006E5C0 0180013C */  lui        $at, %hi(jtbl_800159B0)
    /* 5EDC4 8006E5C4 21082200 */  addu       $at, $at, $v0
    /* 5EDC8 8006E5C8 B059228C */  lw         $v0, %lo(jtbl_800159B0)($at)
    /* 5EDCC 8006E5CC 00000000 */  nop
    /* 5EDD0 8006E5D0 08004000 */  jr         $v0
    /* 5EDD4 8006E5D4 00000000 */   nop
  jlabel .L8006E5D8
    /* 5EDD8 8006E5D8 A004858F */  lw         $a1, %gp_rel(D_800A356C)($gp)
    /* 5EDDC 8006E5DC 88B90108 */  j          .L8006E620
    /* 5EDE0 8006E5E0 05000424 */   addiu     $a0, $zero, 0x5
  jlabel .L8006E5E4
    /* 5EDE4 8006E5E4 A004858F */  lw         $a1, %gp_rel(D_800A356C)($gp)
    /* 5EDE8 8006E5E8 88B90108 */  j          .L8006E620
    /* 5EDEC 8006E5EC 03000424 */   addiu     $a0, $zero, 0x3
  jlabel .L8006E5F0
    /* 5EDF0 8006E5F0 9C04828F */  lw         $v0, %gp_rel(D_800A3568)($gp)
    /* 5EDF4 8006E5F4 00000000 */  nop
    /* 5EDF8 8006E5F8 1400428C */  lw         $v0, 0x14($v0)
    /* 5EDFC 8006E5FC 0200033C */  lui        $v1, (0x20000 >> 16)
    /* 5EE00 8006E600 24104300 */  and        $v0, $v0, $v1
    /* 5EE04 8006E604 04004010 */  beqz       $v0, .L8006E618
    /* 5EE08 8006E608 03000424 */   addiu     $a0, $zero, 0x3
    /* 5EE0C 8006E60C A004858F */  lw         $a1, %gp_rel(D_800A356C)($gp)
    /* 5EE10 8006E610 88B90108 */  j          .L8006E620
    /* 5EE14 8006E614 00000000 */   nop
  jlabel .L8006E618
    /* 5EE18 8006E618 A004858F */  lw         $a1, %gp_rel(D_800A356C)($gp)
    /* 5EE1C 8006E61C 04000424 */  addiu      $a0, $zero, 0x4
  .L8006E620:
    /* 5EE20 8006E620 54BA010C */  jal        func_8006E950
    /* 5EE24 8006E624 00000000 */   nop
  jlabel .L8006E628
    /* 5EE28 8006E628 A004848F */  lw         $a0, %gp_rel(D_800A356C)($gp)
    /* 5EE2C 8006E62C 8ABA010C */  jal        func_8006EA28
    /* 5EE30 8006E630 00000000 */   nop
    /* 5EE34 8006E634 E004858F */  lw         $a1, %gp_rel(D_800A35AC)($gp)
    /* 5EE38 8006E638 21204000 */  addu       $a0, $v0, $zero
    /* 5EE3C 8006E63C A00484AF */  sw         $a0, %gp_rel(D_800A356C)($gp)
    /* 5EE40 8006E640 27B9010C */  jal        func_8006E49C
    /* 5EE44 8006E644 00000000 */   nop
    /* 5EE48 8006E648 21280000 */  addu       $a1, $zero, $zero
    /* 5EE4C 8006E64C 0A80033C */  lui        $v1, %hi(D_8009BC40)
    /* 5EE50 8006E650 40BC6390 */  lbu        $v1, %lo(D_8009BC40)($v1)
    /* 5EE54 8006E654 0A80043C */  lui        $a0, %hi(D_8009BC44)
    /* 5EE58 8006E658 44BC8490 */  lbu        $a0, %lo(D_8009BC44)($a0)
    /* 5EE5C 8006E65C 01000624 */  addiu      $a2, $zero, 0x1
    /* 5EE60 8006E660 A00482AF */  sw         $v0, %gp_rel(D_800A356C)($gp)
    /* 5EE64 8006E664 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 5EE68 8006E668 940482AF */  sw         $v0, %gp_rel(D_800A3560)($gp)
    /* 5EE6C 8006E66C 02000224 */  addiu      $v0, $zero, 0x2
    /* 5EE70 8006E670 A40480A7 */  sh         $zero, %gp_rel(D_800A3570)($gp)
    /* 5EE74 8006E674 AC0480A7 */  sh         $zero, %gp_rel(D_800A3578)($gp)
    /* 5EE78 8006E678 B00480A7 */  sh         $zero, %gp_rel(D_800A357C)($gp)
    /* 5EE7C 8006E67C B40480A7 */  sh         $zero, %gp_rel(D_800A3580)($gp)
    /* 5EE80 8006E680 D40480AF */  sw         $zero, %gp_rel(D_800A35A0)($gp)
    /* 5EE84 8006E684 BC0480A7 */  sh         $zero, %gp_rel(D_800A3588)($gp)
    /* 5EE88 8006E688 C00480A7 */  sh         $zero, %gp_rel(D_800A358C)($gp)
    /* 5EE8C 8006E68C BE0482A7 */  sh         $v0, %gp_rel(D_800A358A)($gp)
    /* 5EE90 8006E690 C20480A7 */  sh         $zero, %gp_rel(D_800A358E)($gp)
    /* 5EE94 8006E694 950483A3 */  sb         $v1, %gp_rel(D_800A3561)($gp)
    /* 5EE98 8006E698 980484A3 */  sb         $a0, %gp_rel(D_800A3564)($gp)
    /* 5EE9C 8006E69C 00140500 */  sll        $v0, $a1, 16
  .L8006E6A0:
    /* 5EEA0 8006E6A0 031C0200 */  sra        $v1, $v0, 16
    /* 5EEA4 8006E6A4 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5EEA8 8006E6A8 21082300 */  addu       $at, $at, $v1
    /* 5EEAC 8006E6AC 7CBC2290 */  lbu        $v0, %lo(D_8009BC7C)($at)
    /* 5EEB0 8006E6B0 00000000 */  nop
    /* 5EEB4 8006E6B4 FA004430 */  andi       $a0, $v0, 0xFA
    /* 5EEB8 8006E6B8 04106600 */  sllv       $v0, $a2, $v1
    /* 5EEBC 8006E6BC 24106202 */  and        $v0, $s3, $v0
    /* 5EEC0 8006E6C0 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5EEC4 8006E6C4 21082300 */  addu       $at, $at, $v1
    /* 5EEC8 8006E6C8 7CBC24A0 */  sb         $a0, %lo(D_8009BC7C)($at)
    /* 5EECC 8006E6CC 06004010 */  beqz       $v0, .L8006E6E8
    /* 5EED0 8006E6D0 0100A224 */   addiu     $v0, $a1, 0x1
    /* 5EED4 8006E6D4 01008234 */  ori        $v0, $a0, 0x1
    /* 5EED8 8006E6D8 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5EEDC 8006E6DC 21082300 */  addu       $at, $at, $v1
    /* 5EEE0 8006E6E0 7CBC22A0 */  sb         $v0, %lo(D_8009BC7C)($at)
    /* 5EEE4 8006E6E4 0100A224 */  addiu      $v0, $a1, 0x1
  .L8006E6E8:
    /* 5EEE8 8006E6E8 21284000 */  addu       $a1, $v0, $zero
    /* 5EEEC 8006E6EC 00140200 */  sll        $v0, $v0, 16
    /* 5EEF0 8006E6F0 03140200 */  sra        $v0, $v0, 16
    /* 5EEF4 8006E6F4 16004228 */  slti       $v0, $v0, 0x16
    /* 5EEF8 8006E6F8 E9FF4014 */  bnez       $v0, .L8006E6A0
    /* 5EEFC 8006E6FC 00140500 */   sll       $v0, $a1, 16
    /* 5EF00 8006E700 F004838F */  lw         $v1, %gp_rel(D_800A35BC)($gp)
    /* 5EF04 8006E704 00000000 */  nop
    /* 5EF08 8006E708 04006228 */  slti       $v0, $v1, 0x4
    /* 5EF0C 8006E70C 17004010 */  beqz       $v0, .L8006E76C
    /* 5EF10 8006E710 00000000 */   nop
    /* 5EF14 8006E714 16006004 */  bltz       $v1, .L8006E770
    /* 5EF18 8006E718 02151300 */   srl       $v0, $s3, 20
    /* 5EF1C 8006E71C 0A80033C */  lui        $v1, %hi(D_8009BC72)
    /* 5EF20 8006E720 72BC6390 */  lbu        $v1, %lo(D_8009BC72)($v1)
    /* 5EF24 8006E724 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5EF28 8006E728 21082300 */  addu       $at, $at, $v1
    /* 5EF2C 8006E72C 7CBC2290 */  lbu        $v0, %lo(D_8009BC7C)($at)
    /* 5EF30 8006E730 00000000 */  nop
    /* 5EF34 8006E734 04004234 */  ori        $v0, $v0, 0x4
    /* 5EF38 8006E738 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5EF3C 8006E73C 21082300 */  addu       $at, $at, $v1
    /* 5EF40 8006E740 7CBC22A0 */  sb         $v0, %lo(D_8009BC7C)($at)
    /* 5EF44 8006E744 0A80033C */  lui        $v1, %hi(D_8009BC76)
    /* 5EF48 8006E748 76BC6390 */  lbu        $v1, %lo(D_8009BC76)($v1)
    /* 5EF4C 8006E74C 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5EF50 8006E750 21082300 */  addu       $at, $at, $v1
    /* 5EF54 8006E754 7CBC2290 */  lbu        $v0, %lo(D_8009BC7C)($at)
    /* 5EF58 8006E758 00000000 */  nop
    /* 5EF5C 8006E75C 04004234 */  ori        $v0, $v0, 0x4
    /* 5EF60 8006E760 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5EF64 8006E764 21082300 */  addu       $at, $at, $v1
    /* 5EF68 8006E768 7CBC22A0 */  sb         $v0, %lo(D_8009BC7C)($at)
  .L8006E76C:
    /* 5EF6C 8006E76C 02151300 */  srl        $v0, $s3, 20
  .L8006E770:
    /* 5EF70 8006E770 BC048387 */  lh         $v1, %gp_rel(D_800A3588)($gp)
    /* 5EF74 8006E774 C0048487 */  lh         $a0, %gp_rel(D_800A358C)($gp)
    /* 5EF78 8006E778 03004230 */  andi       $v0, $v0, 0x3
    /* 5EF7C 8006E77C EC0482A7 */  sh         $v0, %gp_rel(D_800A35B8)($gp)
    /* 5EF80 8006E780 05000224 */  addiu      $v0, $zero, 0x5
    /* 5EF84 8006E784 E80482A7 */  sh         $v0, %gp_rel(D_800A35B4)($gp)
    /* 5EF88 8006E788 40180300 */  sll        $v1, $v1, 1
    /* 5EF8C 8006E78C 40100400 */  sll        $v0, $a0, 1
    /* 5EF90 8006E790 21104400 */  addu       $v0, $v0, $a0
    /* 5EF94 8006E794 80100200 */  sll        $v0, $v0, 2
    /* 5EF98 8006E798 21186200 */  addu       $v1, $v1, $v0
    /* 5EF9C 8006E79C 0A80013C */  lui        $at, %hi(D_8009BC40)
    /* 5EFA0 8006E7A0 21082300 */  addu       $at, $at, $v1
    /* 5EFA4 8006E7A4 40BC2390 */  lbu        $v1, %lo(D_8009BC40)($at)
    /* 5EFA8 8006E7A8 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5EFAC 8006E7AC 21082300 */  addu       $at, $at, $v1
    /* 5EFB0 8006E7B0 7CBC2290 */  lbu        $v0, %lo(D_8009BC7C)($at)
    /* 5EFB4 8006E7B4 00000000 */  nop
    /* 5EFB8 8006E7B8 04004234 */  ori        $v0, $v0, 0x4
    /* 5EFBC 8006E7BC 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5EFC0 8006E7C0 21082300 */  addu       $at, $at, $v1
    /* 5EFC4 8006E7C4 7CBC22A0 */  sb         $v0, %lo(D_8009BC7C)($at)
    /* 5EFC8 8006E7C8 E404828F */  lw         $v0, %gp_rel(D_800A35B0)($gp)
    /* 5EFCC 8006E7CC 00000000 */  nop
    /* 5EFD0 8006E7D0 13004010 */  beqz       $v0, .L8006E820
    /* 5EFD4 8006E7D4 00000000 */   nop
    /* 5EFD8 8006E7D8 BE048387 */  lh         $v1, %gp_rel(D_800A358A)($gp)
    /* 5EFDC 8006E7DC C2048487 */  lh         $a0, %gp_rel(D_800A358E)($gp)
    /* 5EFE0 8006E7E0 40180300 */  sll        $v1, $v1, 1
    /* 5EFE4 8006E7E4 40100400 */  sll        $v0, $a0, 1
    /* 5EFE8 8006E7E8 21104400 */  addu       $v0, $v0, $a0
    /* 5EFEC 8006E7EC 80100200 */  sll        $v0, $v0, 2
    /* 5EFF0 8006E7F0 21186200 */  addu       $v1, $v1, $v0
    /* 5EFF4 8006E7F4 0A80013C */  lui        $at, %hi(D_8009BC40)
    /* 5EFF8 8006E7F8 21082300 */  addu       $at, $at, $v1
    /* 5EFFC 8006E7FC 40BC2390 */  lbu        $v1, %lo(D_8009BC40)($at)
    /* 5F000 8006E800 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5F004 8006E804 21082300 */  addu       $at, $at, $v1
    /* 5F008 8006E808 7CBC2290 */  lbu        $v0, %lo(D_8009BC7C)($at)
    /* 5F00C 8006E80C 00000000 */  nop
    /* 5F010 8006E810 04004234 */  ori        $v0, $v0, 0x4
    /* 5F014 8006E814 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5F018 8006E818 21082300 */  addu       $at, $at, $v1
    /* 5F01C 8006E81C 7CBC22A0 */  sb         $v0, %lo(D_8009BC7C)($at)
  .L8006E820:
    /* 5F020 8006E820 0A80053C */  lui        $a1, %hi(D_800A32EC)
    /* 5F024 8006E824 EC32A524 */  addiu      $a1, $a1, %lo(D_800A32EC)
    /* 5F028 8006E828 0300A288 */  lwl        $v0, 0x3($a1)
    /* 5F02C 8006E82C 0000A298 */  lwr        $v0, 0x0($a1)
    /* 5F030 8006E830 0700A388 */  lwl        $v1, 0x7($a1)
    /* 5F034 8006E834 0400A398 */  lwr        $v1, 0x4($a1)
    /* 5F038 8006E838 1300A2AB */  swl        $v0, 0x13($sp)
    /* 5F03C 8006E83C 1000A2BB */  swr        $v0, 0x10($sp)
    /* 5F040 8006E840 1700A3AB */  swl        $v1, 0x17($sp)
    /* 5F044 8006E844 1400A3BB */  swr        $v1, 0x14($sp)
    /* 5F048 8006E848 D00480A7 */  sh         $zero, %gp_rel(D_800A359C)($gp)
    /* 5F04C 8006E84C CC0480A7 */  sh         $zero, %gp_rel(D_800A3598)($gp)
    /* 5F050 8006E850 CFEC010C */  jal        func_8007B33C
    /* 5F054 8006E854 21200000 */   addu      $a0, $zero, $zero
    /* 5F058 8006E858 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5F05C 8006E85C C0030524 */  addiu      $a1, $zero, 0x3C0
    /* 5F060 8006E860 B2ED010C */  jal        func_8007B6C8
    /* 5F064 8006E864 FE010624 */   addiu     $a2, $zero, 0x1FE
    /* 5F068 8006E868 CFEC010C */  jal        func_8007B33C
    /* 5F06C 8006E86C 21200000 */   addu      $a0, $zero, $zero
    /* 5F070 8006E870 A004848F */  lw         $a0, %gp_rel(D_800A356C)($gp)
    /* 5F074 8006E874 01000224 */  addiu      $v0, $zero, 0x1
    /* 5F078 8006E878 14008324 */  addiu      $v1, $a0, 0x14
    /* 5F07C 8006E87C F80484AF */  sw         $a0, %gp_rel(D_800A35C4)($gp)
    /* 5F080 8006E880 A00483AF */  sw         $v1, %gp_rel(D_800A356C)($gp)
    /* 5F084 8006E884 080080AC */  sw         $zero, 0x8($a0)
    /* 5F088 8006E888 0C0080AC */  sw         $zero, 0xC($a0)
    /* 5F08C 8006E88C 2800BF8F */  lw         $ra, 0x28($sp)
    /* 5F090 8006E890 2400B38F */  lw         $s3, 0x24($sp)
    /* 5F094 8006E894 2000B28F */  lw         $s2, 0x20($sp)
    /* 5F098 8006E898 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 5F09C 8006E89C 1800B08F */  lw         $s0, 0x18($sp)
    /* 5F0A0 8006E8A0 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 5F0A4 8006E8A4 0800E003 */  jr         $ra
    /* 5F0A8 8006E8A8 00000000 */   nop
endlabel func_8006E534
