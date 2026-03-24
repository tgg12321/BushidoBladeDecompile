glabel func_8004DDB4
    /* 3E5B4 8004DDB4 74FFBD27 */  addiu      $sp, $sp, -0x8C
    /* 3E5B8 8004DDB8 8800BFAF */  sw         $ra, 0x88($sp)
    /* 3E5BC 8004DDBC 8400BEAF */  sw         $fp, 0x84($sp)
    /* 3E5C0 8004DDC0 8000B7AF */  sw         $s7, 0x80($sp)
    /* 3E5C4 8004DDC4 7C00B6AF */  sw         $s6, 0x7C($sp)
    /* 3E5C8 8004DDC8 7800B5AF */  sw         $s5, 0x78($sp)
    /* 3E5CC 8004DDCC 7400B4AF */  sw         $s4, 0x74($sp)
    /* 3E5D0 8004DDD0 7000B3AF */  sw         $s3, 0x70($sp)
    /* 3E5D4 8004DDD4 6C00B2AF */  sw         $s2, 0x6C($sp)
    /* 3E5D8 8004DDD8 6800B1AF */  sw         $s1, 0x68($sp)
    /* 3E5DC 8004DDDC 6400B0AF */  sw         $s0, 0x64($sp)
    /* 3E5E0 8004DDE0 21F08000 */  addu       $fp, $a0, $zero
    /* 3E5E4 8004DDE4 2180A000 */  addu       $s0, $a1, $zero
    /* 3E5E8 8004DDE8 21B8C000 */  addu       $s7, $a2, $zero
    /* 3E5EC 8004DDEC 2188E000 */  addu       $s1, $a3, $zero
    /* 3E5F0 8004DDF0 0C001524 */  addiu      $s5, $zero, 0xC
    /* 3E5F4 8004DDF4 801F163C */  lui        $s6, (0x1F800020 >> 16)
    /* 3E5F8 8004DDF8 2000D636 */  ori        $s6, $s6, (0x1F800020 & 0xFFFF)
    /* 3E5FC 8004DDFC 2198C002 */  addu       $s3, $s6, $zero
    /* 3E600 8004DE00 2402D422 */  addi       $s4, $s6, 0x224 /* handwritten instruction */
    /* 3E604 8004DE04 02000422 */  addi       $a0, $s0, 0x2 /* handwritten instruction */
    /* 3E608 8004DE08 1051010C */  jal        func_80054440
    /* 3E60C 8004DE0C 2128C003 */   addu      $a1, $fp, $zero
    /* 3E610 8004DE10 FEFF5020 */  addi       $s0, $v0, -0x2 /* handwritten instruction */
  .L8004DE14:
    /* 3E614 8004DE14 0A80043C */  lui        $a0, %hi(D_800A3708)
    /* 3E618 8004DE18 0837848C */  lw         $a0, %lo(D_800A3708)($a0)
    /* 3E61C 8004DE1C 0000D28F */  lw         $s2, 0x0($fp)
    /* 3E620 8004DE20 2C00888C */  lw         $t0, 0x2C($a0)
    /* 3E624 8004DE24 3000898C */  lw         $t1, 0x30($a0)
    /* 3E628 8004DE28 34008A8C */  lw         $t2, 0x34($a0)
    /* 3E62C 8004DE2C 14004B8E */  lw         $t3, 0x14($s2)
    /* 3E630 8004DE30 18004C8E */  lw         $t4, 0x18($s2)
    /* 3E634 8004DE34 1C004D8E */  lw         $t5, 0x1C($s2)
    /* 3E638 8004DE38 22586801 */  sub        $t3, $t3, $t0 /* handwritten instruction */
    /* 3E63C 8004DE3C 22608901 */  sub        $t4, $t4, $t1 /* handwritten instruction */
    /* 3E640 8004DE40 2268AA01 */  sub        $t5, $t5, $t2 /* handwritten instruction */
    /* 3E644 8004DE44 07582B02 */  srav       $t3, $t3, $s1
    /* 3E648 8004DE48 07602C02 */  srav       $t4, $t4, $s1
    /* 3E64C 8004DE4C 07682D02 */  srav       $t5, $t5, $s1
    /* 3E650 8004DE50 6803CBAE */  sw         $t3, 0x368($s6)
    /* 3E654 8004DE54 6C03CCAE */  sw         $t4, 0x36C($s6)
    /* 3E658 8004DE58 7003CDAE */  sw         $t5, 0x370($s6)
    /* 3E65C 8004DE5C 4003C222 */  addi       $v0, $s6, 0x340 /* handwritten instruction */
    /* 3E660 8004DE60 00004886 */  lh         $t0, 0x0($s2)
    /* 3E664 8004DE64 02004986 */  lh         $t1, 0x2($s2)
    /* 3E668 8004DE68 04004A86 */  lh         $t2, 0x4($s2)
    /* 3E66C 8004DE6C 06004B86 */  lh         $t3, 0x6($s2)
    /* 3E670 8004DE70 08004C86 */  lh         $t4, 0x8($s2)
    /* 3E674 8004DE74 0A004D86 */  lh         $t5, 0xA($s2)
    /* 3E678 8004DE78 0C004E86 */  lh         $t6, 0xC($s2)
    /* 3E67C 8004DE7C 0E004F86 */  lh         $t7, 0xE($s2)
    /* 3E680 8004DE80 10005886 */  lh         $t8, 0x10($s2)
    /* 3E684 8004DE84 07402802 */  srav       $t0, $t0, $s1
    /* 3E688 8004DE88 07482902 */  srav       $t1, $t1, $s1
    /* 3E68C 8004DE8C 07502A02 */  srav       $t2, $t2, $s1
    /* 3E690 8004DE90 07582B02 */  srav       $t3, $t3, $s1
    /* 3E694 8004DE94 07602C02 */  srav       $t4, $t4, $s1
    /* 3E698 8004DE98 07682D02 */  srav       $t5, $t5, $s1
    /* 3E69C 8004DE9C 07702E02 */  srav       $t6, $t6, $s1
    /* 3E6A0 8004DEA0 07782F02 */  srav       $t7, $t7, $s1
    /* 3E6A4 8004DEA4 07C03802 */  srav       $t8, $t8, $s1
    /* 3E6A8 8004DEA8 000048A4 */  sh         $t0, 0x0($v0)
    /* 3E6AC 8004DEAC 020049A4 */  sh         $t1, 0x2($v0)
    /* 3E6B0 8004DEB0 04004AA4 */  sh         $t2, 0x4($v0)
    /* 3E6B4 8004DEB4 06004BA4 */  sh         $t3, 0x6($v0)
    /* 3E6B8 8004DEB8 08004CA4 */  sh         $t4, 0x8($v0)
    /* 3E6BC 8004DEBC 0A004DA4 */  sh         $t5, 0xA($v0)
    /* 3E6C0 8004DEC0 0C004EA4 */  sh         $t6, 0xC($v0)
    /* 3E6C4 8004DEC4 0E004FA4 */  sh         $t7, 0xE($v0)
    /* 3E6C8 8004DEC8 100058A4 */  sh         $t8, 0x10($v0)
    /* 3E6CC 8004DECC 21904000 */  addu       $s2, $v0, $zero
    /* 3E6D0 8004DED0 0400DE23 */  addi       $fp, $fp, 0x4 /* handwritten instruction */
    /* 3E6D4 8004DED4 2120E002 */  addu       $a0, $s7, $zero
    /* 3E6D8 8004DED8 21284002 */  addu       $a1, $s2, $zero
    /* 3E6DC 8004DEDC 4C4A010C */  jal        func_80052930
    /* 3E6E0 8004DEE0 5403C622 */   addi      $a2, $s6, 0x354 /* handwritten instruction */
    /* 3E6E4 8004DEE4 0000E88E */  lw         $t0, 0x0($s7)
    /* 3E6E8 8004DEE8 0400E98E */  lw         $t1, 0x4($s7)
    /* 3E6EC 8004DEEC 0800EA8E */  lw         $t2, 0x8($s7)
    /* 3E6F0 8004DEF0 0C00EB8E */  lw         $t3, 0xC($s7)
    /* 3E6F4 8004DEF4 1000EC8E */  lw         $t4, 0x10($s7)
    /* 3E6F8 8004DEF8 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3E6FC 8004DEFC 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3E700 8004DF00 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3E704 8004DF04 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3E708 8004DF08 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3E70C 8004DF0C 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 3E710 8004DF10 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 3E714 8004DF14 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
    /* 3E718 8004DF18 6803C88E */  lw         $t0, 0x368($s6)
    /* 3E71C 8004DF1C 6C03C98E */  lw         $t1, 0x36C($s6)
    /* 3E720 8004DF20 7003CA8E */  lw         $t2, 0x370($s6)
    /* 3E724 8004DF24 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3E728 8004DF28 004C0900 */  sll        $t1, $t1, 16
    /* 3E72C 8004DF2C 25482801 */  or         $t1, $t1, $t0
    /* 3E730 8004DF30 00008948 */  mtc2       $t1, $0 /* handwritten instruction */
    /* 3E734 8004DF34 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 3E738 8004DF38 00000000 */  nop
    /* 3E73C 8004DF3C 00000000 */  nop
    /* 3E740 8004DF40 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 3E744 8004DF44 00000000 */  nop
    /* 3E748 8004DF48 00000000 */  nop
    /* 3E74C 8004DF4C 7403D9EA */  swc2       $25, 0x374($s6)
    /* 3E750 8004DF50 7803DAEA */  swc2       $26, 0x378($s6) /* handwritten instruction */
    /* 3E754 8004DF54 7C03DBEA */  swc2       $27, 0x37C($s6) /* handwritten instruction */
    /* 3E758 8004DF58 00000000 */  nop
    /* 3E75C 8004DF5C 02001022 */  addi       $s0, $s0, 0x2 /* handwritten instruction */
    /* 3E760 8004DF60 00000386 */  lh         $v1, 0x0($s0)
    /* 3E764 8004DF64 00000000 */  nop
    /* 3E768 8004DF68 2C006010 */  beqz       $v1, .L8004E01C
    /* 3E76C 8004DF6C 02001022 */   addi      $s0, $s0, 0x2 /* handwritten instruction */
    /* 3E770 8004DF70 FFFF6320 */  addi       $v1, $v1, -0x1 /* handwritten instruction */
    /* 3E774 8004DF74 0000488E */  lw         $t0, 0x0($s2)
    /* 3E778 8004DF78 0400498E */  lw         $t1, 0x4($s2)
    /* 3E77C 8004DF7C 08004A8E */  lw         $t2, 0x8($s2)
    /* 3E780 8004DF80 0C004B8E */  lw         $t3, 0xC($s2)
    /* 3E784 8004DF84 10004C8E */  lw         $t4, 0x10($s2)
    /* 3E788 8004DF88 6803CD8E */  lw         $t5, 0x368($s6)
    /* 3E78C 8004DF8C 6C03CE8E */  lw         $t6, 0x36C($s6)
    /* 3E790 8004DF90 7003CF8E */  lw         $t7, 0x370($s6)
    /* 3E794 8004DF94 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3E798 8004DF98 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3E79C 8004DF9C 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3E7A0 8004DFA0 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3E7A4 8004DFA4 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3E7A8 8004DFA8 0028CD48 */  ctc2       $t5, $5 /* handwritten instruction */
    /* 3E7AC 8004DFAC 0030CE48 */  ctc2       $t6, $6 /* handwritten instruction */
    /* 3E7B0 8004DFB0 0038CF48 */  ctc2       $t7, $7 /* handwritten instruction */
  .L8004DFB4:
    /* 3E7B4 8004DFB4 00000896 */  lhu        $t0, 0x0($s0)
    /* 3E7B8 8004DFB8 02000986 */  lh         $t1, 0x2($s0)
    /* 3E7BC 8004DFBC 04000A86 */  lh         $t2, 0x4($s0)
    /* 3E7C0 8004DFC0 004C0900 */  sll        $t1, $t1, 16
    /* 3E7C4 8004DFC4 25482801 */  or         $t1, $t1, $t0
    /* 3E7C8 8004DFC8 00008948 */  mtc2       $t1, $0 /* handwritten instruction */
    /* 3E7CC 8004DFCC 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 3E7D0 8004DFD0 06000886 */  lh         $t0, 0x6($s0)
    /* 3E7D4 8004DFD4 00000000 */  nop
    /* 3E7D8 8004DFD8 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 3E7DC 8004DFDC 08000986 */  lh         $t1, 0x8($s0)
    /* 3E7E0 8004DFE0 0A000A86 */  lh         $t2, 0xA($s0)
    /* 3E7E4 8004DFE4 0C001022 */  addi       $s0, $s0, 0xC /* handwritten instruction */
    /* 3E7E8 8004DFE8 04007322 */  addi       $s3, $s3, 0x4 /* handwritten instruction */
    /* 3E7EC 8004DFEC 02009422 */  addi       $s4, $s4, 0x2 /* handwritten instruction */
    /* 3E7F0 8004DFF0 00C80B48 */  mfc2       $t3, $25 /* handwritten instruction */
    /* 3E7F4 8004DFF4 00D00C48 */  mfc2       $t4, $26 /* handwritten instruction */
    /* 3E7F8 8004DFF8 00D80D48 */  mfc2       $t5, $27 /* handwritten instruction */
    /* 3E7FC 8004DFFC 20400B01 */  add        $t0, $t0, $t3 /* handwritten instruction */
    /* 3E800 8004E000 20482C01 */  add        $t1, $t1, $t4 /* handwritten instruction */
    /* 3E804 8004E004 20504D01 */  add        $t2, $t2, $t5 /* handwritten instruction */
    /* 3E808 8004E008 FCFF68A6 */  sh         $t0, -0x4($s3)
    /* 3E80C 8004E00C FEFF69A6 */  sh         $t1, -0x2($s3)
    /* 3E810 8004E010 FEFF8AA6 */  sh         $t2, -0x2($s4)
    /* 3E814 8004E014 E7FF6014 */  bnez       $v1, .L8004DFB4
    /* 3E818 8004E018 FFFF6320 */   addi      $v1, $v1, -0x1 /* handwritten instruction */
  .L8004E01C:
    /* 3E81C 8004E01C 00000386 */  lh         $v1, 0x0($s0)
    /* 3E820 8004E020 00000000 */  nop
    /* 3E824 8004E024 46006010 */  beqz       $v1, .L8004E140
    /* 3E828 8004E028 02001022 */   addi      $s0, $s0, 0x2 /* handwritten instruction */
    /* 3E82C 8004E02C FFFF6320 */  addi       $v1, $v1, -0x1 /* handwritten instruction */
  .L8004E030:
    /* 3E830 8004E030 02000896 */  lhu        $t0, 0x2($s0)
    /* 3E834 8004E034 04000986 */  lh         $t1, 0x4($s0)
    /* 3E838 8004E038 06000A86 */  lh         $t2, 0x6($s0)
    /* 3E83C 8004E03C 004C0900 */  sll        $t1, $t1, 16
    /* 3E840 8004E040 25482801 */  or         $t1, $t1, $t0
    /* 3E844 8004E044 00008948 */  mtc2       $t1, $0 /* handwritten instruction */
    /* 3E848 8004E048 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 3E84C 8004E04C 00000886 */  lh         $t0, 0x0($s0)
    /* 3E850 8004E050 08000B86 */  lh         $t3, 0x8($s0)
    /* 3E854 8004E054 0A000C86 */  lh         $t4, 0xA($s0)
    /* 3E858 8004E058 0C000D86 */  lh         $t5, 0xC($s0)
    /* 3E85C 8004E05C 20400801 */  add        $t0, $t0, $t0 /* handwritten instruction */
    /* 3E860 8004E060 2402D922 */  addi       $t9, $s6, 0x224 /* handwritten instruction */
    /* 3E864 8004E064 20C82803 */  add        $t9, $t9, $t0 /* handwritten instruction */
    /* 3E868 8004E068 20400801 */  add        $t0, $t0, $t0 /* handwritten instruction */
    /* 3E86C 8004E06C 20C0C802 */  add        $t8, $s6, $t0 /* handwritten instruction */
    /* 3E870 8004E070 00000887 */  lh         $t0, 0x0($t8)
    /* 3E874 8004E074 02000987 */  lh         $t1, 0x2($t8)
    /* 3E878 8004E078 00002A87 */  lh         $t2, 0x0($t9)
    /* 3E87C 8004E07C 20400B01 */  add        $t0, $t0, $t3 /* handwritten instruction */
    /* 3E880 8004E080 20482C01 */  add        $t1, $t1, $t4 /* handwritten instruction */
    /* 3E884 8004E084 20504D01 */  add        $t2, $t2, $t5 /* handwritten instruction */
    /* 3E888 8004E088 00004B8E */  lw         $t3, 0x0($s2)
    /* 3E88C 8004E08C 04004C8E */  lw         $t4, 0x4($s2)
    /* 3E890 8004E090 08004D8E */  lw         $t5, 0x8($s2)
    /* 3E894 8004E094 0C004E8E */  lw         $t6, 0xC($s2)
    /* 3E898 8004E098 10004F8E */  lw         $t7, 0x10($s2)
    /* 3E89C 8004E09C 0000CB48 */  ctc2       $t3, $0 /* handwritten instruction */
    /* 3E8A0 8004E0A0 0008CC48 */  ctc2       $t4, $1 /* handwritten instruction */
    /* 3E8A4 8004E0A4 0010CD48 */  ctc2       $t5, $2 /* handwritten instruction */
    /* 3E8A8 8004E0A8 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 3E8AC 8004E0AC 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 3E8B0 8004E0B0 0028C848 */  ctc2       $t0, $5 /* handwritten instruction */
    /* 3E8B4 8004E0B4 0030C948 */  ctc2       $t1, $6 /* handwritten instruction */
    /* 3E8B8 8004E0B8 0038CA48 */  ctc2       $t2, $7 /* handwritten instruction */
    /* 3E8BC 8004E0BC 00000000 */  nop
    /* 3E8C0 8004E0C0 00000000 */  nop
    /* 3E8C4 8004E0C4 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 3E8C8 8004E0C8 0000E88E */  lw         $t0, 0x0($s7)
    /* 3E8CC 8004E0CC 0400E98E */  lw         $t1, 0x4($s7)
    /* 3E8D0 8004E0D0 0800EA8E */  lw         $t2, 0x8($s7)
    /* 3E8D4 8004E0D4 0C00EB8E */  lw         $t3, 0xC($s7)
    /* 3E8D8 8004E0D8 1000EC8E */  lw         $t4, 0x10($s7)
    /* 3E8DC 8004E0DC 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3E8E0 8004E0E0 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3E8E4 8004E0E4 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3E8E8 8004E0E8 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3E8EC 8004E0EC 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3E8F0 8004E0F0 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 3E8F4 8004E0F4 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 3E8F8 8004E0F8 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
    /* 3E8FC 8004E0FC 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 3E900 8004E100 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 3E904 8004E104 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 3E908 8004E108 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3E90C 8004E10C 004C0900 */  sll        $t1, $t1, 16
    /* 3E910 8004E110 25482801 */  or         $t1, $t1, $t0
    /* 3E914 8004E114 00008948 */  mtc2       $t1, $0 /* handwritten instruction */
    /* 3E918 8004E118 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 3E91C 8004E11C 00000000 */  nop
    /* 3E920 8004E120 00000000 */  nop
    /* 3E924 8004E124 0100184A */  rtps
    /* 3E928 8004E128 0E001022 */  addi       $s0, $s0, 0xE /* handwritten instruction */
    /* 3E92C 8004E12C 00980848 */  mfc2       $t0, $19 /* handwritten instruction */
    /* 3E930 8004E130 00000EEB */  swc2       $14, 0x0($t8)
    /* 3E934 8004E134 000028A7 */  sh         $t0, 0x0($t9)
    /* 3E938 8004E138 BDFF6014 */  bnez       $v1, .L8004E030
    /* 3E93C 8004E13C FFFF6320 */   addi      $v1, $v1, -0x1 /* handwritten instruction */
  .L8004E140:
    /* 3E940 8004E140 00000386 */  lh         $v1, 0x0($s0)
    /* 3E944 8004E144 00000000 */  nop
    /* 3E948 8004E148 24006010 */  beqz       $v1, .L8004E1DC
    /* 3E94C 8004E14C 02001022 */   addi      $s0, $s0, 0x2 /* handwritten instruction */
    /* 3E950 8004E150 FFFF6320 */  addi       $v1, $v1, -0x1 /* handwritten instruction */
    /* 3E954 8004E154 5403C88E */  lw         $t0, 0x354($s6)
    /* 3E958 8004E158 5803C98E */  lw         $t1, 0x358($s6)
    /* 3E95C 8004E15C 5C03CA8E */  lw         $t2, 0x35C($s6)
    /* 3E960 8004E160 6003CB8E */  lw         $t3, 0x360($s6)
    /* 3E964 8004E164 6403CC8E */  lw         $t4, 0x364($s6)
    /* 3E968 8004E168 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3E96C 8004E16C 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3E970 8004E170 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3E974 8004E174 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3E978 8004E178 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3E97C 8004E17C 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 3E980 8004E180 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 3E984 8004E184 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
  .L8004E188:
    /* 3E988 8004E188 00000896 */  lhu        $t0, 0x0($s0)
    /* 3E98C 8004E18C 02000986 */  lh         $t1, 0x2($s0)
    /* 3E990 8004E190 04000A86 */  lh         $t2, 0x4($s0)
    /* 3E994 8004E194 004C0900 */  sll        $t1, $t1, 16
    /* 3E998 8004E198 25482801 */  or         $t1, $t1, $t0
    /* 3E99C 8004E19C 00008948 */  mtc2       $t1, $0 /* handwritten instruction */
    /* 3E9A0 8004E1A0 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 3E9A4 8004E1A4 00000000 */  nop
    /* 3E9A8 8004E1A8 00000000 */  nop
    /* 3E9AC 8004E1AC 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 3E9B0 8004E1B0 06001022 */  addi       $s0, $s0, 0x6 /* handwritten instruction */
    /* 3E9B4 8004E1B4 04007322 */  addi       $s3, $s3, 0x4 /* handwritten instruction */
    /* 3E9B8 8004E1B8 02009422 */  addi       $s4, $s4, 0x2 /* handwritten instruction */
    /* 3E9BC 8004E1BC 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 3E9C0 8004E1C0 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 3E9C4 8004E1C4 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 3E9C8 8004E1C8 FCFF68A6 */  sh         $t0, -0x4($s3)
    /* 3E9CC 8004E1CC FEFF69A6 */  sh         $t1, -0x2($s3)
    /* 3E9D0 8004E1D0 FEFF8AA6 */  sh         $t2, -0x2($s4)
    /* 3E9D4 8004E1D4 ECFF6014 */  bnez       $v1, .L8004E188
    /* 3E9D8 8004E1D8 FFFF6320 */   addi      $v1, $v1, -0x1 /* handwritten instruction */
  .L8004E1DC:
    /* 3E9DC 8004E1DC 00000386 */  lh         $v1, 0x0($s0)
    /* 3E9E0 8004E1E0 00000000 */  nop
    /* 3E9E4 8004E1E4 2E006010 */  beqz       $v1, .L8004E2A0
    /* 3E9E8 8004E1E8 02001022 */   addi      $s0, $s0, 0x2 /* handwritten instruction */
    /* 3E9EC 8004E1EC 5403C88E */  lw         $t0, 0x354($s6)
    /* 3E9F0 8004E1F0 5803C98E */  lw         $t1, 0x358($s6)
    /* 3E9F4 8004E1F4 5C03CA8E */  lw         $t2, 0x35C($s6)
    /* 3E9F8 8004E1F8 6003CB8E */  lw         $t3, 0x360($s6)
    /* 3E9FC 8004E1FC 6403CC8E */  lw         $t4, 0x364($s6)
    /* 3EA00 8004E200 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3EA04 8004E204 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3EA08 8004E208 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3EA0C 8004E20C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3EA10 8004E210 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3EA14 8004E214 7403CB8E */  lw         $t3, 0x374($s6)
    /* 3EA18 8004E218 7803CC8E */  lw         $t4, 0x378($s6)
    /* 3EA1C 8004E21C 7C03CD8E */  lw         $t5, 0x37C($s6)
    /* 3EA20 8004E220 FFFF6320 */  addi       $v1, $v1, -0x1 /* handwritten instruction */
  .L8004E224:
    /* 3EA24 8004E224 02000896 */  lhu        $t0, 0x2($s0)
    /* 3EA28 8004E228 04000986 */  lh         $t1, 0x4($s0)
    /* 3EA2C 8004E22C 06000A86 */  lh         $t2, 0x6($s0)
    /* 3EA30 8004E230 004C0900 */  sll        $t1, $t1, 16
    /* 3EA34 8004E234 25482801 */  or         $t1, $t1, $t0
    /* 3EA38 8004E238 00000886 */  lh         $t0, 0x0($s0)
    /* 3EA3C 8004E23C 00008948 */  mtc2       $t1, $0 /* handwritten instruction */
    /* 3EA40 8004E240 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 3EA44 8004E244 20400801 */  add        $t0, $t0, $t0 /* handwritten instruction */
    /* 3EA48 8004E248 2402D922 */  addi       $t9, $s6, 0x224 /* handwritten instruction */
    /* 3EA4C 8004E24C 20C82803 */  add        $t9, $t9, $t0 /* handwritten instruction */
    /* 3EA50 8004E250 20400801 */  add        $t0, $t0, $t0 /* handwritten instruction */
    /* 3EA54 8004E254 20C0C802 */  add        $t8, $s6, $t0 /* handwritten instruction */
    /* 3EA58 8004E258 00000887 */  lh         $t0, 0x0($t8)
    /* 3EA5C 8004E25C 02000987 */  lh         $t1, 0x2($t8)
    /* 3EA60 8004E260 00002A87 */  lh         $t2, 0x0($t9)
    /* 3EA64 8004E264 20400B01 */  add        $t0, $t0, $t3 /* handwritten instruction */
    /* 3EA68 8004E268 20482C01 */  add        $t1, $t1, $t4 /* handwritten instruction */
    /* 3EA6C 8004E26C 20504D01 */  add        $t2, $t2, $t5 /* handwritten instruction */
    /* 3EA70 8004E270 0028C848 */  ctc2       $t0, $5 /* handwritten instruction */
    /* 3EA74 8004E274 0030C948 */  ctc2       $t1, $6 /* handwritten instruction */
    /* 3EA78 8004E278 0038CA48 */  ctc2       $t2, $7 /* handwritten instruction */
    /* 3EA7C 8004E27C 00000000 */  nop
    /* 3EA80 8004E280 00000000 */  nop
    /* 3EA84 8004E284 0100184A */  rtps
    /* 3EA88 8004E288 08001022 */  addi       $s0, $s0, 0x8 /* handwritten instruction */
    /* 3EA8C 8004E28C 00980848 */  mfc2       $t0, $19 /* handwritten instruction */
    /* 3EA90 8004E290 00000EEB */  swc2       $14, 0x0($t8)
    /* 3EA94 8004E294 000028A7 */  sh         $t0, 0x0($t9)
    /* 3EA98 8004E298 E2FF6014 */  bnez       $v1, .L8004E224
    /* 3EA9C 8004E29C FFFF6320 */   addi      $v1, $v1, -0x1 /* handwritten instruction */
  .L8004E2A0:
    /* 3EAA0 8004E2A0 00000386 */  lh         $v1, 0x0($s0)
    /* 3EAA4 8004E2A4 00000000 */  nop
    /* 3EAA8 8004E2A8 5F006010 */  beqz       $v1, .L8004E428
    /* 3EAAC 8004E2AC 02001022 */   addi      $s0, $s0, 0x2 /* handwritten instruction */
    /* 3EAB0 8004E2B0 5403C88E */  lw         $t0, 0x354($s6)
    /* 3EAB4 8004E2B4 5803C98E */  lw         $t1, 0x358($s6)
    /* 3EAB8 8004E2B8 5C03CA8E */  lw         $t2, 0x35C($s6)
    /* 3EABC 8004E2BC 6003CB8E */  lw         $t3, 0x360($s6)
    /* 3EAC0 8004E2C0 6403CC8E */  lw         $t4, 0x364($s6)
    /* 3EAC4 8004E2C4 7403CD8E */  lw         $t5, 0x374($s6)
    /* 3EAC8 8004E2C8 7803CE8E */  lw         $t6, 0x378($s6)
    /* 3EACC 8004E2CC 7C03CF8E */  lw         $t7, 0x37C($s6)
    /* 3EAD0 8004E2D0 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3EAD4 8004E2D4 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3EAD8 8004E2D8 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3EADC 8004E2DC 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3EAE0 8004E2E0 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3EAE4 8004E2E4 0028CD48 */  ctc2       $t5, $5 /* handwritten instruction */
    /* 3EAE8 8004E2E8 0030CE48 */  ctc2       $t6, $6 /* handwritten instruction */
    /* 3EAEC 8004E2EC 0038CF48 */  ctc2       $t7, $7 /* handwritten instruction */
    /* 3EAF0 8004E2F0 FDFF6324 */  addiu      $v1, $v1, -0x3
    /* 3EAF4 8004E2F4 00000896 */  lhu        $t0, 0x0($s0)
    /* 3EAF8 8004E2F8 02000986 */  lh         $t1, 0x2($s0)
    /* 3EAFC 8004E2FC 04000A86 */  lh         $t2, 0x4($s0)
    /* 3EB00 8004E300 004C0900 */  sll        $t1, $t1, 16
    /* 3EB04 8004E304 25482801 */  or         $t1, $t1, $t0
    /* 3EB08 8004E308 00008948 */  mtc2       $t1, $0 /* handwritten instruction */
    /* 3EB0C 8004E30C 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 3EB10 8004E310 06000896 */  lhu        $t0, 0x6($s0)
    /* 3EB14 8004E314 08000986 */  lh         $t1, 0x8($s0)
    /* 3EB18 8004E318 0A000A86 */  lh         $t2, 0xA($s0)
    /* 3EB1C 8004E31C 004C0900 */  sll        $t1, $t1, 16
    /* 3EB20 8004E320 25482801 */  or         $t1, $t1, $t0
    /* 3EB24 8004E324 00108948 */  mtc2       $t1, $2 /* handwritten instruction */
    /* 3EB28 8004E328 00188A48 */  mtc2       $t2, $3 /* handwritten instruction */
    /* 3EB2C 8004E32C 0C000896 */  lhu        $t0, 0xC($s0)
    /* 3EB30 8004E330 0E000986 */  lh         $t1, 0xE($s0)
    /* 3EB34 8004E334 10000A86 */  lh         $t2, 0x10($s0)
    /* 3EB38 8004E338 004C0900 */  sll        $t1, $t1, 16
    /* 3EB3C 8004E33C 25482801 */  or         $t1, $t1, $t0
    /* 3EB40 8004E340 00208948 */  mtc2       $t1, $4 /* handwritten instruction */
    /* 3EB44 8004E344 00288A48 */  mtc2       $t2, $5 /* handwritten instruction */
    /* 3EB48 8004E348 00000000 */  nop
    /* 3EB4C 8004E34C 00000000 */  nop
    /* 3EB50 8004E350 3000284A */  rtpt
    /* 3EB54 8004E354 DC380108 */  j          .L8004E370
    /* 3EB58 8004E358 12001026 */   addiu     $s0, $s0, 0x12
  .L8004E35C:
    /* 3EB5C 8004E35C 3000284A */  rtpt
    /* 3EB60 8004E360 000088A6 */  sh         $t0, 0x0($s4)
    /* 3EB64 8004E364 020089A6 */  sh         $t1, 0x2($s4)
    /* 3EB68 8004E368 04008AA6 */  sh         $t2, 0x4($s4)
    /* 3EB6C 8004E36C 06009426 */  addiu      $s4, $s4, 0x6
  .L8004E370:
    /* 3EB70 8004E370 20006018 */  blez       $v1, .L8004E3F4
    /* 3EB74 8004E374 00000000 */   nop
    /* 3EB78 8004E378 FDFF6324 */  addiu      $v1, $v1, -0x3
    /* 3EB7C 8004E37C 00000896 */  lhu        $t0, 0x0($s0)
    /* 3EB80 8004E380 02000A86 */  lh         $t2, 0x2($s0)
    /* 3EB84 8004E384 04000986 */  lh         $t1, 0x4($s0)
    /* 3EB88 8004E388 00540A00 */  sll        $t2, $t2, 16
    /* 3EB8C 8004E38C 25400A01 */  or         $t0, $t0, $t2
    /* 3EB90 8004E390 06000A96 */  lhu        $t2, 0x6($s0)
    /* 3EB94 8004E394 08000C86 */  lh         $t4, 0x8($s0)
    /* 3EB98 8004E398 0A000B86 */  lh         $t3, 0xA($s0)
    /* 3EB9C 8004E39C 00640C00 */  sll        $t4, $t4, 16
    /* 3EBA0 8004E3A0 25504C01 */  or         $t2, $t2, $t4
    /* 3EBA4 8004E3A4 0C000C96 */  lhu        $t4, 0xC($s0)
    /* 3EBA8 8004E3A8 0E000E86 */  lh         $t6, 0xE($s0)
    /* 3EBAC 8004E3AC 10000D86 */  lh         $t5, 0x10($s0)
    /* 3EBB0 8004E3B0 00740E00 */  sll        $t6, $t6, 16
    /* 3EBB4 8004E3B4 25608E01 */  or         $t4, $t4, $t6
    /* 3EBB8 8004E3B8 00006CEA */  swc2       $12, 0x0($s3)
    /* 3EBBC 8004E3BC 04006DEA */  swc2       $13, 0x4($s3)
    /* 3EBC0 8004E3C0 08006EEA */  swc2       $14, 0x8($s3)
    /* 3EBC4 8004E3C4 0C007326 */  addiu      $s3, $s3, 0xC
    /* 3EBC8 8004E3C8 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 3EBCC 8004E3CC 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 3EBD0 8004E3D0 00108A48 */  mtc2       $t2, $2 /* handwritten instruction */
    /* 3EBD4 8004E3D4 00188B48 */  mtc2       $t3, $3 /* handwritten instruction */
    /* 3EBD8 8004E3D8 00208C48 */  mtc2       $t4, $4 /* handwritten instruction */
    /* 3EBDC 8004E3DC 00288D48 */  mtc2       $t5, $5 /* handwritten instruction */
    /* 3EBE0 8004E3E0 00880848 */  mfc2       $t0, $17 /* handwritten instruction */
    /* 3EBE4 8004E3E4 00900948 */  mfc2       $t1, $18 /* handwritten instruction */
    /* 3EBE8 8004E3E8 00980A48 */  mfc2       $t2, $19 /* handwritten instruction */
    /* 3EBEC 8004E3EC D7380108 */  j          .L8004E35C
    /* 3EBF0 8004E3F0 12001026 */   addiu     $s0, $s0, 0x12
  .L8004E3F4:
    /* 3EBF4 8004E3F4 00006CEA */  swc2       $12, 0x0($s3)
    /* 3EBF8 8004E3F8 04006DEA */  swc2       $13, 0x4($s3)
    /* 3EBFC 8004E3FC 08006EEA */  swc2       $14, 0x8($s3)
    /* 3EC00 8004E400 00880848 */  mfc2       $t0, $17 /* handwritten instruction */
    /* 3EC04 8004E404 00900948 */  mfc2       $t1, $18 /* handwritten instruction */
    /* 3EC08 8004E408 00980A48 */  mfc2       $t2, $19 /* handwritten instruction */
    /* 3EC0C 8004E40C 000088A6 */  sh         $t0, 0x0($s4)
    /* 3EC10 8004E410 020089A6 */  sh         $t1, 0x2($s4)
    /* 3EC14 8004E414 04008AA6 */  sh         $t2, 0x4($s4)
    /* 3EC18 8004E418 20186300 */  add        $v1, $v1, $v1 /* handwritten instruction */
    /* 3EC1C 8004E41C 20406300 */  add        $t0, $v1, $v1 /* handwritten instruction */
    /* 3EC20 8004E420 20180301 */  add        $v1, $t0, $v1 /* handwritten instruction */
    /* 3EC24 8004E424 20800302 */  add        $s0, $s0, $v1 /* handwritten instruction */
  .L8004E428:
    /* 3EC28 8004E428 02001022 */  addi       $s0, $s0, 0x2 /* handwritten instruction */
    /* 3EC2C 8004E42C 04000924 */  addiu      $t1, $zero, 0x4
    /* 3EC30 8004E430 03000832 */  andi       $t0, $s0, 0x3
    /* 3EC34 8004E434 22402801 */  sub        $t0, $t1, $t0 /* handwritten instruction */
    /* 3EC38 8004E438 03000832 */  andi       $t0, $s0, 0x3
    /* 3EC3C 8004E43C 20800802 */  add        $s0, $s0, $t0 /* handwritten instruction */
    /* 3EC40 8004E440 E0FFD422 */  addi       $s4, $s6, -0x20 /* handwritten instruction */
    /* 3EC44 8004E444 1C00938E */  lw         $s3, 0x1C($s4)
  .L8004E448:
    /* 3EC48 8004E448 00000486 */  lh         $a0, 0x0($s0)
    /* 3EC4C 8004E44C 02001022 */  addi       $s0, $s0, 0x2 /* handwritten instruction */
    /* 3EC50 8004E450 0B008010 */  beqz       $a0, .L8004E480
    /* 3EC54 8004E454 00000896 */   lhu       $t0, 0x0($s0)
    /* 3EC58 8004E458 02001022 */  addi       $s0, $s0, 0x2 /* handwritten instruction */
    /* 3EC5C 8004E45C 80400800 */  sll        $t0, $t0, 2
    /* 3EC60 8004E460 20401301 */  add        $t0, $t0, $s3 /* handwritten instruction */
    /* 3EC64 8004E464 0000088D */  lw         $t0, 0x0($t0)
    /* 3EC68 8004E468 00000000 */  nop
    /* 3EC6C 8004E46C 09F80001 */  jalr       $t0
    /* 3EC70 8004E470 00000000 */   nop
    /* 3EC74 8004E474 00000000 */  nop
    /* 3EC78 8004E478 12390108 */  j          .L8004E448
    /* 3EC7C 8004E47C 00000000 */   nop
  .L8004E480:
    /* 3EC80 8004E480 2A00A012 */  beqz       $s5, .L8004E52C
    /* 3EC84 8004E484 FFFF1234 */   ori       $s2, $zero, 0xFFFF
  .L8004E488:
    /* 3EC88 8004E488 00000896 */  lhu        $t0, 0x0($s0)
    /* 3EC8C 8004E48C 02000996 */  lhu        $t1, 0x2($s0)
    /* 3EC90 8004E490 1B001211 */  beq        $t0, $s2, .L8004E500
    /* 3EC94 8004E494 2160C002 */   addu      $t4, $s6, $zero
    /* 3EC98 8004E498 2402CE22 */  addi       $t6, $s6, 0x224 /* handwritten instruction */
    /* 3EC9C 8004E49C 04000B86 */  lh         $t3, 0x4($s0)
    /* 3ECA0 8004E4A0 21688001 */  addu       $t5, $t4, $zero
    /* 3ECA4 8004E4A4 2178C001 */  addu       $t7, $t6, $zero
    /* 3ECA8 8004E4A8 20400801 */  add        $t0, $t0, $t0 /* handwritten instruction */
    /* 3ECAC 8004E4AC 20482901 */  add        $t1, $t1, $t1 /* handwritten instruction */
    /* 3ECB0 8004E4B0 2070C801 */  add        $t6, $t6, $t0 /* handwritten instruction */
    /* 3ECB4 8004E4B4 2078E901 */  add        $t7, $t7, $t1 /* handwritten instruction */
    /* 3ECB8 8004E4B8 20400801 */  add        $t0, $t0, $t0 /* handwritten instruction */
    /* 3ECBC 8004E4BC 20482901 */  add        $t1, $t1, $t1 /* handwritten instruction */
    /* 3ECC0 8004E4C0 20608801 */  add        $t4, $t4, $t0 /* handwritten instruction */
    /* 3ECC4 8004E4C4 2068A901 */  add        $t5, $t5, $t1 /* handwritten instruction */
    /* 3ECC8 8004E4C8 06001022 */  addi       $s0, $s0, 0x6 /* handwritten instruction */
    /* 3ECCC 8004E4CC FFFF6B21 */  addi       $t3, $t3, -0x1 /* handwritten instruction */
  .L8004E4D0:
    /* 3ECD0 8004E4D0 0000888D */  lw         $t0, 0x0($t4)
    /* 3ECD4 8004E4D4 0000C985 */  lh         $t1, 0x0($t6)
    /* 3ECD8 8004E4D8 0000A8AD */  sw         $t0, 0x0($t5)
    /* 3ECDC 8004E4DC 0000E9A5 */  sh         $t1, 0x0($t7)
    /* 3ECE0 8004E4E0 FCFF8C21 */  addi       $t4, $t4, -0x4 /* handwritten instruction */
    /* 3ECE4 8004E4E4 0400AD21 */  addi       $t5, $t5, 0x4 /* handwritten instruction */
    /* 3ECE8 8004E4E8 FEFFCE21 */  addi       $t6, $t6, -0x2 /* handwritten instruction */
    /* 3ECEC 8004E4EC 0200EF21 */  addi       $t7, $t7, 0x2 /* handwritten instruction */
    /* 3ECF0 8004E4F0 F7FF6015 */  bnez       $t3, .L8004E4D0
    /* 3ECF4 8004E4F4 FFFF6B21 */   addi      $t3, $t3, -0x1 /* handwritten instruction */
    /* 3ECF8 8004E4F8 22390108 */  j          .L8004E488
    /* 3ECFC 8004E4FC 00000000 */   nop
  .L8004E500:
    /* 3ED00 8004E500 02001022 */  addi       $s0, $s0, 0x2 /* handwritten instruction */
    /* 3ED04 8004E504 00000886 */  lh         $t0, 0x0($s0)
    /* 3ED08 8004E508 2198C002 */  addu       $s3, $s6, $zero
    /* 3ED0C 8004E50C 2402D422 */  addi       $s4, $s6, 0x224 /* handwritten instruction */
    /* 3ED10 8004E510 20400801 */  add        $t0, $t0, $t0 /* handwritten instruction */
    /* 3ED14 8004E514 20A08802 */  add        $s4, $s4, $t0 /* handwritten instruction */
    /* 3ED18 8004E518 20400801 */  add        $t0, $t0, $t0 /* handwritten instruction */
    /* 3ED1C 8004E51C 20986802 */  add        $s3, $s3, $t0 /* handwritten instruction */
    /* 3ED20 8004E520 02001022 */  addi       $s0, $s0, 0x2 /* handwritten instruction */
    /* 3ED24 8004E524 3BFE0104 */  bgez       $zero, .L8004DE14
    /* 3ED28 8004E528 FFFFB522 */   addi      $s5, $s5, -0x1 /* handwritten instruction */
  .L8004E52C:
    /* 3ED2C 8004E52C 21100002 */  addu       $v0, $s0, $zero
    /* 3ED30 8004E530 8800BF8F */  lw         $ra, 0x88($sp)
    /* 3ED34 8004E534 8400BE8F */  lw         $fp, 0x84($sp)
    /* 3ED38 8004E538 8000B78F */  lw         $s7, 0x80($sp)
    /* 3ED3C 8004E53C 7C00B68F */  lw         $s6, 0x7C($sp)
    /* 3ED40 8004E540 7800B58F */  lw         $s5, 0x78($sp)
    /* 3ED44 8004E544 7400B48F */  lw         $s4, 0x74($sp)
    /* 3ED48 8004E548 7000B38F */  lw         $s3, 0x70($sp)
    /* 3ED4C 8004E54C 6C00B28F */  lw         $s2, 0x6C($sp)
    /* 3ED50 8004E550 6800B18F */  lw         $s1, 0x68($sp)
    /* 3ED54 8004E554 6400B08F */  lw         $s0, 0x64($sp)
    /* 3ED58 8004E558 0800E003 */  jr         $ra
    /* 3ED5C 8004E55C 8C00BD27 */   addiu     $sp, $sp, 0x8C
endlabel func_8004DDB4
