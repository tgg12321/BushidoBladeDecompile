glabel func_8005E54C
    /* 4ED4C 8005E54C 48FFBD27 */  addiu      $sp, $sp, -0xB8
    /* 4ED50 8005E550 A800B6AF */  sw         $s6, 0xA8($sp)
    /* 4ED54 8005E554 6800A5AF */  sw         $a1, 0x68($sp)
    /* 4ED58 8005E558 6800B68F */  lw         $s6, 0x68($sp)
    /* 4ED5C 8005E55C A000B4AF */  sw         $s4, 0xA0($sp)
    /* 4ED60 8005E560 21A00000 */  addu       $s4, $zero, $zero
    /* 4ED64 8005E564 B400BFAF */  sw         $ra, 0xB4($sp)
    /* 4ED68 8005E568 B000BEAF */  sw         $fp, 0xB0($sp)
    /* 4ED6C 8005E56C AC00B7AF */  sw         $s7, 0xAC($sp)
    /* 4ED70 8005E570 A400B5AF */  sw         $s5, 0xA4($sp)
    /* 4ED74 8005E574 9C00B3AF */  sw         $s3, 0x9C($sp)
    /* 4ED78 8005E578 9800B2AF */  sw         $s2, 0x98($sp)
    /* 4ED7C 8005E57C 9400B1AF */  sw         $s1, 0x94($sp)
    /* 4ED80 8005E580 9000B0AF */  sw         $s0, 0x90($sp)
    /* 4ED84 8005E584 6000A4AF */  sw         $a0, 0x60($sp)
    /* 4ED88 8005E588 7000A6AF */  sw         $a2, 0x70($sp)
    /* 4ED8C 8005E58C 5000A0A3 */  sb         $zero, 0x50($sp)
    /* 4ED90 8005E590 3800A0AF */  sw         $zero, 0x38($sp)
    /* 4ED94 8005E594 4400A0AF */  sw         $zero, 0x44($sp)
    /* 4ED98 8005E598 A000D226 */  addiu      $s2, $s6, 0xA0
    /* 4ED9C 8005E59C B80BC926 */  addiu      $t1, $s6, 0xBB8
    /* 4EDA0 8005E5A0 8000A9AF */  sw         $t1, 0x80($sp)
    /* 4EDA4 8005E5A4 7000A98F */  lw         $t1, 0x70($sp)
    /* 4EDA8 8005E5A8 9808C826 */  addiu      $t0, $s6, 0x898
    /* 4EDAC 8005E5AC 7800A8AF */  sw         $t0, 0x78($sp)
    /* 4EDB0 8005E5B0 C40BC826 */  addiu      $t0, $s6, 0xBC4
    /* 4EDB4 8005E5B4 8800A8AF */  sw         $t0, 0x88($sp)
    /* 4EDB8 8005E5B8 3C00A9AF */  sw         $t1, 0x3C($sp)
  .L8005E5BC:
    /* 4EDBC 8005E5BC 0A80023C */  lui        $v0, %hi(D_8009BD38)
    /* 4EDC0 8005E5C0 38BD428C */  lw         $v0, %lo(D_8009BD38)($v0)
    /* 4EDC4 8005E5C4 00000000 */  nop
    /* 4EDC8 8005E5C8 C2130200 */  srl        $v0, $v0, 15
    /* 4EDCC 8005E5CC 03004230 */  andi       $v0, $v0, 0x3
    /* 4EDD0 8005E5D0 07108202 */  srav       $v0, $v0, $s4
    /* 4EDD4 8005E5D4 01004230 */  andi       $v0, $v0, 0x1
    /* 4EDD8 8005E5D8 05004014 */  bnez       $v0, .L8005E5F0
    /* 4EDDC 8005E5DC 00000000 */   nop
    /* 4EDE0 8005E5E0 0A80023C */  lui        $v0, %hi(D_8009B524)
    /* 4EDE4 8005E5E4 24B54224 */  addiu      $v0, $v0, %lo(D_8009B524)
    /* 4EDE8 8005E5E8 7F790108 */  j          .L8005E5FC
    /* 4EDEC 8005E5EC 2800A2AF */   sw        $v0, 0x28($sp)
  .L8005E5F0:
    /* 4EDF0 8005E5F0 0A80023C */  lui        $v0, %hi(D_8009B53C)
    /* 4EDF4 8005E5F4 3CB54224 */  addiu      $v0, $v0, %lo(D_8009B53C)
    /* 4EDF8 8005E5F8 2800A2AF */  sw         $v0, 0x28($sp)
  .L8005E5FC:
    /* 4EDFC 8005E5FC 2800A427 */  addiu      $a0, $sp, 0x28
    /* 4EE00 8005E600 00841400 */  sll        $s0, $s4, 16
    /* 4EE04 8005E604 03841000 */  sra        $s0, $s0, 16
    /* 4EE08 8005E608 80101000 */  sll        $v0, $s0, 2
    /* 4EE0C 8005E60C 21105000 */  addu       $v0, $v0, $s0
    /* 4EE10 8005E610 80110200 */  sll        $v0, $v0, 6
    /* 4EE14 8005E614 4000A2AF */  sw         $v0, 0x40($sp)
    /* 4EE18 8005E618 0A80023C */  lui        $v0, %hi(D_8009B554)
    /* 4EE1C 8005E61C 54B54224 */  addiu      $v0, $v0, %lo(D_8009B554)
    /* 4EE20 8005E620 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 4EE24 8005E624 4BCD010C */  jal        func_8007352C
    /* 4EE28 8005E628 3000B2AF */   sw        $s2, 0x30($sp)
    /* 4EE2C 8005E62C 0A80033C */  lui        $v1, %hi(D_8009BD38)
    /* 4EE30 8005E630 38BD638C */  lw         $v1, %lo(D_8009BD38)($v1)
    /* 4EE34 8005E634 00000000 */  nop
    /* 4EE38 8005E638 C21B0300 */  srl        $v1, $v1, 15
    /* 4EE3C 8005E63C 03006330 */  andi       $v1, $v1, 0x3
    /* 4EE40 8005E640 07180302 */  srav       $v1, $v1, $s0
    /* 4EE44 8005E644 01006330 */  andi       $v1, $v1, 0x1
    /* 4EE48 8005E648 08006014 */  bnez       $v1, .L8005E66C
    /* 4EE4C 8005E64C 21904000 */   addu      $s2, $v0, $zero
    /* 4EE50 8005E650 0A80023C */  lui        $v0, %hi(D_8009B530)
    /* 4EE54 8005E654 30B54224 */  addiu      $v0, $v0, %lo(D_8009B530)
    /* 4EE58 8005E658 2800A2AF */  sw         $v0, 0x28($sp)
    /* 4EE5C 8005E65C 0A80023C */  lui        $v0, %hi(D_8009B56C)
    /* 4EE60 8005E660 6CB54224 */  addiu      $v0, $v0, %lo(D_8009B56C)
    /* 4EE64 8005E664 A1790108 */  j          .L8005E684
    /* 4EE68 8005E668 2C00A2AF */   sw        $v0, 0x2C($sp)
  .L8005E66C:
    /* 4EE6C 8005E66C 0A80023C */  lui        $v0, %hi(D_8009B548)
    /* 4EE70 8005E670 48B54224 */  addiu      $v0, $v0, %lo(D_8009B548)
    /* 4EE74 8005E674 2800A2AF */  sw         $v0, 0x28($sp)
    /* 4EE78 8005E678 0A80023C */  lui        $v0, %hi(D_8009B57C)
    /* 4EE7C 8005E67C 7CB54224 */  addiu      $v0, $v0, %lo(D_8009B57C)
    /* 4EE80 8005E680 2C00A2AF */  sw         $v0, 0x2C($sp)
  .L8005E684:
    /* 4EE84 8005E684 3000B2AF */  sw         $s2, 0x30($sp)
    /* 4EE88 8005E688 4BCD010C */  jal        func_8007352C
    /* 4EE8C 8005E68C 2800A427 */   addiu     $a0, $sp, 0x28
    /* 4EE90 8005E690 21904000 */  addu       $s2, $v0, $zero
    /* 4EE94 8005E694 01008226 */  addiu      $v0, $s4, 0x1
    /* 4EE98 8005E698 21A04000 */  addu       $s4, $v0, $zero
    /* 4EE9C 8005E69C 00140200 */  sll        $v0, $v0, 16
    /* 4EEA0 8005E6A0 03140200 */  sra        $v0, $v0, 16
    /* 4EEA4 8005E6A4 02004228 */  slti       $v0, $v0, 0x2
    /* 4EEA8 8005E6A8 C4FF4014 */  bnez       $v0, .L8005E5BC
    /* 4EEAC 8005E6AC 00000000 */   nop
    /* 4EEB0 8005E6B0 0A80053C */  lui        $a1, %hi(D_8009BD38)
    /* 4EEB4 8005E6B4 38BDA524 */  addiu      $a1, $a1, %lo(D_8009BD38)
    /* 4EEB8 8005E6B8 3800A0AF */  sw         $zero, 0x38($sp)
    /* 4EEBC 8005E6BC 5000A0A3 */  sb         $zero, 0x50($sp)
    /* 4EEC0 8005E6C0 0000A28C */  lw         $v0, 0x0($a1)
    /* 4EEC4 8005E6C4 00000000 */  nop
    /* 4EEC8 8005E6C8 82120200 */  srl        $v0, $v0, 10
    /* 4EECC 8005E6CC 03004230 */  andi       $v0, $v0, 0x3
    /* 4EED0 8005E6D0 03004224 */  addiu      $v0, $v0, 0x3
    /* 4EED4 8005E6D4 BC004010 */  beqz       $v0, .L8005E9C8
    /* 4EED8 8005E6D8 21A00000 */   addu      $s4, $zero, $zero
    /* 4EEDC 8005E6DC 1800B327 */  addiu      $s3, $sp, 0x18
    /* 4EEE0 8005E6E0 00141400 */  sll        $v0, $s4, 16
  .L8005E6E4:
    /* 4EEE4 8005E6E4 03240200 */  sra        $a0, $v0, 16
    /* 4EEE8 8005E6E8 6000A88F */  lw         $t0, 0x60($sp)
    /* 4EEEC 8005E6EC 80180400 */  sll        $v1, $a0, 2
    /* 4EEF0 8005E6F0 06106800 */  srlv       $v0, $t0, $v1
    /* 4EEF4 8005E6F4 03004230 */  andi       $v0, $v0, 0x3
    /* 4EEF8 8005E6F8 02006324 */  addiu      $v1, $v1, 0x2
    /* 4EEFC 8005E6FC 06186800 */  srlv       $v1, $t0, $v1
    /* 4EF00 8005E700 03006330 */  andi       $v1, $v1, 0x3
    /* 4EF04 8005E704 1800A2A7 */  sh         $v0, 0x18($sp)
    /* 4EF08 8005E708 1A00A3A7 */  sh         $v1, 0x1A($sp)
    /* 4EF0C 8005E70C 0000A28C */  lw         $v0, 0x0($a1)
    /* 4EF10 8005E710 00000000 */  nop
    /* 4EF14 8005E714 000C4330 */  andi       $v1, $v0, 0xC00
    /* 4EF18 8005E718 00080224 */  addiu      $v0, $zero, 0x800
    /* 4EF1C 8005E71C 05006214 */  bne        $v1, $v0, .L8005E734
    /* 4EF20 8005E720 40100400 */   sll       $v0, $a0, 1
    /* 4EF24 8005E724 21104400 */  addu       $v0, $v0, $a0
    /* 4EF28 8005E728 C0100200 */  sll        $v0, $v0, 3
    /* 4EF2C 8005E72C D7790108 */  j          .L8005E75C
    /* 4EF30 8005E730 44004224 */   addiu     $v0, $v0, 0x44
  .L8005E734:
    /* 4EF34 8005E734 00040224 */  addiu      $v0, $zero, 0x400
    /* 4EF38 8005E738 04006214 */  bne        $v1, $v0, .L8005E74C
    /* 4EF3C 8005E73C 40100400 */   sll       $v0, $a0, 1
    /* 4EF40 8005E740 21104400 */  addu       $v0, $v0, $a0
    /* 4EF44 8005E744 D6790108 */  j          .L8005E758
    /* 4EF48 8005E748 C0100200 */   sll       $v0, $v0, 3
  .L8005E74C:
    /* 4EF4C 8005E74C 00110400 */  sll        $v0, $a0, 4
    /* 4EF50 8005E750 21104400 */  addu       $v0, $v0, $a0
    /* 4EF54 8005E754 40100200 */  sll        $v0, $v0, 1
  .L8005E758:
    /* 4EF58 8005E758 4F004224 */  addiu      $v0, $v0, 0x4F
  .L8005E75C:
    /* 4EF5C 8005E75C 4400A2AF */  sw         $v0, 0x44($sp)
    /* 4EF60 8005E760 1800A287 */  lh         $v0, 0x18($sp)
    /* 4EF64 8005E764 03000324 */  addiu      $v1, $zero, 0x3
    /* 4EF68 8005E768 05004310 */  beq        $v0, $v1, .L8005E780
    /* 4EF6C 8005E76C 00000000 */   nop
    /* 4EF70 8005E770 1A00A287 */  lh         $v0, 0x1A($sp)
    /* 4EF74 8005E774 00000000 */  nop
    /* 4EF78 8005E778 17004314 */  bne        $v0, $v1, .L8005E7D8
    /* 4EF7C 8005E77C 2800A427 */   addiu     $a0, $sp, 0x28
  .L8005E780:
    /* 4EF80 8005E780 0A80023C */  lui        $v0, %hi(D_8009B4B0)
    /* 4EF84 8005E784 B0B44224 */  addiu      $v0, $v0, %lo(D_8009B4B0)
    /* 4EF88 8005E788 2800A2AF */  sw         $v0, 0x28($sp)
    /* 4EF8C 8005E78C 00141400 */  sll        $v0, $s4, 16
    /* 4EF90 8005E790 03140200 */  sra        $v0, $v0, 16
    /* 4EF94 8005E794 4400A38F */  lw         $v1, 0x44($sp)
    /* 4EF98 8005E798 2800A427 */  addiu      $a0, $sp, 0x28
    /* 4EF9C 8005E79C 4000A0AF */  sw         $zero, 0x40($sp)
    /* 4EFA0 8005E7A0 02006324 */  addiu      $v1, $v1, 0x2
    /* 4EFA4 8005E7A4 4400A3AF */  sw         $v1, 0x44($sp)
    /* 4EFA8 8005E7A8 0A80013C */  lui        $at, %hi(D_800A3270)
    /* 4EFAC 8005E7AC 21082200 */  addu       $at, $at, $v0
    /* 4EFB0 8005E7B0 70322390 */  lbu        $v1, %lo(D_800A3270)($at)
    /* 4EFB4 8005E7B4 0A80023C */  lui        $v0, %hi(D_8009B4BC)
    /* 4EFB8 8005E7B8 BCB44224 */  addiu      $v0, $v0, %lo(D_8009B4BC)
    /* 4EFBC 8005E7BC 3000B2AF */  sw         $s2, 0x30($sp)
    /* 4EFC0 8005E7C0 C0180300 */  sll        $v1, $v1, 3
    /* 4EFC4 8005E7C4 21186200 */  addu       $v1, $v1, $v0
    /* 4EFC8 8005E7C8 4BCD010C */  jal        func_8007352C
    /* 4EFCC 8005E7CC 2C00A3AF */   sw        $v1, 0x2C($sp)
    /* 4EFD0 8005E7D0 657A0108 */  j          .L8005E994
    /* 4EFD4 8005E7D4 21904000 */   addu      $s2, $v0, $zero
  .L8005E7D8:
    /* 4EFD8 8005E7D8 0A80023C */  lui        $v0, %hi(D_8009B4E4)
    /* 4EFDC 8005E7DC E4B44224 */  addiu      $v0, $v0, %lo(D_8009B4E4)
    /* 4EFE0 8005E7E0 2800A2AF */  sw         $v0, 0x28($sp)
    /* 4EFE4 8005E7E4 0A80023C */  lui        $v0, %hi(D_8009B514)
    /* 4EFE8 8005E7E8 14B54224 */  addiu      $v0, $v0, %lo(D_8009B514)
    /* 4EFEC 8005E7EC 4000A0AF */  sw         $zero, 0x40($sp)
    /* 4EFF0 8005E7F0 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 4EFF4 8005E7F4 4BCD010C */  jal        func_8007352C
    /* 4EFF8 8005E7F8 3000B2AF */   sw        $s2, 0x30($sp)
    /* 4EFFC 8005E7FC 21904000 */  addu       $s2, $v0, $zero
    /* 4F000 8005E800 21880000 */  addu       $s1, $zero, $zero
    /* 4F004 8005E804 00141100 */  sll        $v0, $s1, 16
  .L8005E808:
    /* 4F008 8005E808 031C0200 */  sra        $v1, $v0, 16
    /* 4F00C 8005E80C C0100300 */  sll        $v0, $v1, 3
    /* 4F010 8005E810 21104300 */  addu       $v0, $v0, $v1
    /* 4F014 8005E814 80100200 */  sll        $v0, $v0, 2
    /* 4F018 8005E818 23104300 */  subu       $v0, $v0, $v1
    /* 4F01C 8005E81C 40100200 */  sll        $v0, $v0, 1
    /* 4F020 8005E820 4000A2AF */  sw         $v0, 0x40($sp)
    /* 4F024 8005E824 21206002 */  addu       $a0, $s3, $zero
    /* 4F028 8005E828 40100300 */  sll        $v0, $v1, 1
    /* 4F02C 8005E82C 02006014 */  bnez       $v1, .L8005E838
    /* 4F030 8005E830 21105300 */   addu      $v0, $v0, $s3
    /* 4F034 8005E834 1A00A427 */  addiu      $a0, $sp, 0x1A
  .L8005E838:
    /* 4F038 8005E838 00004584 */  lh         $a1, 0x0($v0)
    /* 4F03C 8005E83C 00008284 */  lh         $v0, 0x0($a0)
    /* 4F040 8005E840 00000000 */  nop
    /* 4F044 8005E844 2A104500 */  slt        $v0, $v0, $a1
    /* 4F048 8005E848 05004010 */  beqz       $v0, .L8005E860
    /* 4F04C 8005E84C 00000000 */   nop
    /* 4F050 8005E850 0A80023C */  lui        $v0, %hi(D_8009B4FC)
    /* 4F054 8005E854 FCB44224 */  addiu      $v0, $v0, %lo(D_8009B4FC)
    /* 4F058 8005E858 277A0108 */  j          .L8005E89C
    /* 4F05C 8005E85C 2C00A2AF */   sw        $v0, 0x2C($sp)
  .L8005E860:
    /* 4F060 8005E860 02006014 */  bnez       $v1, .L8005E86C
    /* 4F064 8005E864 21106002 */   addu      $v0, $s3, $zero
    /* 4F068 8005E868 1A00A227 */  addiu      $v0, $sp, 0x1A
  .L8005E86C:
    /* 4F06C 8005E86C 00004284 */  lh         $v0, 0x0($v0)
    /* 4F070 8005E870 00000000 */  nop
    /* 4F074 8005E874 2A10A200 */  slt        $v0, $a1, $v0
    /* 4F078 8005E878 05004010 */  beqz       $v0, .L8005E890
    /* 4F07C 8005E87C 00000000 */   nop
    /* 4F080 8005E880 0A80023C */  lui        $v0, %hi(D_8009B504)
    /* 4F084 8005E884 04B54224 */  addiu      $v0, $v0, %lo(D_8009B504)
    /* 4F088 8005E888 277A0108 */  j          .L8005E89C
    /* 4F08C 8005E88C 2C00A2AF */   sw        $v0, 0x2C($sp)
  .L8005E890:
    /* 4F090 8005E890 0A80023C */  lui        $v0, %hi(D_8009B50C)
    /* 4F094 8005E894 0CB54224 */  addiu      $v0, $v0, %lo(D_8009B50C)
    /* 4F098 8005E898 2C00A2AF */  sw         $v0, 0x2C($sp)
  .L8005E89C:
    /* 4F09C 8005E89C 3000B2AF */  sw         $s2, 0x30($sp)
    /* 4F0A0 8005E8A0 4BCD010C */  jal        func_8007352C
    /* 4F0A4 8005E8A4 2800A427 */   addiu     $a0, $sp, 0x28
    /* 4F0A8 8005E8A8 21904000 */  addu       $s2, $v0, $zero
    /* 4F0AC 8005E8AC 01002226 */  addiu      $v0, $s1, 0x1
    /* 4F0B0 8005E8B0 21884000 */  addu       $s1, $v0, $zero
    /* 4F0B4 8005E8B4 00140200 */  sll        $v0, $v0, 16
    /* 4F0B8 8005E8B8 03140200 */  sra        $v0, $v0, 16
    /* 4F0BC 8005E8BC 02004228 */  slti       $v0, $v0, 0x2
    /* 4F0C0 8005E8C0 D1FF4014 */  bnez       $v0, .L8005E808
    /* 4F0C4 8005E8C4 00141100 */   sll       $v0, $s1, 16
    /* 4F0C8 8005E8C8 4400A28F */  lw         $v0, 0x44($sp)
    /* 4F0CC 8005E8CC 21880000 */  addu       $s1, $zero, $zero
    /* 4F0D0 8005E8D0 05004224 */  addiu      $v0, $v0, 0x5
    /* 4F0D4 8005E8D4 4400A2AF */  sw         $v0, 0x44($sp)
  .L8005E8D8:
    /* 4F0D8 8005E8D8 00241100 */  sll        $a0, $s1, 16
    /* 4F0DC 8005E8DC 0A80023C */  lui        $v0, %hi(D_8009B4F0)
    /* 4F0E0 8005E8E0 F0B44224 */  addiu      $v0, $v0, %lo(D_8009B4F0)
    /* 4F0E4 8005E8E4 2800A2AF */  sw         $v0, 0x28($sp)
    /* 4F0E8 8005E8E8 0A80023C */  lui        $v0, %hi(D_8009B51C)
    /* 4F0EC 8005E8EC 1CB54224 */  addiu      $v0, $v0, %lo(D_8009B51C)
    /* 4F0F0 8005E8F0 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 4F0F4 8005E8F4 C3130400 */  sra        $v0, $a0, 15
    /* 4F0F8 8005E8F8 21105300 */  addu       $v0, $v0, $s3
    /* 4F0FC 8005E8FC 00004284 */  lh         $v0, 0x0($v0)
    /* 4F100 8005E900 00000000 */  nop
    /* 4F104 8005E904 1C004018 */  blez       $v0, .L8005E978
    /* 4F108 8005E908 21800000 */   addu      $s0, $zero, $zero
  .L8005E90C:
    /* 4F10C 8005E90C 04008010 */  beqz       $a0, .L8005E920
    /* 4F110 8005E910 00141000 */   sll       $v0, $s0, 16
    /* 4F114 8005E914 03130200 */  sra        $v0, $v0, 12
    /* 4F118 8005E918 4E7A0108 */  j          .L8005E938
    /* 4F11C 8005E91C 79014224 */   addiu     $v0, $v0, 0x179
  .L8005E920:
    /* 4F120 8005E920 001C1000 */  sll        $v1, $s0, 16
    /* 4F124 8005E924 031C0300 */  sra        $v1, $v1, 16
    /* 4F128 8005E928 01000224 */  addiu      $v0, $zero, 0x1
    /* 4F12C 8005E92C 23104300 */  subu       $v0, $v0, $v1
    /* 4F130 8005E930 00110200 */  sll        $v0, $v0, 4
    /* 4F134 8005E934 E2004224 */  addiu      $v0, $v0, 0xE2
  .L8005E938:
    /* 4F138 8005E938 4000A2AF */  sw         $v0, 0x40($sp)
    /* 4F13C 8005E93C 2800A427 */  addiu      $a0, $sp, 0x28
    /* 4F140 8005E940 4BCD010C */  jal        func_8007352C
    /* 4F144 8005E944 3000B2AF */   sw        $s2, 0x30($sp)
    /* 4F148 8005E948 21904000 */  addu       $s2, $v0, $zero
    /* 4F14C 8005E94C 01000226 */  addiu      $v0, $s0, 0x1
    /* 4F150 8005E950 21804000 */  addu       $s0, $v0, $zero
    /* 4F154 8005E954 00241100 */  sll        $a0, $s1, 16
    /* 4F158 8005E958 C31B0400 */  sra        $v1, $a0, 15
    /* 4F15C 8005E95C 21187300 */  addu       $v1, $v1, $s3
    /* 4F160 8005E960 00140200 */  sll        $v0, $v0, 16
    /* 4F164 8005E964 00006384 */  lh         $v1, 0x0($v1)
    /* 4F168 8005E968 03140200 */  sra        $v0, $v0, 16
    /* 4F16C 8005E96C 2A104300 */  slt        $v0, $v0, $v1
    /* 4F170 8005E970 E6FF4014 */  bnez       $v0, .L8005E90C
    /* 4F174 8005E974 00000000 */   nop
  .L8005E978:
    /* 4F178 8005E978 01002226 */  addiu      $v0, $s1, 0x1
    /* 4F17C 8005E97C 21884000 */  addu       $s1, $v0, $zero
    /* 4F180 8005E980 00140200 */  sll        $v0, $v0, 16
    /* 4F184 8005E984 03140200 */  sra        $v0, $v0, 16
    /* 4F188 8005E988 02004228 */  slti       $v0, $v0, 0x2
    /* 4F18C 8005E98C D2FF4014 */  bnez       $v0, .L8005E8D8
    /* 4F190 8005E990 00000000 */   nop
  .L8005E994:
    /* 4F194 8005E994 01008326 */  addiu      $v1, $s4, 0x1
    /* 4F198 8005E998 21A06000 */  addu       $s4, $v1, $zero
    /* 4F19C 8005E99C 0A80053C */  lui        $a1, %hi(D_8009BD38)
    /* 4F1A0 8005E9A0 38BDA524 */  addiu      $a1, $a1, %lo(D_8009BD38)
    /* 4F1A4 8005E9A4 001C0300 */  sll        $v1, $v1, 16
    /* 4F1A8 8005E9A8 0000A28C */  lw         $v0, 0x0($a1)
    /* 4F1AC 8005E9AC 031C0300 */  sra        $v1, $v1, 16
    /* 4F1B0 8005E9B0 82120200 */  srl        $v0, $v0, 10
    /* 4F1B4 8005E9B4 03004230 */  andi       $v0, $v0, 0x3
    /* 4F1B8 8005E9B8 03004224 */  addiu      $v0, $v0, 0x3
    /* 4F1BC 8005E9BC 2A186200 */  slt        $v1, $v1, $v0
    /* 4F1C0 8005E9C0 48FF6014 */  bnez       $v1, .L8005E6E4
    /* 4F1C4 8005E9C4 00141400 */   sll       $v0, $s4, 16
  .L8005E9C8:
    /* 4F1C8 8005E9C8 0A80023C */  lui        $v0, %hi(D_8009BD38)
    /* 4F1CC 8005E9CC 38BD428C */  lw         $v0, %lo(D_8009BD38)($v0)
    /* 4F1D0 8005E9D0 0A80033C */  lui        $v1, %hi(D_8009B4E4)
    /* 4F1D4 8005E9D4 E4B46324 */  addiu      $v1, $v1, %lo(D_8009B4E4)
    /* 4F1D8 8005E9D8 2800A3AF */  sw         $v1, 0x28($sp)
    /* 4F1DC 8005E9DC 000C4330 */  andi       $v1, $v0, 0xC00
    /* 4F1E0 8005E9E0 00080224 */  addiu      $v0, $zero, 0x800
    /* 4F1E4 8005E9E4 03006214 */  bne        $v1, $v0, .L8005E9F4
    /* 4F1E8 8005E9E8 4000A0AF */   sw        $zero, 0x40($sp)
    /* 4F1EC 8005E9EC 817A0108 */  j          .L8005EA04
    /* 4F1F0 8005E9F0 C6001024 */   addiu     $s0, $zero, 0xC6
  .L8005E9F4:
    /* 4F1F4 8005E9F4 00040224 */  addiu      $v0, $zero, 0x400
    /* 4F1F8 8005E9F8 02006214 */  bne        $v1, $v0, .L8005EA04
    /* 4F1FC 8005E9FC BE001024 */   addiu     $s0, $zero, 0xBE
    /* 4F200 8005EA00 C2001024 */  addiu      $s0, $zero, 0xC2
  .L8005EA04:
    /* 4F204 8005EA04 2800A427 */  addiu      $a0, $sp, 0x28
    /* 4F208 8005EA08 03000226 */  addiu      $v0, $s0, 0x3
    /* 4F20C 8005EA0C 4400A2AF */  sw         $v0, 0x44($sp)
    /* 4F210 8005EA10 0A80023C */  lui        $v0, %hi(D_8009B514)
    /* 4F214 8005EA14 14B54224 */  addiu      $v0, $v0, %lo(D_8009B514)
    /* 4F218 8005EA18 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 4F21C 8005EA1C 4BCD010C */  jal        func_8007352C
    /* 4F220 8005EA20 3000B2AF */   sw        $s2, 0x30($sp)
    /* 4F224 8005EA24 21904000 */  addu       $s2, $v0, $zero
    /* 4F228 8005EA28 0A80023C */  lui        $v0, %hi(D_8009BD38)
    /* 4F22C 8005EA2C 38BD428C */  lw         $v0, %lo(D_8009BD38)($v0)
    /* 4F230 8005EA30 21A00000 */  addu       $s4, $zero, $zero
    /* 4F234 8005EA34 82120200 */  srl        $v0, $v0, 10
    /* 4F238 8005EA38 03004230 */  andi       $v0, $v0, 0x3
    /* 4F23C 8005EA3C 03004224 */  addiu      $v0, $v0, 0x3
    /* 4F240 8005EA40 1F004010 */  beqz       $v0, .L8005EAC0
    /* 4F244 8005EA44 1800A0AF */   sw        $zero, 0x18($sp)
    /* 4F248 8005EA48 03000524 */  addiu      $a1, $zero, 0x3
    /* 4F24C 8005EA4C 21304000 */  addu       $a2, $v0, $zero
    /* 4F250 8005EA50 00141400 */  sll        $v0, $s4, 16
  .L8005EA54:
    /* 4F254 8005EA54 6000A98F */  lw         $t1, 0x60($sp)
    /* 4F258 8005EA58 83230200 */  sra        $a0, $v0, 14
    /* 4F25C 8005EA5C 06108900 */  srlv       $v0, $t1, $a0
    /* 4F260 8005EA60 03004330 */  andi       $v1, $v0, 0x3
    /* 4F264 8005EA64 05006510 */  beq        $v1, $a1, .L8005EA7C
    /* 4F268 8005EA68 00000000 */   nop
    /* 4F26C 8005EA6C 1800A297 */  lhu        $v0, 0x18($sp)
    /* 4F270 8005EA70 00000000 */  nop
    /* 4F274 8005EA74 21104300 */  addu       $v0, $v0, $v1
    /* 4F278 8005EA78 1800A2A7 */  sh         $v0, 0x18($sp)
  .L8005EA7C:
    /* 4F27C 8005EA7C 6000A88F */  lw         $t0, 0x60($sp)
    /* 4F280 8005EA80 02008224 */  addiu      $v0, $a0, 0x2
    /* 4F284 8005EA84 06104800 */  srlv       $v0, $t0, $v0
    /* 4F288 8005EA88 03004330 */  andi       $v1, $v0, 0x3
    /* 4F28C 8005EA8C 06006510 */  beq        $v1, $a1, .L8005EAA8
    /* 4F290 8005EA90 01008226 */   addiu     $v0, $s4, 0x1
    /* 4F294 8005EA94 1A00A297 */  lhu        $v0, 0x1A($sp)
    /* 4F298 8005EA98 00000000 */  nop
    /* 4F29C 8005EA9C 21104300 */  addu       $v0, $v0, $v1
    /* 4F2A0 8005EAA0 1A00A2A7 */  sh         $v0, 0x1A($sp)
    /* 4F2A4 8005EAA4 01008226 */  addiu      $v0, $s4, 0x1
  .L8005EAA8:
    /* 4F2A8 8005EAA8 21A04000 */  addu       $s4, $v0, $zero
    /* 4F2AC 8005EAAC 00140200 */  sll        $v0, $v0, 16
    /* 4F2B0 8005EAB0 03140200 */  sra        $v0, $v0, 16
    /* 4F2B4 8005EAB4 2A104600 */  slt        $v0, $v0, $a2
    /* 4F2B8 8005EAB8 E6FF4014 */  bnez       $v0, .L8005EA54
    /* 4F2BC 8005EABC 00141400 */   sll       $v0, $s4, 16
  .L8005EAC0:
    /* 4F2C0 8005EAC0 21880000 */  addu       $s1, $zero, $zero
    /* 4F2C4 8005EAC4 1800B327 */  addiu      $s3, $sp, 0x18
    /* 4F2C8 8005EAC8 21A00002 */  addu       $s4, $s0, $zero
    /* 4F2CC 8005EACC 00241100 */  sll        $a0, $s1, 16
  .L8005EAD0:
    /* 4F2D0 8005EAD0 0A80023C */  lui        $v0, %hi(D_8009B4F0)
    /* 4F2D4 8005EAD4 F0B44224 */  addiu      $v0, $v0, %lo(D_8009B4F0)
    /* 4F2D8 8005EAD8 2800A2AF */  sw         $v0, 0x28($sp)
    /* 4F2DC 8005EADC 0A80023C */  lui        $v0, %hi(D_8009B51C)
    /* 4F2E0 8005EAE0 1CB54224 */  addiu      $v0, $v0, %lo(D_8009B51C)
    /* 4F2E4 8005EAE4 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 4F2E8 8005EAE8 C3130400 */  sra        $v0, $a0, 15
    /* 4F2EC 8005EAEC 21105300 */  addu       $v0, $v0, $s3
    /* 4F2F0 8005EAF0 00004284 */  lh         $v0, 0x0($v0)
    /* 4F2F4 8005EAF4 00000000 */  nop
    /* 4F2F8 8005EAF8 26004018 */  blez       $v0, .L8005EB94
    /* 4F2FC 8005EAFC 21800000 */   addu      $s0, $zero, $zero
  .L8005EB00:
    /* 4F300 8005EB00 07008010 */  beqz       $a0, .L8005EB20
    /* 4F304 8005EB04 001C1000 */   sll       $v1, $s0, 16
    /* 4F308 8005EB08 431C0300 */  sra        $v1, $v1, 17
    /* 4F30C 8005EB0C 80100300 */  sll        $v0, $v1, 2
    /* 4F310 8005EB10 21104300 */  addu       $v0, $v0, $v1
    /* 4F314 8005EB14 80100200 */  sll        $v0, $v0, 2
    /* 4F318 8005EB18 CF7A0108 */  j          .L8005EB3C
    /* 4F31C 8005EB1C 81014224 */   addiu     $v0, $v0, 0x181
  .L8005EB20:
    /* 4F320 8005EB20 00141000 */  sll        $v0, $s0, 16
    /* 4F324 8005EB24 43140200 */  sra        $v0, $v0, 17
    /* 4F328 8005EB28 80180200 */  sll        $v1, $v0, 2
    /* 4F32C 8005EB2C 21186200 */  addu       $v1, $v1, $v0
    /* 4F330 8005EB30 80180300 */  sll        $v1, $v1, 2
    /* 4F334 8005EB34 F2000224 */  addiu      $v0, $zero, 0xF2
    /* 4F338 8005EB38 23104300 */  subu       $v0, $v0, $v1
  .L8005EB3C:
    /* 4F33C 8005EB3C 4000A2AF */  sw         $v0, 0x40($sp)
    /* 4F340 8005EB40 2800A427 */  addiu      $a0, $sp, 0x28
    /* 4F344 8005EB44 01000332 */  andi       $v1, $s0, 0x1
    /* 4F348 8005EB48 40100300 */  sll        $v0, $v1, 1
    /* 4F34C 8005EB4C 21104300 */  addu       $v0, $v0, $v1
    /* 4F350 8005EB50 80100200 */  sll        $v0, $v0, 2
    /* 4F354 8005EB54 21108202 */  addu       $v0, $s4, $v0
    /* 4F358 8005EB58 4400A2AF */  sw         $v0, 0x44($sp)
    /* 4F35C 8005EB5C 4BCD010C */  jal        func_8007352C
    /* 4F360 8005EB60 3000B2AF */   sw        $s2, 0x30($sp)
    /* 4F364 8005EB64 21904000 */  addu       $s2, $v0, $zero
    /* 4F368 8005EB68 01000226 */  addiu      $v0, $s0, 0x1
    /* 4F36C 8005EB6C 21804000 */  addu       $s0, $v0, $zero
    /* 4F370 8005EB70 00241100 */  sll        $a0, $s1, 16
    /* 4F374 8005EB74 C31B0400 */  sra        $v1, $a0, 15
    /* 4F378 8005EB78 21187300 */  addu       $v1, $v1, $s3
    /* 4F37C 8005EB7C 00140200 */  sll        $v0, $v0, 16
    /* 4F380 8005EB80 00006384 */  lh         $v1, 0x0($v1)
    /* 4F384 8005EB84 03140200 */  sra        $v0, $v0, 16
    /* 4F388 8005EB88 2A104300 */  slt        $v0, $v0, $v1
    /* 4F38C 8005EB8C DCFF4014 */  bnez       $v0, .L8005EB00
    /* 4F390 8005EB90 00000000 */   nop
  .L8005EB94:
    /* 4F394 8005EB94 01002226 */  addiu      $v0, $s1, 0x1
    /* 4F398 8005EB98 21884000 */  addu       $s1, $v0, $zero
    /* 4F39C 8005EB9C 00140200 */  sll        $v0, $v0, 16
    /* 4F3A0 8005EBA0 03140200 */  sra        $v0, $v0, 16
    /* 4F3A4 8005EBA4 02004228 */  slti       $v0, $v0, 0x2
    /* 4F3A8 8005EBA8 C9FF4014 */  bnez       $v0, .L8005EAD0
    /* 4F3AC 8005EBAC 00241100 */   sll       $a0, $s1, 16
    /* 4F3B0 8005EBB0 0A80043C */  lui        $a0, %hi(D_8009B524)
    /* 4F3B4 8005EBB4 24B58424 */  addiu      $a0, $a0, %lo(D_8009B524)
    /* 4F3B8 8005EBB8 20B9010C */  jal        func_8006E480
    /* 4F3BC 8005EBBC 21280000 */   addu      $a1, $zero, $zero
    /* 4F3C0 8005EBC0 01000524 */  addiu      $a1, $zero, 0x1
    /* 4F3C4 8005EBC4 21300000 */  addu       $a2, $zero, $zero
    /* 4F3C8 8005EBC8 8000A48F */  lw         $a0, 0x80($sp)
    /* 4F3CC 8005EBCC 21384000 */  addu       $a3, $v0, $zero
    /* 4F3D0 8005EBD0 92F0010C */  jal        initTexPage
    /* 4F3D4 8005EBD4 1000A0AF */   sw        $zero, 0x10($sp)
    /* 4F3D8 8005EBD8 8000A58F */  lw         $a1, 0x80($sp)
    /* 4F3DC 8005EBDC 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 4F3E0 8005EBE0 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 4F3E4 8005EBE4 7000A88F */  lw         $t0, 0x70($sp)
    /* 4F3E8 8005EBE8 2148A000 */  addu       $t1, $a1, $zero
    /* 4F3EC 8005EBEC 0C002925 */  addiu      $t1, $t1, 0xC
    /* 4F3F0 8005EBF0 80200800 */  sll        $a0, $t0, 2
    /* 4F3F4 8005EBF4 21204400 */  addu       $a0, $v0, $a0
    /* 4F3F8 8005EBF8 2DEA010C */  jal        ot_Link
    /* 4F3FC 8005EBFC 8000A9AF */   sw        $t1, 0x80($sp)
    /* 4F400 8005EC00 0A80023C */  lui        $v0, %hi(D_8009BD38)
    /* 4F404 8005EC04 38BD428C */  lw         $v0, %lo(D_8009BD38)($v0)
    /* 4F408 8005EC08 7000A98F */  lw         $t1, 0x70($sp)
    /* 4F40C 8005EC0C 21A00000 */  addu       $s4, $zero, $zero
    /* 4F410 8005EC10 2200A0A7 */  sh         $zero, 0x22($sp)
    /* 4F414 8005EC14 2000A0A7 */  sh         $zero, 0x20($sp)
    /* 4F418 8005EC18 82120200 */  srl        $v0, $v0, 10
    /* 4F41C 8005EC1C 03004230 */  andi       $v0, $v0, 0x3
    /* 4F420 8005EC20 03004224 */  addiu      $v0, $v0, 0x3
    /* 4F424 8005EC24 C0004010 */  beqz       $v0, .L8005EF28
    /* 4F428 8005EC28 3C00A9AF */   sw        $t1, 0x3C($sp)
    /* 4F42C 8005EC2C 2800B527 */  addiu      $s5, $sp, 0x28
  .L8005EC30:
    /* 4F430 8005EC30 21880000 */  addu       $s1, $zero, $zero
    /* 4F434 8005EC34 00141400 */  sll        $v0, $s4, 16
    /* 4F438 8005EC38 03140200 */  sra        $v0, $v0, 16
    /* 4F43C 8005EC3C 21804000 */  addu       $s0, $v0, $zero
    /* 4F440 8005EC40 40981000 */  sll        $s3, $s0, 1
    /* 4F444 8005EC44 21107002 */  addu       $v0, $s3, $s0
    /* 4F448 8005EC48 C0100200 */  sll        $v0, $v0, 3
    /* 4F44C 8005EC4C F8FF5E24 */  addiu      $fp, $v0, -0x8
    /* 4F450 8005EC50 03005724 */  addiu      $s7, $v0, 0x3
    /* 4F454 8005EC54 0A80023C */  lui        $v0, %hi(D_8009ADB4)
    /* 4F458 8005EC58 B4AD4224 */  addiu      $v0, $v0, %lo(D_8009ADB4)
    /* 4F45C 8005EC5C 6000A88F */  lw         $t0, 0x60($sp)
    /* 4F460 8005EC60 80181000 */  sll        $v1, $s0, 2
    /* 4F464 8005EC64 2800A2AF */  sw         $v0, 0x28($sp)
    /* 4F468 8005EC68 3800A0AF */  sw         $zero, 0x38($sp)
    /* 4F46C 8005EC6C 06106800 */  srlv       $v0, $t0, $v1
    /* 4F470 8005EC70 03004230 */  andi       $v0, $v0, 0x3
    /* 4F474 8005EC74 02006324 */  addiu      $v1, $v1, 0x2
    /* 4F478 8005EC78 06186800 */  srlv       $v1, $t0, $v1
    /* 4F47C 8005EC7C 03006330 */  andi       $v1, $v1, 0x3
    /* 4F480 8005EC80 1800A2A7 */  sh         $v0, 0x18($sp)
    /* 4F484 8005EC84 40000224 */  addiu      $v0, $zero, 0x40
    /* 4F488 8005EC88 1A00A3A7 */  sh         $v1, 0x1A($sp)
    /* 4F48C 8005EC8C 5300A2A3 */  sb         $v0, 0x53($sp)
    /* 4F490 8005EC90 5200A2A3 */  sb         $v0, 0x52($sp)
    /* 4F494 8005EC94 5100A2A3 */  sb         $v0, 0x51($sp)
    /* 4F498 8005EC98 1800A227 */  addiu      $v0, $sp, 0x18
  .L8005EC9C:
    /* 4F49C 8005EC9C 21204000 */  addu       $a0, $v0, $zero
    /* 4F4A0 8005ECA0 001C1100 */  sll        $v1, $s1, 16
    /* 4F4A4 8005ECA4 031C0300 */  sra        $v1, $v1, 16
    /* 4F4A8 8005ECA8 40100300 */  sll        $v0, $v1, 1
    /* 4F4AC 8005ECAC 02006014 */  bnez       $v1, .L8005ECB8
    /* 4F4B0 8005ECB0 21284400 */   addu      $a1, $v0, $a0
    /* 4F4B4 8005ECB4 1A00A427 */  addiu      $a0, $sp, 0x1A
  .L8005ECB8:
    /* 4F4B8 8005ECB8 0000A384 */  lh         $v1, 0x0($a1)
    /* 4F4BC 8005ECBC 00008284 */  lh         $v0, 0x0($a0)
    /* 4F4C0 8005ECC0 00000000 */  nop
    /* 4F4C4 8005ECC4 2A104300 */  slt        $v0, $v0, $v1
    /* 4F4C8 8005ECC8 05004014 */  bnez       $v0, .L8005ECE0
    /* 4F4CC 8005ECCC 03000924 */   addiu     $t1, $zero, 0x3
    /* 4F4D0 8005ECD0 0A006910 */  beq        $v1, $t1, .L8005ECFC
    /* 4F4D4 8005ECD4 01000224 */   addiu     $v0, $zero, 0x1
    /* 4F4D8 8005ECD8 407B0108 */  j          .L8005ED00
    /* 4F4DC 8005ECDC 5000A2A3 */   sb        $v0, 0x50($sp)
  .L8005ECE0:
    /* 4F4E0 8005ECE0 03000824 */  addiu      $t0, $zero, 0x3
    /* 4F4E4 8005ECE4 05006810 */  beq        $v1, $t0, .L8005ECFC
    /* 4F4E8 8005ECE8 00000000 */   nop
    /* 4F4EC 8005ECEC 0800A294 */  lhu        $v0, 0x8($a1)
    /* 4F4F0 8005ECF0 00000000 */  nop
    /* 4F4F4 8005ECF4 01004224 */  addiu      $v0, $v0, 0x1
    /* 4F4F8 8005ECF8 0800A2A4 */  sh         $v0, 0x8($a1)
  .L8005ECFC:
    /* 4F4FC 8005ECFC 5000A0A3 */  sb         $zero, 0x50($sp)
  .L8005ED00:
    /* 4F500 8005ED00 00141100 */  sll        $v0, $s1, 16
    /* 4F504 8005ED04 03140200 */  sra        $v0, $v0, 16
    /* 4F508 8005ED08 80180200 */  sll        $v1, $v0, 2
    /* 4F50C 8005ED0C 21286200 */  addu       $a1, $v1, $v0
    /* 4F510 8005ED10 40100500 */  sll        $v0, $a1, 1
    /* 4F514 8005ED14 21106202 */  addu       $v0, $s3, $v0
    /* 4F518 8005ED18 0A80013C */  lui        $at, %hi(D_8009BD24)
    /* 4F51C 8005ED1C 21082200 */  addu       $at, $at, $v0
    /* 4F520 8005ED20 24BD2490 */  lbu        $a0, %lo(D_8009BD24)($at)
    /* 4F524 8005ED24 00000000 */  nop
    /* 4F528 8005ED28 0C008228 */  slti       $v0, $a0, 0xC
    /* 4F52C 8005ED2C 02004014 */  bnez       $v0, .L8005ED38
    /* 4F530 8005ED30 00000000 */   nop
    /* 4F534 8005ED34 FEFF8424 */  addiu      $a0, $a0, -0x2
  .L8005ED38:
    /* 4F538 8005ED38 00240400 */  sll        $a0, $a0, 16
    /* 4F53C 8005ED3C 03240400 */  sra        $a0, $a0, 16
    /* 4F540 8005ED40 40100400 */  sll        $v0, $a0, 1
    /* 4F544 8005ED44 21104400 */  addu       $v0, $v0, $a0
    /* 4F548 8005ED48 C0100200 */  sll        $v0, $v0, 3
    /* 4F54C 8005ED4C 0A80033C */  lui        $v1, %hi(UesrWorkDef)
    /* 4F550 8005ED50 D8AD6324 */  addiu      $v1, $v1, %lo(UesrWorkDef)
    /* 4F554 8005ED54 21104300 */  addu       $v0, $v0, $v1
    /* 4F558 8005ED58 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 4F55C 8005ED5C 80110500 */  sll        $v0, $a1, 6
    /* 4F560 8005ED60 0A80013C */  lui        $at, %hi(D_8009B58C)
    /* 4F564 8005ED64 21082400 */  addu       $at, $at, $a0
    /* 4F568 8005ED68 8CB52390 */  lbu        $v1, %lo(D_8009B58C)($at)
    /* 4F56C 8005ED6C 0A80043C */  lui        $a0, %hi(D_8009BD38)
    /* 4F570 8005ED70 38BD848C */  lw         $a0, %lo(D_8009BD38)($a0)
    /* 4F574 8005ED74 21104300 */  addu       $v0, $v0, $v1
    /* 4F578 8005ED78 000C8430 */  andi       $a0, $a0, 0xC00
    /* 4F57C 8005ED7C 4000A2AF */  sw         $v0, 0x40($sp)
    /* 4F580 8005ED80 00080224 */  addiu      $v0, $zero, 0x800
    /* 4F584 8005ED84 03008214 */  bne        $a0, $v0, .L8005ED94
    /* 4F588 8005ED88 00040224 */   addiu     $v0, $zero, 0x400
    /* 4F58C 8005ED8C 6D7B0108 */  j          .L8005EDB4
    /* 4F590 8005ED90 4400BEAF */   sw        $fp, 0x44($sp)
  .L8005ED94:
    /* 4F594 8005ED94 03008214 */  bne        $a0, $v0, .L8005EDA4
    /* 4F598 8005ED98 00111000 */   sll       $v0, $s0, 4
    /* 4F59C 8005ED9C 6D7B0108 */  j          .L8005EDB4
    /* 4F5A0 8005EDA0 4400B7AF */   sw        $s7, 0x44($sp)
  .L8005EDA4:
    /* 4F5A4 8005EDA4 21105000 */  addu       $v0, $v0, $s0
    /* 4F5A8 8005EDA8 40100200 */  sll        $v0, $v0, 1
    /* 4F5AC 8005EDAC 03004224 */  addiu      $v0, $v0, 0x3
    /* 4F5B0 8005EDB0 4400A2AF */  sw         $v0, 0x44($sp)
  .L8005EDB4:
    /* 4F5B4 8005EDB4 2120A002 */  addu       $a0, $s5, $zero
    /* 4F5B8 8005EDB8 4BCD010C */  jal        func_8007352C
    /* 4F5BC 8005EDBC 3000B2AF */   sw        $s2, 0x30($sp)
    /* 4F5C0 8005EDC0 21904000 */  addu       $s2, $v0, $zero
    /* 4F5C4 8005EDC4 001C1100 */  sll        $v1, $s1, 16
    /* 4F5C8 8005EDC8 031C0300 */  sra        $v1, $v1, 16
    /* 4F5CC 8005EDCC 80100300 */  sll        $v0, $v1, 2
    /* 4F5D0 8005EDD0 21104300 */  addu       $v0, $v0, $v1
    /* 4F5D4 8005EDD4 40100200 */  sll        $v0, $v0, 1
    /* 4F5D8 8005EDD8 21106202 */  addu       $v0, $s3, $v0
    /* 4F5DC 8005EDDC 0A80013C */  lui        $at, %hi(D_8009BD24)
    /* 4F5E0 8005EDE0 21082200 */  addu       $at, $at, $v0
    /* 4F5E4 8005EDE4 24BD2390 */  lbu        $v1, %lo(D_8009BD24)($at)
    /* 4F5E8 8005EDE8 08000224 */  addiu      $v0, $zero, 0x8
    /* 4F5EC 8005EDEC 09006214 */  bne        $v1, $v0, .L8005EE14
    /* 4F5F0 8005EDF0 01002226 */   addiu     $v0, $s1, 0x1
    /* 4F5F4 8005EDF4 2120A002 */  addu       $a0, $s5, $zero
    /* 4F5F8 8005EDF8 0A80023C */  lui        $v0, %hi(D_8009ADC0)
    /* 4F5FC 8005EDFC C0AD4224 */  addiu      $v0, $v0, %lo(D_8009ADC0)
    /* 4F600 8005EE00 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 4F604 8005EE04 4BCD010C */  jal        func_8007352C
    /* 4F608 8005EE08 3000B2AF */   sw        $s2, 0x30($sp)
    /* 4F60C 8005EE0C 21904000 */  addu       $s2, $v0, $zero
    /* 4F610 8005EE10 01002226 */  addiu      $v0, $s1, 0x1
  .L8005EE14:
    /* 4F614 8005EE14 21884000 */  addu       $s1, $v0, $zero
    /* 4F618 8005EE18 00140200 */  sll        $v0, $v0, 16
    /* 4F61C 8005EE1C 03140200 */  sra        $v0, $v0, 16
    /* 4F620 8005EE20 02004228 */  slti       $v0, $v0, 0x2
    /* 4F624 8005EE24 9DFF4014 */  bnez       $v0, .L8005EC9C
    /* 4F628 8005EE28 1800A227 */   addiu     $v0, $sp, 0x18
    /* 4F62C 8005EE2C 1800A287 */  lh         $v0, 0x18($sp)
    /* 4F630 8005EE30 03000924 */  addiu      $t1, $zero, 0x3
    /* 4F634 8005EE34 30004914 */  bne        $v0, $t1, .L8005EEF8
    /* 4F638 8005EE38 5000A0A3 */   sb        $zero, 0x50($sp)
    /* 4F63C 8005EE3C 21880000 */  addu       $s1, $zero, $zero
    /* 4F640 8005EE40 0A80133C */  lui        $s3, %hi(D_8009B498)
    /* 4F644 8005EE44 98B47326 */  addiu      $s3, $s3, %lo(D_8009B498)
    /* 4F648 8005EE48 4400A38F */  lw         $v1, 0x44($sp)
    /* 4F64C 8005EE4C 00010224 */  addiu      $v0, $zero, 0x100
    /* 4F650 8005EE50 4800A2AF */  sw         $v0, 0x48($sp)
    /* 4F654 8005EE54 00040224 */  addiu      $v0, $zero, 0x400
    /* 4F658 8005EE58 4000A0AF */  sw         $zero, 0x40($sp)
    /* 4F65C 8005EE5C 3800A0AF */  sw         $zero, 0x38($sp)
    /* 4F660 8005EE60 4C00A2AF */  sw         $v0, 0x4C($sp)
    /* 4F664 8005EE64 4C006224 */  addiu      $v0, $v1, 0x4C
    /* 4F668 8005EE68 52006324 */  addiu      $v1, $v1, 0x52
    /* 4F66C 8005EE6C 4400A2AF */  sw         $v0, 0x44($sp)
    /* 4F670 8005EE70 4400A3AF */  sw         $v1, 0x44($sp)
    /* 4F674 8005EE74 2120A002 */  addu       $a0, $s5, $zero
  .L8005EE78:
    /* 4F678 8005EE78 21280000 */  addu       $a1, $zero, $zero
    /* 4F67C 8005EE7C 00841100 */  sll        $s0, $s1, 16
    /* 4F680 8005EE80 03841000 */  sra        $s0, $s0, 16
    /* 4F684 8005EE84 40101000 */  sll        $v0, $s0, 1
    /* 4F688 8005EE88 21105000 */  addu       $v0, $v0, $s0
    /* 4F68C 8005EE8C 80100200 */  sll        $v0, $v0, 2
    /* 4F690 8005EE90 0A80033C */  lui        $v1, %hi(D_8009B3B0)
    /* 4F694 8005EE94 B0B36324 */  addiu      $v1, $v1, %lo(D_8009B3B0)
    /* 4F698 8005EE98 21104300 */  addu       $v0, $v0, $v1
    /* 4F69C 8005EE9C 00811000 */  sll        $s0, $s0, 4
    /* 4F6A0 8005EEA0 7800A98F */  lw         $t1, 0x78($sp)
    /* 4F6A4 8005EEA4 0A80083C */  lui        $t0, %hi(D_8009B490)
    /* 4F6A8 8005EEA8 90B40825 */  addiu      $t0, $t0, %lo(D_8009B490)
    /* 4F6AC 8005EEAC 2800A2AF */  sw         $v0, 0x28($sp)
    /* 4F6B0 8005EEB0 21100802 */  addu       $v0, $s0, $t0
    /* 4F6B4 8005EEB4 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 4F6B8 8005EEB8 CACD010C */  jal        func_80073728
    /* 4F6BC 8005EEBC 3400A9AF */   sw        $t1, 0x34($sp)
    /* 4F6C0 8005EEC0 2120A002 */  addu       $a0, $s5, $zero
    /* 4F6C4 8005EEC4 21280000 */  addu       $a1, $zero, $zero
    /* 4F6C8 8005EEC8 21801302 */  addu       $s0, $s0, $s3
    /* 4F6CC 8005EECC 2C00B0AF */  sw         $s0, 0x2C($sp)
    /* 4F6D0 8005EED0 CACD010C */  jal        func_80073728
    /* 4F6D4 8005EED4 3400A2AF */   sw        $v0, 0x34($sp)
    /* 4F6D8 8005EED8 7800A2AF */  sw         $v0, 0x78($sp)
    /* 4F6DC 8005EEDC 01002226 */  addiu      $v0, $s1, 0x1
    /* 4F6E0 8005EEE0 21884000 */  addu       $s1, $v0, $zero
    /* 4F6E4 8005EEE4 00140200 */  sll        $v0, $v0, 16
    /* 4F6E8 8005EEE8 03140200 */  sra        $v0, $v0, 16
    /* 4F6EC 8005EEEC 02004228 */  slti       $v0, $v0, 0x2
    /* 4F6F0 8005EEF0 E1FF4014 */  bnez       $v0, .L8005EE78
    /* 4F6F4 8005EEF4 2120A002 */   addu      $a0, $s5, $zero
  .L8005EEF8:
    /* 4F6F8 8005EEF8 01008326 */  addiu      $v1, $s4, 0x1
    /* 4F6FC 8005EEFC 21A06000 */  addu       $s4, $v1, $zero
    /* 4F700 8005EF00 001C0300 */  sll        $v1, $v1, 16
    /* 4F704 8005EF04 0A80023C */  lui        $v0, %hi(D_8009BD38)
    /* 4F708 8005EF08 38BD428C */  lw         $v0, %lo(D_8009BD38)($v0)
    /* 4F70C 8005EF0C 031C0300 */  sra        $v1, $v1, 16
    /* 4F710 8005EF10 82120200 */  srl        $v0, $v0, 10
    /* 4F714 8005EF14 03004230 */  andi       $v0, $v0, 0x3
    /* 4F718 8005EF18 03004224 */  addiu      $v0, $v0, 0x3
    /* 4F71C 8005EF1C 2A186200 */  slt        $v1, $v1, $v0
    /* 4F720 8005EF20 43FF6014 */  bnez       $v1, .L8005EC30
    /* 4F724 8005EF24 00000000 */   nop
  .L8005EF28:
    /* 4F728 8005EF28 0A80023C */  lui        $v0, %hi(D_8009BD38)
    /* 4F72C 8005EF2C 38BD428C */  lw         $v0, %lo(D_8009BD38)($v0)
    /* 4F730 8005EF30 0A80033C */  lui        $v1, %hi(D_8009B398)
    /* 4F734 8005EF34 98B36324 */  addiu      $v1, $v1, %lo(D_8009B398)
    /* 4F738 8005EF38 2800A3AF */  sw         $v1, 0x28($sp)
    /* 4F73C 8005EF3C 000C4330 */  andi       $v1, $v0, 0xC00
    /* 4F740 8005EF40 00080224 */  addiu      $v0, $zero, 0x800
    /* 4F744 8005EF44 03006214 */  bne        $v1, $v0, .L8005EF54
    /* 4F748 8005EF48 3800A0AF */   sw        $zero, 0x38($sp)
    /* 4F74C 8005EF4C D97B0108 */  j          .L8005EF64
    /* 4F750 8005EF50 C9000224 */   addiu     $v0, $zero, 0xC9
  .L8005EF54:
    /* 4F754 8005EF54 00040224 */  addiu      $v0, $zero, 0x400
    /* 4F758 8005EF58 02006214 */  bne        $v1, $v0, .L8005EF64
    /* 4F75C 8005EF5C C1000224 */   addiu     $v0, $zero, 0xC1
    /* 4F760 8005EF60 C5000224 */  addiu      $v0, $zero, 0xC5
  .L8005EF64:
    /* 4F764 8005EF64 4400A2AF */  sw         $v0, 0x44($sp)
    /* 4F768 8005EF68 21880000 */  addu       $s1, $zero, $zero
    /* 4F76C 8005EF6C 001C1100 */  sll        $v1, $s1, 16
  .L8005EF70:
    /* 4F770 8005EF70 031C0300 */  sra        $v1, $v1, 16
    /* 4F774 8005EF74 C0100300 */  sll        $v0, $v1, 3
    /* 4F778 8005EF78 21104300 */  addu       $v0, $v0, $v1
    /* 4F77C 8005EF7C 80100200 */  sll        $v0, $v0, 2
    /* 4F780 8005EF80 23104300 */  subu       $v0, $v0, $v1
    /* 4F784 8005EF84 40280200 */  sll        $a1, $v0, 1
    /* 4F788 8005EF88 1301A224 */  addiu      $v0, $a1, 0x113
    /* 4F78C 8005EF8C 40180300 */  sll        $v1, $v1, 1
    /* 4F790 8005EF90 4000A2AF */  sw         $v0, 0x40($sp)
    /* 4F794 8005EF94 1800A227 */  addiu      $v0, $sp, 0x18
    /* 4F798 8005EF98 21206200 */  addu       $a0, $v1, $v0
    /* 4F79C 8005EF9C 08008384 */  lh         $v1, 0x8($a0)
    /* 4F7A0 8005EFA0 01000224 */  addiu      $v0, $zero, 0x1
    /* 4F7A4 8005EFA4 02006214 */  bne        $v1, $v0, .L8005EFB0
    /* 4F7A8 8005EFA8 1601A224 */   addiu     $v0, $a1, 0x116
    /* 4F7AC 8005EFAC 4000A2AF */  sw         $v0, 0x40($sp)
  .L8005EFB0:
    /* 4F7B0 8005EFB0 08008284 */  lh         $v0, 0x8($a0)
    /* 4F7B4 8005EFB4 2800A427 */  addiu      $a0, $sp, 0x28
    /* 4F7B8 8005EFB8 0A80033C */  lui        $v1, %hi(D_8009B400)
    /* 4F7BC 8005EFBC 00B46324 */  addiu      $v1, $v1, %lo(D_8009B400)
    /* 4F7C0 8005EFC0 C0100200 */  sll        $v0, $v0, 3
    /* 4F7C4 8005EFC4 21104300 */  addu       $v0, $v0, $v1
    /* 4F7C8 8005EFC8 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 4F7CC 8005EFCC 020040A4 */  sh         $zero, 0x2($v0)
    /* 4F7D0 8005EFD0 000040A4 */  sh         $zero, 0x0($v0)
    /* 4F7D4 8005EFD4 4BCD010C */  jal        func_8007352C
    /* 4F7D8 8005EFD8 3000B2AF */   sw        $s2, 0x30($sp)
    /* 4F7DC 8005EFDC 21904000 */  addu       $s2, $v0, $zero
    /* 4F7E0 8005EFE0 01002226 */  addiu      $v0, $s1, 0x1
    /* 4F7E4 8005EFE4 21884000 */  addu       $s1, $v0, $zero
    /* 4F7E8 8005EFE8 00140200 */  sll        $v0, $v0, 16
    /* 4F7EC 8005EFEC 03140200 */  sra        $v0, $v0, 16
    /* 4F7F0 8005EFF0 02004228 */  slti       $v0, $v0, 0x2
    /* 4F7F4 8005EFF4 DEFF4014 */  bnez       $v0, .L8005EF70
    /* 4F7F8 8005EFF8 001C1100 */   sll       $v1, $s1, 16
    /* 4F7FC 8005EFFC B4EA010C */  jal        initTile
    /* 4F800 8005F000 2120C002 */   addu      $a0, $s6, $zero
    /* 4F804 8005F004 2120C002 */  addu       $a0, $s6, $zero
    /* 4F808 8005F008 21280000 */  addu       $a1, $zero, $zero
    /* 4F80C 8005F00C FF001124 */  addiu      $s1, $zero, 0xFF
    /* 4F810 8005F010 10001024 */  addiu      $s0, $zero, 0x10
    /* 4F814 8005F014 08000224 */  addiu      $v0, $zero, 0x8
    /* 4F818 8005F018 3A001524 */  addiu      $s5, $zero, 0x3A
    /* 4F81C 8005F01C DC001424 */  addiu      $s4, $zero, 0xDC
    /* 4F820 8005F020 01001324 */  addiu      $s3, $zero, 0x1
    /* 4F824 8005F024 0400D1A2 */  sb         $s1, 0x4($s6)
    /* 4F828 8005F028 0500D0A2 */  sb         $s0, 0x5($s6)
    /* 4F82C 8005F02C 0600D0A2 */  sb         $s0, 0x6($s6)
    /* 4F830 8005F030 0800C2A6 */  sh         $v0, 0x8($s6)
    /* 4F834 8005F034 0A00D5A6 */  sh         $s5, 0xA($s6)
    /* 4F838 8005F038 0C00D4A6 */  sh         $s4, 0xC($s6)
    /* 4F83C 8005F03C 5AEA010C */  jal        gpu_SetSemiTransp
    /* 4F840 8005F040 0E00D3A6 */   sh        $s3, 0xE($s6)
    /* 4F844 8005F044 2128C002 */  addu       $a1, $s6, $zero
    /* 4F848 8005F048 1000D626 */  addiu      $s6, $s6, 0x10
    /* 4F84C 8005F04C 7000A88F */  lw         $t0, 0x70($sp)
    /* 4F850 8005F050 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4F854 8005F054 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4F858 8005F058 80900800 */  sll        $s2, $t0, 2
    /* 4F85C 8005F05C 2DEA010C */  jal        ot_Link
    /* 4F860 8005F060 21209200 */   addu      $a0, $a0, $s2
    /* 4F864 8005F064 B4EA010C */  jal        initTile
    /* 4F868 8005F068 2120C002 */   addu      $a0, $s6, $zero
    /* 4F86C 8005F06C 2120C002 */  addu       $a0, $s6, $zero
    /* 4F870 8005F070 21280000 */  addu       $a1, $zero, $zero
    /* 4F874 8005F074 9D010224 */  addiu      $v0, $zero, 0x19D
    /* 4F878 8005F078 0400D1A2 */  sb         $s1, 0x4($s6)
    /* 4F87C 8005F07C 0500D0A2 */  sb         $s0, 0x5($s6)
    /* 4F880 8005F080 0600D0A2 */  sb         $s0, 0x6($s6)
    /* 4F884 8005F084 0800C2A6 */  sh         $v0, 0x8($s6)
    /* 4F888 8005F088 0A00D5A6 */  sh         $s5, 0xA($s6)
    /* 4F88C 8005F08C 0C00D4A6 */  sh         $s4, 0xC($s6)
    /* 4F890 8005F090 5AEA010C */  jal        gpu_SetSemiTransp
    /* 4F894 8005F094 0E00D3A6 */   sh        $s3, 0xE($s6)
    /* 4F898 8005F098 2128C002 */  addu       $a1, $s6, $zero
    /* 4F89C 8005F09C 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4F8A0 8005F0A0 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4F8A4 8005F0A4 1000D626 */  addiu      $s6, $s6, 0x10
    /* 4F8A8 8005F0A8 2DEA010C */  jal        ot_Link
    /* 4F8AC 8005F0AC 21209200 */   addu      $a0, $a0, $s2
    /* 4F8B0 8005F0B0 B4EA010C */  jal        initTile
    /* 4F8B4 8005F0B4 2120C002 */   addu      $a0, $s6, $zero
    /* 4F8B8 8005F0B8 0400D1A2 */  sb         $s1, 0x4($s6)
    /* 4F8BC 8005F0BC 0500D0A2 */  sb         $s0, 0x5($s6)
    /* 4F8C0 8005F0C0 0600D0A2 */  sb         $s0, 0x6($s6)
    /* 4F8C4 8005F0C4 0A80023C */  lui        $v0, %hi(D_8009BD38)
    /* 4F8C8 8005F0C8 38BD428C */  lw         $v0, %lo(D_8009BD38)($v0)
    /* 4F8CC 8005F0CC 00000000 */  nop
    /* 4F8D0 8005F0D0 000C4330 */  andi       $v1, $v0, 0xC00
    /* 4F8D4 8005F0D4 00080224 */  addiu      $v0, $zero, 0x800
    /* 4F8D8 8005F0D8 04006214 */  bne        $v1, $v0, .L8005F0EC
    /* 4F8DC 8005F0DC 5E000224 */   addiu     $v0, $zero, 0x5E
    /* 4F8E0 8005F0E0 0800C2A6 */  sh         $v0, 0x8($s6)
    /* 4F8E4 8005F0E4 437C0108 */  j          .L8005F10C
    /* 4F8E8 8005F0E8 C1000224 */   addiu     $v0, $zero, 0xC1
  .L8005F0EC:
    /* 4F8EC 8005F0EC 00040224 */  addiu      $v0, $zero, 0x400
    /* 4F8F0 8005F0F0 04006214 */  bne        $v1, $v0, .L8005F104
    /* 4F8F4 8005F0F4 5E000224 */   addiu     $v0, $zero, 0x5E
    /* 4F8F8 8005F0F8 0800C2A6 */  sh         $v0, 0x8($s6)
    /* 4F8FC 8005F0FC 437C0108 */  j          .L8005F10C
    /* 4F900 8005F100 BD000224 */   addiu     $v0, $zero, 0xBD
  .L8005F104:
    /* 4F904 8005F104 0800C2A6 */  sh         $v0, 0x8($s6)
    /* 4F908 8005F108 B9000224 */  addiu      $v0, $zero, 0xB9
  .L8005F10C:
    /* 4F90C 8005F10C 0A00C2A6 */  sh         $v0, 0xA($s6)
    /* 4F910 8005F110 2120C002 */  addu       $a0, $s6, $zero
    /* 4F914 8005F114 21280000 */  addu       $a1, $zero, $zero
    /* 4F918 8005F118 C5010224 */  addiu      $v0, $zero, 0x1C5
    /* 4F91C 8005F11C 0C00C2A6 */  sh         $v0, 0xC($s6)
    /* 4F920 8005F120 01000224 */  addiu      $v0, $zero, 0x1
    /* 4F924 8005F124 5AEA010C */  jal        gpu_SetSemiTransp
    /* 4F928 8005F128 0E00C2A6 */   sh        $v0, 0xE($s6)
    /* 4F92C 8005F12C 2128C002 */  addu       $a1, $s6, $zero
    /* 4F930 8005F130 7000A98F */  lw         $t1, 0x70($sp)
    /* 4F934 8005F134 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4F938 8005F138 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4F93C 8005F13C 80800900 */  sll        $s0, $t1, 2
    /* 4F940 8005F140 2DEA010C */  jal        ot_Link
    /* 4F944 8005F144 21209000 */   addu      $a0, $a0, $s0
    /* 4F948 8005F148 0A80043C */  lui        $a0, %hi(D_8009ADB4)
    /* 4F94C 8005F14C B4AD8424 */  addiu      $a0, $a0, %lo(D_8009ADB4)
    /* 4F950 8005F150 20B9010C */  jal        func_8006E480
    /* 4F954 8005F154 21280000 */   addu      $a1, $zero, $zero
    /* 4F958 8005F158 01000524 */  addiu      $a1, $zero, 0x1
    /* 4F95C 8005F15C 21300000 */  addu       $a2, $zero, $zero
    /* 4F960 8005F160 8000A48F */  lw         $a0, 0x80($sp)
    /* 4F964 8005F164 21384000 */  addu       $a3, $v0, $zero
    /* 4F968 8005F168 92F0010C */  jal        initTexPage
    /* 4F96C 8005F16C 1000A0AF */   sw        $zero, 0x10($sp)
    /* 4F970 8005F170 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4F974 8005F174 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4F978 8005F178 8000A58F */  lw         $a1, 0x80($sp)
    /* 4F97C 8005F17C 2DEA010C */  jal        ot_Link
    /* 4F980 8005F180 21209000 */   addu      $a0, $a0, $s0
    /* 4F984 8005F184 8800A88F */  lw         $t0, 0x88($sp)
    /* 4F988 8005F188 6800A98F */  lw         $t1, 0x68($sp)
    /* 4F98C 8005F18C 00000000 */  nop
    /* 4F990 8005F190 23100901 */  subu       $v0, $t0, $t1
    /* 4F994 8005F194 B400BF8F */  lw         $ra, 0xB4($sp)
    /* 4F998 8005F198 B000BE8F */  lw         $fp, 0xB0($sp)
    /* 4F99C 8005F19C AC00B78F */  lw         $s7, 0xAC($sp)
    /* 4F9A0 8005F1A0 A800B68F */  lw         $s6, 0xA8($sp)
    /* 4F9A4 8005F1A4 A400B58F */  lw         $s5, 0xA4($sp)
    /* 4F9A8 8005F1A8 A000B48F */  lw         $s4, 0xA0($sp)
    /* 4F9AC 8005F1AC 9C00B38F */  lw         $s3, 0x9C($sp)
    /* 4F9B0 8005F1B0 9800B28F */  lw         $s2, 0x98($sp)
    /* 4F9B4 8005F1B4 9400B18F */  lw         $s1, 0x94($sp)
    /* 4F9B8 8005F1B8 9000B08F */  lw         $s0, 0x90($sp)
    /* 4F9BC 8005F1BC B800BD27 */  addiu      $sp, $sp, 0xB8
    /* 4F9C0 8005F1C0 0800E003 */  jr         $ra
    /* 4F9C4 8005F1C4 00000000 */   nop
endlabel func_8005E54C
