glabel func_8006F528
    /* 5FD28 8006F528 A0FFBD27 */  addiu      $sp, $sp, -0x60
    /* 5FD2C 8006F52C DC04838F */  lw         $v1, %gp_rel(D_800A35A8)($gp)
    /* 5FD30 8006F530 00010224 */  addiu      $v0, $zero, 0x100
    /* 5FD34 8006F534 5800BFAF */  sw         $ra, 0x58($sp)
    /* 5FD38 8006F538 5400B3AF */  sw         $s3, 0x54($sp)
    /* 5FD3C 8006F53C 5000B2AF */  sw         $s2, 0x50($sp)
    /* 5FD40 8006F540 4C00B1AF */  sw         $s1, 0x4C($sp)
    /* 5FD44 8006F544 4800B0AF */  sw         $s0, 0x48($sp)
    /* 5FD48 8006F548 2000A0AF */  sw         $zero, 0x20($sp)
    /* 5FD4C 8006F54C 2C00A0AF */  sw         $zero, 0x2C($sp)
    /* 5FD50 8006F550 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5FD54 8006F554 3400A2AF */  sw         $v0, 0x34($sp)
    /* 5FD58 8006F558 3800A0A3 */  sb         $zero, 0x38($sp)
    /* 5FD5C 8006F55C 5C00738C */  lw         $s3, 0x5C($v1)
    /* 5FD60 8006F560 10000224 */  addiu      $v0, $zero, 0x10
    /* 5FD64 8006F564 2400A2AF */  sw         $v0, 0x24($sp)
    /* 5FD68 8006F568 AC048293 */  lbu        $v0, %gp_rel(D_800A3578)($gp)
    /* 5FD6C 8006F56C 21888000 */  addu       $s1, $a0, $zero
    /* 5FD70 8006F570 21904000 */  addu       $s2, $v0, $zero
    /* 5FD74 8006F574 0000638E */  lw         $v1, 0x0($s3)
    /* 5FD78 8006F578 03004228 */  slti       $v0, $v0, 0x3
    /* 5FD7C 8006F57C 0C006524 */  addiu      $a1, $v1, 0xC
    /* 5FD80 8006F580 04004014 */  bnez       $v0, .L8006F594
    /* 5FD84 8006F584 1000A3AF */   sw        $v1, 0x10($sp)
    /* 5FD88 8006F588 A4048287 */  lh         $v0, %gp_rel(D_800A3570)($gp)
    /* 5FD8C 8006F58C 66BD0108 */  j          .L8006F598
    /* 5FD90 8006F590 2800A2AF */   sw        $v0, 0x28($sp)
  .L8006F594:
    /* 5FD94 8006F594 2800A0AF */  sw         $zero, 0x28($sp)
  .L8006F598:
    /* 5FD98 8006F598 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5FD9C 8006F59C 1400A5AF */  sw         $a1, 0x14($sp)
    /* 5FDA0 8006F5A0 0400228E */  lw         $v0, 0x4($s1)
    /* 5FDA4 8006F5A4 21280000 */  addu       $a1, $zero, $zero
    /* 5FDA8 8006F5A8 CACD010C */  jal        func_80073728
    /* 5FDAC 8006F5AC 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 5FDB0 8006F5B0 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5FDB4 8006F5B4 040022AE */  sw         $v0, 0x4($s1)
    /* 5FDB8 8006F5B8 1400A28F */  lw         $v0, 0x14($sp)
    /* 5FDBC 8006F5BC 004C0324 */  addiu      $v1, $zero, 0x4C00
    /* 5FDC0 8006F5C0 3400A3AF */  sw         $v1, 0x34($sp)
    /* 5FDC4 8006F5C4 08004224 */  addiu      $v0, $v0, 0x8
    /* 5FDC8 8006F5C8 1400A2AF */  sw         $v0, 0x14($sp)
    /* 5FDCC 8006F5CC 0400228E */  lw         $v0, 0x4($s1)
    /* 5FDD0 8006F5D0 21280000 */  addu       $a1, $zero, $zero
    /* 5FDD4 8006F5D4 CACD010C */  jal        func_80073728
    /* 5FDD8 8006F5D8 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 5FDDC 8006F5DC 040022AE */  sw         $v0, 0x4($s1)
    /* 5FDE0 8006F5E0 00010224 */  addiu      $v0, $zero, 0x100
    /* 5FDE4 8006F5E4 3400A2AF */  sw         $v0, 0x34($sp)
    /* 5FDE8 8006F5E8 0400628E */  lw         $v0, 0x4($s3)
    /* 5FDEC 8006F5EC 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5FDF0 8006F5F0 0C004524 */  addiu      $a1, $v0, 0xC
    /* 5FDF4 8006F5F4 1000A2AF */  sw         $v0, 0x10($sp)
    /* 5FDF8 8006F5F8 1400A5AF */  sw         $a1, 0x14($sp)
    /* 5FDFC 8006F5FC 0400228E */  lw         $v0, 0x4($s1)
    /* 5FE00 8006F600 21280000 */  addu       $a1, $zero, $zero
    /* 5FE04 8006F604 CACD010C */  jal        func_80073728
    /* 5FE08 8006F608 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 5FE0C 8006F60C 040022AE */  sw         $v0, 0x4($s1)
    /* 5FE10 8006F610 0000628E */  lw         $v0, 0x0($s3)
    /* 5FE14 8006F614 00000000 */  nop
    /* 5FE18 8006F618 0C004524 */  addiu      $a1, $v0, 0xC
    /* 5FE1C 8006F61C 1000A2AF */  sw         $v0, 0x10($sp)
    /* 5FE20 8006F620 0300422A */  slti       $v0, $s2, 0x3
    /* 5FE24 8006F624 06004010 */  beqz       $v0, .L8006F640
    /* 5FE28 8006F628 00000000 */   nop
    /* 5FE2C 8006F62C A4048287 */  lh         $v0, %gp_rel(D_800A3570)($gp)
    /* 5FE30 8006F630 00000000 */  nop
    /* 5FE34 8006F634 23100200 */  negu       $v0, $v0
    /* 5FE38 8006F638 91BD0108 */  j          .L8006F644
    /* 5FE3C 8006F63C 00024224 */   addiu     $v0, $v0, 0x200
  .L8006F640:
    /* 5FE40 8006F640 00020224 */  addiu      $v0, $zero, 0x200
  .L8006F644:
    /* 5FE44 8006F644 2800A2AF */  sw         $v0, 0x28($sp)
    /* 5FE48 8006F648 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5FE4C 8006F64C 1400A5AF */  sw         $a1, 0x14($sp)
    /* 5FE50 8006F650 0400228E */  lw         $v0, 0x4($s1)
    /* 5FE54 8006F654 01000524 */  addiu      $a1, $zero, 0x1
    /* 5FE58 8006F658 CACD010C */  jal        func_80073728
    /* 5FE5C 8006F65C 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 5FE60 8006F660 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5FE64 8006F664 040022AE */  sw         $v0, 0x4($s1)
    /* 5FE68 8006F668 1400A28F */  lw         $v0, 0x14($sp)
    /* 5FE6C 8006F66C 004C0324 */  addiu      $v1, $zero, 0x4C00
    /* 5FE70 8006F670 3400A3AF */  sw         $v1, 0x34($sp)
    /* 5FE74 8006F674 08004224 */  addiu      $v0, $v0, 0x8
    /* 5FE78 8006F678 1400A2AF */  sw         $v0, 0x14($sp)
    /* 5FE7C 8006F67C 0400228E */  lw         $v0, 0x4($s1)
    /* 5FE80 8006F680 21280000 */  addu       $a1, $zero, $zero
    /* 5FE84 8006F684 CACD010C */  jal        func_80073728
    /* 5FE88 8006F688 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 5FE8C 8006F68C 040022AE */  sw         $v0, 0x4($s1)
    /* 5FE90 8006F690 00010224 */  addiu      $v0, $zero, 0x100
    /* 5FE94 8006F694 3400A2AF */  sw         $v0, 0x34($sp)
    /* 5FE98 8006F698 0400628E */  lw         $v0, 0x4($s3)
    /* 5FE9C 8006F69C 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5FEA0 8006F6A0 0C004524 */  addiu      $a1, $v0, 0xC
    /* 5FEA4 8006F6A4 1000A2AF */  sw         $v0, 0x10($sp)
    /* 5FEA8 8006F6A8 1400A5AF */  sw         $a1, 0x14($sp)
    /* 5FEAC 8006F6AC 0400228E */  lw         $v0, 0x4($s1)
    /* 5FEB0 8006F6B0 01000524 */  addiu      $a1, $zero, 0x1
    /* 5FEB4 8006F6B4 CACD010C */  jal        func_80073728
    /* 5FEB8 8006F6B8 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 5FEBC 8006F6BC 040022AE */  sw         $v0, 0x4($s1)
    /* 5FEC0 8006F6C0 0300422A */  slti       $v0, $s2, 0x3
    /* 5FEC4 8006F6C4 04004014 */  bnez       $v0, .L8006F6D8
    /* 5FEC8 8006F6C8 00000000 */   nop
    /* 5FECC 8006F6CC A4048297 */  lhu        $v0, %gp_rel(D_800A3570)($gp)
    /* 5FED0 8006F6D0 B7BD0108 */  j          .L8006F6DC
    /* 5FED4 8006F6D4 4C004224 */   addiu     $v0, $v0, 0x4C
  .L8006F6D8:
    /* 5FED8 8006F6D8 4C000224 */  addiu      $v0, $zero, 0x4C
  .L8006F6DC:
    /* 5FEDC 8006F6DC 4000A2A7 */  sh         $v0, 0x40($sp)
    /* 5FEE0 8006F6E0 E8010224 */  addiu      $v0, $zero, 0x1E8
    /* 5FEE4 8006F6E4 4000B027 */  addiu      $s0, $sp, 0x40
    /* 5FEE8 8006F6E8 F404838F */  lw         $v1, %gp_rel(D_800A35C0)($gp)
    /* 5FEEC 8006F6EC A4048497 */  lhu        $a0, %gp_rel(D_800A3570)($gp)
    /* 5FEF0 8006F6F0 02006394 */  lhu        $v1, 0x2($v1)
    /* 5FEF4 8006F6F4 23104400 */  subu       $v0, $v0, $a0
    /* 5FEF8 8006F6F8 4400A2A7 */  sh         $v0, 0x44($sp)
    /* 5FEFC 8006F6FC 54000224 */  addiu      $v0, $zero, 0x54
    /* 5FF00 8006F700 4600A2A7 */  sh         $v0, 0x46($sp)
    /* 5FF04 8006F704 7C006324 */  addiu      $v1, $v1, 0x7C
    /* 5FF08 8006F708 4200A3A7 */  sh         $v1, 0x42($sp)
    /* 5FF0C 8006F70C 1C00248E */  lw         $a0, 0x1C($s1)
    /* 5FF10 8006F710 55F0010C */  jal        initDrawArea
    /* 5FF14 8006F714 21280002 */   addu      $a1, $s0, $zero
    /* 5FF18 8006F718 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5FF1C 8006F71C 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5FF20 8006F720 1C00258E */  lw         $a1, 0x1C($s1)
    /* 5FF24 8006F724 2DEA010C */  jal        ot_Link
    /* 5FF28 8006F728 3C008424 */   addiu     $a0, $a0, 0x3C
    /* 5FF2C 8006F72C 1C00228E */  lw         $v0, 0x1C($s1)
    /* 5FF30 8006F730 F404838F */  lw         $v1, %gp_rel(D_800A35C0)($gp)
    /* 5FF34 8006F734 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5FF38 8006F738 1C0022AE */  sw         $v0, 0x1C($s1)
    /* 5FF3C 8006F73C 00006294 */  lhu        $v0, 0x0($v1)
    /* 5FF40 8006F740 00000000 */  nop
    /* 5FF44 8006F744 4000A2A7 */  sh         $v0, 0x40($sp)
    /* 5FF48 8006F748 02006294 */  lhu        $v0, 0x2($v1)
    /* 5FF4C 8006F74C 00000000 */  nop
    /* 5FF50 8006F750 4200A2A7 */  sh         $v0, 0x42($sp)
    /* 5FF54 8006F754 04006294 */  lhu        $v0, 0x4($v1)
    /* 5FF58 8006F758 00000000 */  nop
    /* 5FF5C 8006F75C 4400A2A7 */  sh         $v0, 0x44($sp)
    /* 5FF60 8006F760 06006294 */  lhu        $v0, 0x6($v1)
    /* 5FF64 8006F764 00000000 */  nop
    /* 5FF68 8006F768 4600A2A7 */  sh         $v0, 0x46($sp)
    /* 5FF6C 8006F76C 1C00248E */  lw         $a0, 0x1C($s1)
    /* 5FF70 8006F770 55F0010C */  jal        initDrawArea
    /* 5FF74 8006F774 21280002 */   addu      $a1, $s0, $zero
    /* 5FF78 8006F778 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5FF7C 8006F77C 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5FF80 8006F780 1C00258E */  lw         $a1, 0x1C($s1)
    /* 5FF84 8006F784 2DEA010C */  jal        ot_Link
    /* 5FF88 8006F788 18008424 */   addiu     $a0, $a0, 0x18
    /* 5FF8C 8006F78C 21184002 */  addu       $v1, $s2, $zero
    /* 5FF90 8006F790 1C00228E */  lw         $v0, 0x1C($s1)
    /* 5FF94 8006F794 02000624 */  addiu      $a2, $zero, 0x2
    /* 5FF98 8006F798 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5FF9C 8006F79C 04006610 */  beq        $v1, $a2, .L8006F7B0
    /* 5FFA0 8006F7A0 1C0022AE */   sw        $v0, 0x1C($s1)
    /* 5FFA4 8006F7A4 04000224 */  addiu      $v0, $zero, 0x4
    /* 5FFA8 8006F7A8 31006214 */  bne        $v1, $v0, .L8006F870
    /* 5FFAC 8006F7AC 00000000 */   nop
  .L8006F7B0:
    /* 5FFB0 8006F7B0 F404828F */  lw         $v0, %gp_rel(D_800A35C0)($gp)
    /* 5FFB4 8006F7B4 F804858F */  lw         $a1, %gp_rel(D_800A35C4)($gp)
    /* 5FFB8 8006F7B8 08004484 */  lh         $a0, 0x8($v0)
    /* 5FFBC 8006F7BC 04006614 */  bne        $v1, $a2, .L8006F7D0
    /* 5FFC0 8006F7C0 00000000 */   nop
    /* 5FFC4 8006F7C4 A4048287 */  lh         $v0, %gp_rel(D_800A3570)($gp)
    /* 5FFC8 8006F7C8 F7BD0108 */  j          .L8006F7DC
    /* 5FFCC 8006F7CC 23108200 */   subu      $v0, $a0, $v0
  .L8006F7D0:
    /* 5FFD0 8006F7D0 A4048287 */  lh         $v0, %gp_rel(D_800A3570)($gp)
    /* 5FFD4 8006F7D4 00000000 */  nop
    /* 5FFD8 8006F7D8 21108200 */  addu       $v0, $a0, $v0
  .L8006F7DC:
    /* 5FFDC 8006F7DC 1000A2A4 */  sh         $v0, 0x10($a1)
    /* 5FFE0 8006F7E0 F404828F */  lw         $v0, %gp_rel(D_800A35C0)($gp)
    /* 5FFE4 8006F7E4 F804858F */  lw         $a1, %gp_rel(D_800A35C4)($gp)
    /* 5FFE8 8006F7E8 0A004294 */  lhu        $v0, 0xA($v0)
    /* 5FFEC 8006F7EC 00000000 */  nop
    /* 5FFF0 8006F7F0 1200A2A4 */  sh         $v0, 0x12($a1)
    /* 5FFF4 8006F7F4 2000248E */  lw         $a0, 0x20($s1)
    /* 5FFF8 8006F7F8 76F0010C */  jal        initDrawOffset
    /* 5FFFC 8006F7FC 1000A524 */   addiu     $a1, $a1, 0x10
    /* 60000 8006F800 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 60004 8006F804 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 60008 8006F808 2000258E */  lw         $a1, 0x20($s1)
    /* 6000C 8006F80C 2DEA010C */  jal        ot_Link
    /* 60010 8006F810 3C008424 */   addiu     $a0, $a0, 0x3C
    /* 60014 8006F814 2000228E */  lw         $v0, 0x20($s1)
    /* 60018 8006F818 F404838F */  lw         $v1, %gp_rel(D_800A35C0)($gp)
    /* 6001C 8006F81C F804858F */  lw         $a1, %gp_rel(D_800A35C4)($gp)
    /* 60020 8006F820 0C004224 */  addiu      $v0, $v0, 0xC
    /* 60024 8006F824 200022AE */  sw         $v0, 0x20($s1)
    /* 60028 8006F828 08006294 */  lhu        $v0, 0x8($v1)
    /* 6002C 8006F82C 00000000 */  nop
    /* 60030 8006F830 1000A2A4 */  sh         $v0, 0x10($a1)
    /* 60034 8006F834 0A006294 */  lhu        $v0, 0xA($v1)
    /* 60038 8006F838 00000000 */  nop
    /* 6003C 8006F83C 1200A2A4 */  sh         $v0, 0x12($a1)
    /* 60040 8006F840 2000248E */  lw         $a0, 0x20($s1)
    /* 60044 8006F844 76F0010C */  jal        initDrawOffset
    /* 60048 8006F848 1000A524 */   addiu     $a1, $a1, 0x10
    /* 6004C 8006F84C 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 60050 8006F850 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 60054 8006F854 2000258E */  lw         $a1, 0x20($s1)
    /* 60058 8006F858 2DEA010C */  jal        ot_Link
    /* 6005C 8006F85C 18008424 */   addiu     $a0, $a0, 0x18
    /* 60060 8006F860 2000228E */  lw         $v0, 0x20($s1)
    /* 60064 8006F864 00000000 */  nop
    /* 60068 8006F868 0C004224 */  addiu      $v0, $v0, 0xC
    /* 6006C 8006F86C 200022AE */  sw         $v0, 0x20($s1)
  .L8006F870:
    /* 60070 8006F870 1400308E */  lw         $s0, 0x14($s1)
    /* 60074 8006F874 B4EA010C */  jal        initTile
    /* 60078 8006F878 21200002 */   addu      $a0, $s0, $zero
    /* 6007C 8006F87C 21200002 */  addu       $a0, $s0, $zero
    /* 60080 8006F880 5AEA010C */  jal        gpu_SetSemiTransp
    /* 60084 8006F884 21280000 */   addu      $a1, $zero, $zero
    /* 60088 8006F888 9C04828F */  lw         $v0, %gp_rel(D_800A3568)($gp)
    /* 6008C 8006F88C 00000000 */  nop
    /* 60090 8006F890 2000428C */  lw         $v0, 0x20($v0)
    /* 60094 8006F894 00000000 */  nop
    /* 60098 8006F898 01004230 */  andi       $v0, $v0, 0x1
    /* 6009C 8006F89C 04004010 */  beqz       $v0, .L8006F8B0
    /* 600A0 8006F8A0 C8000224 */   addiu     $v0, $zero, 0xC8
    /* 600A4 8006F8A4 040002A2 */  sb         $v0, 0x4($s0)
    /* 600A8 8006F8A8 31BE0108 */  j          .L8006F8C4
    /* 600AC 8006F8AC 050002A2 */   sb        $v0, 0x5($s0)
  .L8006F8B0:
    /* 600B0 8006F8B0 D0000224 */  addiu      $v0, $zero, 0xD0
    /* 600B4 8006F8B4 040002A2 */  sb         $v0, 0x4($s0)
    /* 600B8 8006F8B8 C8000224 */  addiu      $v0, $zero, 0xC8
    /* 600BC 8006F8BC 050002A2 */  sb         $v0, 0x5($s0)
    /* 600C0 8006F8C0 B8000224 */  addiu      $v0, $zero, 0xB8
  .L8006F8C4:
    /* 600C4 8006F8C4 060002A2 */  sb         $v0, 0x6($s0)
    /* 600C8 8006F8C8 21280002 */  addu       $a1, $s0, $zero
    /* 600CC 8006F8CC 4C000324 */  addiu      $v1, $zero, 0x4C
    /* 600D0 8006F8D0 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 600D4 8006F8D4 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 600D8 8006F8D8 80000224 */  addiu      $v0, $zero, 0x80
    /* 600DC 8006F8DC 0A0002A6 */  sh         $v0, 0xA($s0)
    /* 600E0 8006F8E0 15020224 */  addiu      $v0, $zero, 0x215
    /* 600E4 8006F8E4 080003A6 */  sh         $v1, 0x8($s0)
    /* 600E8 8006F8E8 0C0002A6 */  sh         $v0, 0xC($s0)
    /* 600EC 8006F8EC 0E0003A6 */  sh         $v1, 0xE($s0)
    /* 600F0 8006F8F0 10001026 */  addiu      $s0, $s0, 0x10
    /* 600F4 8006F8F4 2DEA010C */  jal        ot_Link
    /* 600F8 8006F8F8 38008424 */   addiu     $a0, $a0, 0x38
    /* 600FC 8006F8FC 1000A427 */  addiu      $a0, $sp, 0x10
    /* 60100 8006F900 140030AE */  sw         $s0, 0x14($s1)
    /* 60104 8006F904 0800638E */  lw         $v1, 0x8($s3)
    /* 60108 8006F908 00010224 */  addiu      $v0, $zero, 0x100
    /* 6010C 8006F90C 3400A2AF */  sw         $v0, 0x34($sp)
    /* 60110 8006F910 0E000224 */  addiu      $v0, $zero, 0xE
    /* 60114 8006F914 2800A0AF */  sw         $zero, 0x28($sp)
    /* 60118 8006F918 2400A2AF */  sw         $v0, 0x24($sp)
    /* 6011C 8006F91C 0C006524 */  addiu      $a1, $v1, 0xC
    /* 60120 8006F920 1000A3AF */  sw         $v1, 0x10($sp)
    /* 60124 8006F924 1400A5AF */  sw         $a1, 0x14($sp)
    /* 60128 8006F928 0400228E */  lw         $v0, 0x4($s1)
    /* 6012C 8006F92C 21280000 */  addu       $a1, $zero, $zero
    /* 60130 8006F930 CACD010C */  jal        func_80073728
    /* 60134 8006F934 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 60138 8006F938 1000A427 */  addiu      $a0, $sp, 0x10
    /* 6013C 8006F93C 040022AE */  sw         $v0, 0x4($s1)
    /* 60140 8006F940 50000224 */  addiu      $v0, $zero, 0x50
    /* 60144 8006F944 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 60148 8006F948 0400228E */  lw         $v0, 0x4($s1)
    /* 6014C 8006F94C 02000524 */  addiu      $a1, $zero, 0x2
    /* 60150 8006F950 CACD010C */  jal        func_80073728
    /* 60154 8006F954 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 60158 8006F958 040022AE */  sw         $v0, 0x4($s1)
    /* 6015C 8006F95C 5800BF8F */  lw         $ra, 0x58($sp)
    /* 60160 8006F960 5400B38F */  lw         $s3, 0x54($sp)
    /* 60164 8006F964 5000B28F */  lw         $s2, 0x50($sp)
    /* 60168 8006F968 4C00B18F */  lw         $s1, 0x4C($sp)
    /* 6016C 8006F96C 4800B08F */  lw         $s0, 0x48($sp)
    /* 60170 8006F970 6000BD27 */  addiu      $sp, $sp, 0x60
    /* 60174 8006F974 0800E003 */  jr         $ra
    /* 60178 8006F978 00000000 */   nop
endlabel func_8006F528
