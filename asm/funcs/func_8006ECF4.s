glabel func_8006ECF4
    /* 5F4F4 8006ECF4 A0FFBD27 */  addiu      $sp, $sp, -0x60
    /* 5F4F8 8006ECF8 5000B2AF */  sw         $s2, 0x50($sp)
    /* 5F4FC 8006ECFC 21908000 */  addu       $s2, $a0, $zero
    /* 5F500 8006ED00 14000224 */  addiu      $v0, $zero, 0x14
    /* 5F504 8006ED04 2400A2AF */  sw         $v0, 0x24($sp)
    /* 5F508 8006ED08 00020224 */  addiu      $v0, $zero, 0x200
    /* 5F50C 8006ED0C 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5F510 8006ED10 00010224 */  addiu      $v0, $zero, 0x100
    /* 5F514 8006ED14 4C00B1AF */  sw         $s1, 0x4C($sp)
    /* 5F518 8006ED18 21880000 */  addu       $s1, $zero, $zero
    /* 5F51C 8006ED1C 5C00BFAF */  sw         $ra, 0x5C($sp)
    /* 5F520 8006ED20 5800B4AF */  sw         $s4, 0x58($sp)
    /* 5F524 8006ED24 5400B3AF */  sw         $s3, 0x54($sp)
    /* 5F528 8006ED28 4800B0AF */  sw         $s0, 0x48($sp)
    /* 5F52C 8006ED2C 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5F530 8006ED30 2C00A0AF */  sw         $zero, 0x2C($sp)
    /* 5F534 8006ED34 3400A2AF */  sw         $v0, 0x34($sp)
    /* 5F538 8006ED38 0000428E */  lw         $v0, 0x0($s2)
    /* 5F53C 8006ED3C E404838F */  lw         $v1, %gp_rel(D_800A35B0)($gp)
    /* 5F540 8006ED40 5400538C */  lw         $s3, 0x54($v0)
    /* 5F544 8006ED44 88048287 */  lh         $v0, %gp_rel(D_800A3554)($gp)
    /* 5F548 8006ED48 00000000 */  nop
    /* 5F54C 8006ED4C 01004224 */  addiu      $v0, $v0, 0x1
    /* 5F550 8006ED50 21186200 */  addu       $v1, $v1, $v0
    /* 5F554 8006ED54 AF006018 */  blez       $v1, .L8006F014
    /* 5F558 8006ED58 0C007026 */   addiu     $s0, $s3, 0xC
    /* 5F55C 8006ED5C 01001424 */  addiu      $s4, $zero, 0x1
    /* 5F560 8006ED60 00141100 */  sll        $v0, $s1, 16
  .L8006ED64:
    /* 5F564 8006ED64 C3130200 */  sra        $v0, $v0, 15
    /* 5F568 8006ED68 0A80013C */  lui        $at, %hi(D_800A3588)
    /* 5F56C 8006ED6C 21082200 */  addu       $at, $at, $v0
    /* 5F570 8006ED70 88352384 */  lh         $v1, %lo(D_800A3588)($at)
    /* 5F574 8006ED74 0A80013C */  lui        $at, %hi(D_800A358C)
    /* 5F578 8006ED78 21082200 */  addu       $at, $at, $v0
    /* 5F57C 8006ED7C 8C352484 */  lh         $a0, %lo(D_800A358C)($at)
    /* 5F580 8006ED80 40180300 */  sll        $v1, $v1, 1
    /* 5F584 8006ED84 40100400 */  sll        $v0, $a0, 1
    /* 5F588 8006ED88 21104400 */  addu       $v0, $v0, $a0
    /* 5F58C 8006ED8C 80100200 */  sll        $v0, $v0, 2
    /* 5F590 8006ED90 21186200 */  addu       $v1, $v1, $v0
    /* 5F594 8006ED94 0A80013C */  lui        $at, %hi(D_8009BC40)
    /* 5F598 8006ED98 21082300 */  addu       $at, $at, $v1
    /* 5F59C 8006ED9C 40BC2390 */  lbu        $v1, %lo(D_8009BC40)($at)
    /* 5F5A0 8006EDA0 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5F5A4 8006EDA4 21082300 */  addu       $at, $at, $v1
    /* 5F5A8 8006EDA8 7CBC2290 */  lbu        $v0, %lo(D_8009BC7C)($at)
    /* 5F5AC 8006EDAC 00000000 */  nop
    /* 5F5B0 8006EDB0 01004230 */  andi       $v0, $v0, 0x1
    /* 5F5B4 8006EDB4 12004010 */  beqz       $v0, .L8006EE00
    /* 5F5B8 8006EDB8 01000224 */   addiu     $v0, $zero, 0x1
    /* 5F5BC 8006EDBC F804828F */  lw         $v0, %gp_rel(D_800A35C4)($gp)
    /* 5F5C0 8006EDC0 2000A0AF */  sw         $zero, 0x20($sp)
    /* 5F5C4 8006EDC4 0800428C */  lw         $v0, 0x8($v0)
    /* 5F5C8 8006EDC8 00000000 */  nop
    /* 5F5CC 8006EDCC 04004230 */  andi       $v0, $v0, 0x4
    /* 5F5D0 8006EDD0 03004010 */  beqz       $v0, .L8006EDE0
    /* 5F5D4 8006EDD4 00000000 */   nop
    /* 5F5D8 8006EDD8 79BB0108 */  j          .L8006EDE4
    /* 5F5DC 8006EDDC 3800B4A3 */   sb        $s4, 0x38($sp)
  .L8006EDE0:
    /* 5F5E0 8006EDE0 3800A0A3 */  sb         $zero, 0x38($sp)
  .L8006EDE4:
    /* 5F5E4 8006EDE4 94000224 */  addiu      $v0, $zero, 0x94
    /* 5F5E8 8006EDE8 3900A2A3 */  sb         $v0, 0x39($sp)
    /* 5F5EC 8006EDEC 80000224 */  addiu      $v0, $zero, 0x80
    /* 5F5F0 8006EDF0 3A00A2A3 */  sb         $v0, 0x3A($sp)
    /* 5F5F4 8006EDF4 6E000224 */  addiu      $v0, $zero, 0x6E
    /* 5F5F8 8006EDF8 85BB0108 */  j          .L8006EE14
    /* 5F5FC 8006EDFC 3B00A2A3 */   sb        $v0, 0x3B($sp)
  .L8006EE00:
    /* 5F600 8006EE00 2000B4AF */  sw         $s4, 0x20($sp)
    /* 5F604 8006EE04 3800A2A3 */  sb         $v0, 0x38($sp)
    /* 5F608 8006EE08 3B00A0A3 */  sb         $zero, 0x3B($sp)
    /* 5F60C 8006EE0C 3A00A0A3 */  sb         $zero, 0x3A($sp)
    /* 5F610 8006EE10 3900A0A3 */  sb         $zero, 0x39($sp)
  .L8006EE14:
    /* 5F614 8006EE14 0F00622C */  sltiu      $v0, $v1, 0xF
    /* 5F618 8006EE18 3D004010 */  beqz       $v0, .L8006EF10
    /* 5F61C 8006EE1C 80100300 */   sll       $v0, $v1, 2
    /* 5F620 8006EE20 0180013C */  lui        $at, %hi(jtbl_800159D0)
    /* 5F624 8006EE24 21082200 */  addu       $at, $at, $v0
    /* 5F628 8006EE28 D059228C */  lw         $v0, %lo(jtbl_800159D0)($at)
    /* 5F62C 8006EE2C 00000000 */  nop
    /* 5F630 8006EE30 08004000 */  jr         $v0
    /* 5F634 8006EE34 00000000 */   nop
  jlabel .L8006EE38
    /* 5F638 8006EE38 DC04828F */  lw         $v0, %gp_rel(D_800A35A8)($gp)
    /* 5F63C 8006EE3C 00000000 */  nop
    /* 5F640 8006EE40 8400468C */  lw         $a2, 0x84($v0)
    /* 5F644 8006EE44 A6BB0108 */  j          .L8006EE98
    /* 5F648 8006EE48 08010226 */   addiu     $v0, $s0, 0x108
  jlabel .L8006EE4C
    /* 5F64C 8006EE4C DC04828F */  lw         $v0, %gp_rel(D_800A35A8)($gp)
    /* 5F650 8006EE50 00000000 */  nop
    /* 5F654 8006EE54 8800468C */  lw         $a2, 0x88($v0)
    /* 5F658 8006EE58 A6BB0108 */  j          .L8006EE98
    /* 5F65C 8006EE5C 14010226 */   addiu     $v0, $s0, 0x114
  jlabel .L8006EE60
    /* 5F660 8006EE60 DC04828F */  lw         $v0, %gp_rel(D_800A35A8)($gp)
    /* 5F664 8006EE64 00000000 */  nop
    /* 5F668 8006EE68 8C00468C */  lw         $a2, 0x8C($v0)
    /* 5F66C 8006EE6C A6BB0108 */  j          .L8006EE98
    /* 5F670 8006EE70 20010226 */   addiu     $v0, $s0, 0x120
  jlabel .L8006EE74
    /* 5F674 8006EE74 DC04828F */  lw         $v0, %gp_rel(D_800A35A8)($gp)
    /* 5F678 8006EE78 00000000 */  nop
    /* 5F67C 8006EE7C 9000468C */  lw         $a2, 0x90($v0)
    /* 5F680 8006EE80 A6BB0108 */  j          .L8006EE98
    /* 5F684 8006EE84 2C010226 */   addiu     $v0, $s0, 0x12C
  jlabel .L8006EE88
    /* 5F688 8006EE88 DC04828F */  lw         $v0, %gp_rel(D_800A35A8)($gp)
    /* 5F68C 8006EE8C 00000000 */  nop
    /* 5F690 8006EE90 9400468C */  lw         $a2, 0x94($v0)
    /* 5F694 8006EE94 38010226 */  addiu      $v0, $s0, 0x138
  .L8006EE98:
    /* 5F698 8006EE98 1000A2AF */  sw         $v0, 0x10($sp)
    /* 5F69C 8006EE9C 00141100 */  sll        $v0, $s1, 16
    /* 5F6A0 8006EEA0 1C004010 */  beqz       $v0, .L8006EF14
    /* 5F6A4 8006EEA4 40100300 */   sll       $v0, $v1, 1
    /* 5F6A8 8006EEA8 1C028293 */  lbu        $v0, %gp_rel(D_800A32E8)($gp)
    /* 5F6AC 8006EEAC 00000000 */  nop
    /* 5F6B0 8006EEB0 06004314 */  bne        $v0, $v1, .L8006EECC
    /* 5F6B4 8006EEB4 00000000 */   nop
    /* 5F6B8 8006EEB8 1D028393 */  lbu        $v1, %gp_rel(D_800A32E9)($gp)
    /* 5F6BC 8006EEBC 88048287 */  lh         $v0, %gp_rel(D_800A3554)($gp)
    /* 5F6C0 8006EEC0 00000000 */  nop
    /* 5F6C4 8006EEC4 17006210 */  beq        $v1, $v0, .L8006EF24
    /* 5F6C8 8006EEC8 00000000 */   nop
  .L8006EECC:
    /* 5F6CC 8006EECC 0A80053C */  lui        $a1, %hi(D_800A32F4)
    /* 5F6D0 8006EED0 F432A524 */  addiu      $a1, $a1, %lo(D_800A32F4)
    /* 5F6D4 8006EED4 0300A288 */  lwl        $v0, 0x3($a1)
    /* 5F6D8 8006EED8 0000A298 */  lwr        $v0, 0x0($a1)
    /* 5F6DC 8006EEDC 0700A388 */  lwl        $v1, 0x7($a1)
    /* 5F6E0 8006EEE0 0400A398 */  lwr        $v1, 0x4($a1)
    /* 5F6E4 8006EEE4 4300A2AB */  swl        $v0, 0x43($sp)
    /* 5F6E8 8006EEE8 4000A2BB */  swr        $v0, 0x40($sp)
    /* 5F6EC 8006EEEC 4700A3AB */  swl        $v1, 0x47($sp)
    /* 5F6F0 8006EEF0 4400A3BB */  swr        $v1, 0x44($sp)
    /* 5F6F4 8006EEF4 4000A427 */  addiu      $a0, $sp, 0x40
    /* 5F6F8 8006EEF8 80ED010C */  jal        gpu_LoadImage
    /* 5F6FC 8006EEFC 2128C000 */   addu      $a1, $a2, $zero
    /* 5F700 8006EF00 CFEC010C */  jal        gpu_DrawSync
    /* 5F704 8006EF04 21200000 */   addu      $a0, $zero, $zero
    /* 5F708 8006EF08 C9BB0108 */  j          .L8006EF24
    /* 5F70C 8006EF0C 00000000 */   nop
  jlabel .L8006EF10
    /* 5F710 8006EF10 40100300 */  sll        $v0, $v1, 1
  .L8006EF14:
    /* 5F714 8006EF14 21104300 */  addu       $v0, $v0, $v1
    /* 5F718 8006EF18 80100200 */  sll        $v0, $v0, 2
    /* 5F71C 8006EF1C 21100202 */  addu       $v0, $s0, $v0
    /* 5F720 8006EF20 1000A2AF */  sw         $v0, 0x10($sp)
  .L8006EF24:
    /* 5F724 8006EF24 E404828F */  lw         $v0, %gp_rel(D_800A35B0)($gp)
    /* 5F728 8006EF28 00000000 */  nop
    /* 5F72C 8006EF2C 1D004014 */  bnez       $v0, .L8006EFA4
    /* 5F730 8006EF30 00141100 */   sll       $v0, $s1, 16
    /* 5F734 8006EF34 001C1100 */  sll        $v1, $s1, 16
    /* 5F738 8006EF38 031C0300 */  sra        $v1, $v1, 16
    /* 5F73C 8006EF3C 40100300 */  sll        $v0, $v1, 1
    /* 5F740 8006EF40 21104300 */  addu       $v0, $v0, $v1
    /* 5F744 8006EF44 0A80013C */  lui        $at, %hi(D_800A3561)
    /* 5F748 8006EF48 21082200 */  addu       $at, $at, $v0
    /* 5F74C 8006EF4C 61352290 */  lbu        $v0, %lo(D_800A3561)($at)
    /* 5F750 8006EF50 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5F754 8006EF54 21082200 */  addu       $at, $at, $v0
    /* 5F758 8006EF58 7CBC2290 */  lbu        $v0, %lo(D_8009BC7C)($at)
    /* 5F75C 8006EF5C 00000000 */  nop
    /* 5F760 8006EF60 02004230 */  andi       $v0, $v0, 0x2
    /* 5F764 8006EF64 0F004014 */  bnez       $v0, .L8006EFA4
    /* 5F768 8006EF68 00141100 */   sll       $v0, $s1, 16
    /* 5F76C 8006EF6C F004838F */  lw         $v1, %gp_rel(D_800A35BC)($gp)
    /* 5F770 8006EF70 02000224 */  addiu      $v0, $zero, 0x2
    /* 5F774 8006EF74 08006214 */  bne        $v1, $v0, .L8006EF98
    /* 5F778 8006EF78 0200033C */   lui       $v1, (0x20000 >> 16)
    /* 5F77C 8006EF7C 9C04828F */  lw         $v0, %gp_rel(D_800A3568)($gp)
    /* 5F780 8006EF80 00000000 */  nop
    /* 5F784 8006EF84 1400428C */  lw         $v0, 0x14($v0)
    /* 5F788 8006EF88 00000000 */  nop
    /* 5F78C 8006EF8C 24104300 */  and        $v0, $v0, $v1
    /* 5F790 8006EF90 04004014 */  bnez       $v0, .L8006EFA4
    /* 5F794 8006EF94 00141100 */   sll       $v0, $s1, 16
  .L8006EF98:
    /* 5F798 8006EF98 0400628E */  lw         $v0, 0x4($s3)
    /* 5F79C 8006EF9C EEBB0108 */  j          .L8006EFB8
    /* 5F7A0 8006EFA0 1400A2AF */   sw        $v0, 0x14($sp)
  .L8006EFA4:
    /* 5F7A4 8006EFA4 83130200 */  sra        $v0, $v0, 14
    /* 5F7A8 8006EFA8 21105300 */  addu       $v0, $v0, $s3
    /* 5F7AC 8006EFAC 0000428C */  lw         $v0, 0x0($v0)
    /* 5F7B0 8006EFB0 00000000 */  nop
    /* 5F7B4 8006EFB4 1400A2AF */  sw         $v0, 0x14($sp)
  .L8006EFB8:
    /* 5F7B8 8006EFB8 0400428E */  lw         $v0, 0x4($s2)
    /* 5F7BC 8006EFBC 00000000 */  nop
    /* 5F7C0 8006EFC0 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 5F7C4 8006EFC4 00141100 */  sll        $v0, $s1, 16
    /* 5F7C8 8006EFC8 03004010 */  beqz       $v0, .L8006EFD8
    /* 5F7CC 8006EFCC 1000A427 */   addiu     $a0, $sp, 0x10
    /* 5F7D0 8006EFD0 F7BB0108 */  j          .L8006EFDC
    /* 5F7D4 8006EFD4 01000524 */   addiu     $a1, $zero, 0x1
  .L8006EFD8:
    /* 5F7D8 8006EFD8 21280000 */  addu       $a1, $zero, $zero
  .L8006EFDC:
    /* 5F7DC 8006EFDC CACD010C */  jal        func_80073728
    /* 5F7E0 8006EFE0 00000000 */   nop
    /* 5F7E4 8006EFE4 040042AE */  sw         $v0, 0x4($s2)
    /* 5F7E8 8006EFE8 01002226 */  addiu      $v0, $s1, 0x1
    /* 5F7EC 8006EFEC 21884000 */  addu       $s1, $v0, $zero
    /* 5F7F0 8006EFF0 00140200 */  sll        $v0, $v0, 16
    /* 5F7F4 8006EFF4 03140200 */  sra        $v0, $v0, 16
    /* 5F7F8 8006EFF8 88048387 */  lh         $v1, %gp_rel(D_800A3554)($gp)
    /* 5F7FC 8006EFFC E404848F */  lw         $a0, %gp_rel(D_800A35B0)($gp)
    /* 5F800 8006F000 01006324 */  addiu      $v1, $v1, 0x1
    /* 5F804 8006F004 21208300 */  addu       $a0, $a0, $v1
    /* 5F808 8006F008 2A104400 */  slt        $v0, $v0, $a0
    /* 5F80C 8006F00C 55FF4014 */  bnez       $v0, .L8006ED64
    /* 5F810 8006F010 00141100 */   sll       $v0, $s1, 16
  .L8006F014:
    /* 5F814 8006F014 5C00BF8F */  lw         $ra, 0x5C($sp)
    /* 5F818 8006F018 5800B48F */  lw         $s4, 0x58($sp)
    /* 5F81C 8006F01C 5400B38F */  lw         $s3, 0x54($sp)
    /* 5F820 8006F020 5000B28F */  lw         $s2, 0x50($sp)
    /* 5F824 8006F024 4C00B18F */  lw         $s1, 0x4C($sp)
    /* 5F828 8006F028 4800B08F */  lw         $s0, 0x48($sp)
    /* 5F82C 8006F02C 6000BD27 */  addiu      $sp, $sp, 0x60
    /* 5F830 8006F030 0800E003 */  jr         $ra
    /* 5F834 8006F034 00000000 */   nop
endlabel func_8006ECF4
