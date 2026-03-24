glabel func_8004EF10
    /* 3F710 8004EF10 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3F714 8004EF14 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3F718 8004EF18 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3F71C 8004EF1C 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3F720 8004EF20 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3F724 8004EF24 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3F728 8004EF28 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3F72C 8004EF2C 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3F730 8004EF30 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3F734 8004EF34 65008010 */  beqz       $a0, .L8004F0CC
    /* 3F738 8004EF38 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3F73C 8004EF3C 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3F740 8004EF40 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3F744 8004EF44 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3F748 8004EF48 B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3F74C 8004EF4C 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3F750 8004EF50 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3F754 8004EF54 0007153C */  lui        $s5, (0x7000000 >> 16)
    /* 3F758 8004EF58 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3F75C 8004EF5C FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3F760 8004EF60 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3F764 8004EF64 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3F768 8004EF68 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3F76C 8004EF6C 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
  .L8004EF70:
    /* 3F770 8004EF70 0C00048E */  lw         $a0, 0xC($s0)
    /* 3F774 8004EF74 000006CA */  lwc2       $6, 0x0($s0)
    /* 3F778 8004EF78 80280400 */  sll        $a1, $a0, 2
    /* 3F77C 8004EF7C FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3F780 8004EF80 21287101 */  addu       $a1, $t3, $s1
    /* 3F784 8004EF84 0000A88C */  lw         $t0, 0x0($a1)
    /* 3F788 8004EF88 82290400 */  srl        $a1, $a0, 6
    /* 3F78C 8004EF8C FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3F790 8004EF90 21289101 */  addu       $a1, $t4, $s1
    /* 3F794 8004EF94 0000A98C */  lw         $t1, 0x0($a1)
    /* 3F798 8004EF98 822B0400 */  srl        $a1, $a0, 14
    /* 3F79C 8004EF9C FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3F7A0 8004EFA0 2128B101 */  addu       $a1, $t5, $s1
    /* 3F7A4 8004EFA4 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3F7A8 8004EFA8 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3F7AC 8004EFAC 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3F7B0 8004EFB0 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3F7B4 8004EFB4 24700901 */  and        $t6, $t0, $t1
    /* 3F7B8 8004EFB8 2470CA01 */  and        $t6, $t6, $t2
    /* 3F7BC 8004EFBC 0600404B */  nclip
    /* 3F7C0 8004EFC0 3F00C005 */  bltz       $t6, .L8004F0C0
    /* 3F7C4 8004EFC4 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3F7C8 8004EFC8 0080CE31 */  andi       $t6, $t6, 0x8000
    /* 3F7CC 8004EFCC 3C00C015 */  bnez       $t6, .L8004F0C0
    /* 3F7D0 8004EFD0 2A384000 */   slt       $a3, $v0, $zero
    /* 3F7D4 8004EFD4 FFFFE720 */  addi       $a3, $a3, -0x1 /* handwritten instruction */
    /* 3F7D8 8004EFD8 42100B00 */  srl        $v0, $t3, 1
    /* 3F7DC 8004EFDC 21105200 */  addu       $v0, $v0, $s2
    /* 3F7E0 8004EFE0 00004484 */  lh         $a0, 0x0($v0)
    /* 3F7E4 8004EFE4 42100C00 */  srl        $v0, $t4, 1
    /* 3F7E8 8004EFE8 21105200 */  addu       $v0, $v0, $s2
    /* 3F7EC 8004EFEC 00004584 */  lh         $a1, 0x0($v0)
    /* 3F7F0 8004EFF0 42100D00 */  srl        $v0, $t5, 1
    /* 3F7F4 8004EFF4 21105200 */  addu       $v0, $v0, $s2
    /* 3F7F8 8004EFF8 00004684 */  lh         $a2, 0x0($v0)
    /* 3F7FC 8004EFFC 00888448 */  mtc2       $a0, $17 /* handwritten instruction */
    /* 3F800 8004F000 00908548 */  mtc2       $a1, $18 /* handwritten instruction */
    /* 3F804 8004F004 00988648 */  mtc2       $a2, $19 /* handwritten instruction */
    /* 3F808 8004F008 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3F80C 8004F00C 00000000 */  nop
    /* 3F810 8004F010 2D00584B */  avsz3
    /* 3F814 8004F014 1000C9AE */  sw         $t1, 0x10($s6)
    /* 3F818 8004F018 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 3F81C 8004F01C 1800CAAE */  sw         $t2, 0x18($s6)
    /* 3F820 8004F020 27004018 */  blez       $v0, .L8004F0C0
    /* 3F824 8004F024 80100200 */   sll       $v0, $v0, 2
    /* 3F828 8004F028 0100013C */  lui        $at, (0x10000 >> 16)
    /* 3F82C 8004F02C 2AC84100 */  slt        $t9, $v0, $at
    /* 3F830 8004F030 23002013 */  beqz       $t9, .L8004F0C0
    /* 3F834 8004F034 00000000 */   nop
    /* 3F838 8004F038 1000088E */  lw         $t0, 0x10($s0)
    /* 3F83C 8004F03C 1400098E */  lw         $t1, 0x14($s0)
    /* 3F840 8004F040 26400701 */  xor        $t0, $t0, $a3
    /* 3F844 8004F044 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 3F848 8004F048 26482701 */  xor        $t1, $t1, $a3
    /* 3F84C 8004F04C 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 3F850 8004F050 06106200 */  srlv       $v0, $v0, $v1
    /* 3F854 8004F054 C2420200 */  srl        $t0, $v0, 11
    /* 3F858 8004F058 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3F85C 8004F05C 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3F860 8004F060 07100201 */  srav       $v0, $v0, $t0
    /* 3F864 8004F064 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3F868 8004F068 1E04C84A */  ncs
    /* 3F86C 8004F06C 04000C86 */  lh         $t4, 0x4($s0)
    /* 3F870 8004F070 00000D86 */  lh         $t5, 0x0($s0)
    /* 3F874 8004F074 00640C00 */  sll        $t4, $t4, 16
    /* 3F878 8004F078 006C0D00 */  sll        $t5, $t5, 16
    /* 3F87C 8004F07C 0400D6EA */  swc2       $22, 0x4($s6)
    /* 3F880 8004F080 06000896 */  lhu        $t0, 0x6($s0)
    /* 3F884 8004F084 08000996 */  lhu        $t1, 0x8($s0)
    /* 3F888 8004F088 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3F88C 8004F08C 25400C01 */  or         $t0, $t0, $t4
    /* 3F890 8004F090 25482D01 */  or         $t1, $t1, $t5
    /* 3F894 8004F094 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3F898 8004F098 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3F89C 8004F09C 1C00CAA6 */  sh         $t2, 0x1C($s6)
    /* 3F8A0 8004F0A0 80100200 */  sll        $v0, $v0, 2
    /* 3F8A4 8004F0A4 21105400 */  addu       $v0, $v0, $s4
    /* 3F8A8 8004F0A8 0000498C */  lw         $t1, 0x0($v0)
    /* 3F8AC 8004F0AC 2440DE02 */  and        $t0, $s6, $fp
    /* 3F8B0 8004F0B0 000048AC */  sw         $t0, 0x0($v0)
    /* 3F8B4 8004F0B4 25483501 */  or         $t1, $t1, $s5
    /* 3F8B8 8004F0B8 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3F8BC 8004F0BC 2000D626 */  addiu      $s6, $s6, 0x20
  .L8004F0C0:
    /* 3F8C0 8004F0C0 18001026 */  addiu      $s0, $s0, 0x18
    /* 3F8C4 8004F0C4 AAFF6016 */  bnez       $s3, .L8004EF70
    /* 3F8C8 8004F0C8 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004F0CC:
    /* 3F8CC 8004F0CC 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3F8D0 8004F0D0 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3F8D4 8004F0D4 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3F8D8 8004F0D8 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3F8DC 8004F0DC 2400B68F */  lw         $s6, 0x24($sp)
    /* 3F8E0 8004F0E0 2000B58F */  lw         $s5, 0x20($sp)
    /* 3F8E4 8004F0E4 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3F8E8 8004F0E8 1800B38F */  lw         $s3, 0x18($sp)
    /* 3F8EC 8004F0EC 1400B28F */  lw         $s2, 0x14($sp)
    /* 3F8F0 8004F0F0 1000B18F */  lw         $s1, 0x10($sp)
    /* 3F8F4 8004F0F4 0800E003 */  jr         $ra
    /* 3F8F8 8004F0F8 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004EF10
