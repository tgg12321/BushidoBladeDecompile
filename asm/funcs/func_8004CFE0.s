glabel func_8004CFE0
    /* 3D7E0 8004CFE0 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3D7E4 8004CFE4 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3D7E8 8004CFE8 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3D7EC 8004CFEC 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3D7F0 8004CFF0 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3D7F4 8004CFF4 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3D7F8 8004CFF8 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3D7FC 8004CFFC 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3D800 8004D000 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3D804 8004D004 83008010 */  beqz       $a0, .L8004D214
    /* 3D808 8004D008 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3D80C 8004D00C 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3D810 8004D010 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3D814 8004D014 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3D818 8004D018 B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3D81C 8004D01C 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3D820 8004D020 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3D824 8004D024 000C153C */  lui        $s5, (0xC000000 >> 16)
    /* 3D828 8004D028 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3D82C 8004D02C FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3D830 8004D030 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3D834 8004D034 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3D838 8004D038 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3D83C 8004D03C 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
    /* 3D840 8004D040 000006CA */  lwc2       $6, 0x0($s0)
  .L8004D044:
    /* 3D844 8004D044 1000048E */  lw         $a0, 0x10($s0)
    /* 3D848 8004D048 00000000 */  nop
    /* 3D84C 8004D04C 80280400 */  sll        $a1, $a0, 2
    /* 3D850 8004D050 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3D854 8004D054 21287101 */  addu       $a1, $t3, $s1
    /* 3D858 8004D058 0000A88C */  lw         $t0, 0x0($a1)
    /* 3D85C 8004D05C 82290400 */  srl        $a1, $a0, 6
    /* 3D860 8004D060 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3D864 8004D064 21289101 */  addu       $a1, $t4, $s1
    /* 3D868 8004D068 0000A98C */  lw         $t1, 0x0($a1)
    /* 3D86C 8004D06C 822B0400 */  srl        $a1, $a0, 14
    /* 3D870 8004D070 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3D874 8004D074 2128B101 */  addu       $a1, $t5, $s1
    /* 3D878 8004D078 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3D87C 8004D07C 822D0400 */  srl        $a1, $a0, 22
    /* 3D880 8004D080 FC03AE30 */  andi       $t6, $a1, 0x3FC
    /* 3D884 8004D084 2128D101 */  addu       $a1, $t6, $s1
    /* 3D888 8004D088 0000AF8C */  lw         $t7, 0x0($a1)
    /* 3D88C 8004D08C 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3D890 8004D090 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3D894 8004D094 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3D898 8004D098 24C00901 */  and        $t8, $t0, $t1
    /* 3D89C 8004D09C 24C00A03 */  and        $t8, $t8, $t2
    /* 3D8A0 8004D0A0 24C00F03 */  and        $t8, $t8, $t7
    /* 3D8A4 8004D0A4 0600404B */  nclip
    /* 3D8A8 8004D0A8 57000007 */  bltz       $t8, .L8004D208
    /* 3D8AC 8004D0AC 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3D8B0 8004D0B0 00801833 */  andi       $t8, $t8, 0x8000
    /* 3D8B4 8004D0B4 54000017 */  bnez       $t8, .L8004D208
    /* 3D8B8 8004D0B8 2A384000 */   slt       $a3, $v0, $zero
    /* 3D8BC 8004D0BC FFFFE720 */  addi       $a3, $a3, -0x1 /* handwritten instruction */
    /* 3D8C0 8004D0C0 42100B00 */  srl        $v0, $t3, 1
    /* 3D8C4 8004D0C4 21105200 */  addu       $v0, $v0, $s2
    /* 3D8C8 8004D0C8 00004484 */  lh         $a0, 0x0($v0)
    /* 3D8CC 8004D0CC 42100C00 */  srl        $v0, $t4, 1
    /* 3D8D0 8004D0D0 21105200 */  addu       $v0, $v0, $s2
    /* 3D8D4 8004D0D4 00004584 */  lh         $a1, 0x0($v0)
    /* 3D8D8 8004D0D8 42100D00 */  srl        $v0, $t5, 1
    /* 3D8DC 8004D0DC 21105200 */  addu       $v0, $v0, $s2
    /* 3D8E0 8004D0E0 00004684 */  lh         $a2, 0x0($v0)
    /* 3D8E4 8004D0E4 42100E00 */  srl        $v0, $t6, 1
    /* 3D8E8 8004D0E8 21105200 */  addu       $v0, $v0, $s2
    /* 3D8EC 8004D0EC 00004B84 */  lh         $t3, 0x0($v0)
    /* 3D8F0 8004D0F0 20208500 */  add        $a0, $a0, $a1 /* handwritten instruction */
    /* 3D8F4 8004D0F4 20208600 */  add        $a0, $a0, $a2 /* handwritten instruction */
    /* 3D8F8 8004D0F8 20208B00 */  add        $a0, $a0, $t3 /* handwritten instruction */
    /* 3D8FC 8004D0FC 83100400 */  sra        $v0, $a0, 2
    /* 3D900 8004D100 41004018 */  blez       $v0, .L8004D208
    /* 3D904 8004D104 0100013C */   lui       $at, (0x10000 >> 16)
    /* 3D908 8004D108 2AC84100 */  slt        $t9, $v0, $at
    /* 3D90C 8004D10C 3E002013 */  beqz       $t9, .L8004D208
    /* 3D910 8004D110 00000000 */   nop
    /* 3D914 8004D114 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3D918 8004D118 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3D91C 8004D11C 2000CAAE */  sw         $t2, 0x20($s6)
    /* 3D920 8004D120 2C00CFAE */  sw         $t7, 0x2C($s6)
    /* 3D924 8004D124 14000A8E */  lw         $t2, 0x14($s0)
    /* 3D928 8004D128 1800098E */  lw         $t1, 0x18($s0)
    /* 3D92C 8004D12C 26504701 */  xor        $t2, $t2, $a3
    /* 3D930 8004D130 26482701 */  xor        $t1, $t1, $a3
    /* 3D934 8004D134 00008A48 */  mtc2       $t2, $0 /* handwritten instruction */
    /* 3D938 8004D138 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 3D93C 8004D13C B0FF0124 */  addiu      $at, $zero, -0x50
    /* 3D940 8004D140 2640E100 */  xor        $t0, $a3, $at
    /* 3D944 8004D144 20104800 */  add        $v0, $v0, $t0 /* handwritten instruction */
    /* 3D948 8004D148 06106200 */  srlv       $v0, $v0, $v1
    /* 3D94C 8004D14C C2420200 */  srl        $t0, $v0, 11
    /* 3D950 8004D150 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3D954 8004D154 1E04C84A */  ncs
    /* 3D958 8004D158 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3D95C 8004D15C 07100201 */  srav       $v0, $v0, $t0
    /* 3D960 8004D160 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3D964 8004D164 0400D6EA */  swc2       $22, 0x4($s6)
    /* 3D968 8004D168 1C00088E */  lw         $t0, 0x1C($s0)
    /* 3D96C 8004D16C 024C0900 */  srl        $t1, $t1, 16
    /* 3D970 8004D170 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 3D974 8004D174 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 3D978 8004D178 20000A8E */  lw         $t2, 0x20($s0)
    /* 3D97C 8004D17C 2400098E */  lw         $t1, 0x24($s0)
    /* 3D980 8004D180 26504701 */  xor        $t2, $t2, $a3
    /* 3D984 8004D184 26482701 */  xor        $t1, $t1, $a3
    /* 3D988 8004D188 00108A48 */  mtc2       $t2, $2 /* handwritten instruction */
    /* 3D98C 8004D18C 00188948 */  mtc2       $t1, $3 /* handwritten instruction */
    /* 3D990 8004D190 2800088E */  lw         $t0, 0x28($s0)
    /* 3D994 8004D194 024C0900 */  srl        $t1, $t1, 16
    /* 3D998 8004D198 00208848 */  mtc2       $t0, $4 /* handwritten instruction */
    /* 3D99C 8004D19C 00288948 */  mtc2       $t1, $5 /* handwritten instruction */
    /* 3D9A0 8004D1A0 04000C86 */  lh         $t4, 0x4($s0)
    /* 3D9A4 8004D1A4 00000D86 */  lh         $t5, 0x0($s0)
    /* 3D9A8 8004D1A8 00640C00 */  sll        $t4, $t4, 16
    /* 3D9AC 8004D1AC 006C0D00 */  sll        $t5, $t5, 16
    /* 3D9B0 8004D1B0 2004D84A */  nct
    /* 3D9B4 8004D1B4 06000896 */  lhu        $t0, 0x6($s0)
    /* 3D9B8 8004D1B8 08000996 */  lhu        $t1, 0x8($s0)
    /* 3D9BC 8004D1BC 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3D9C0 8004D1C0 0C000B96 */  lhu        $t3, 0xC($s0)
    /* 3D9C4 8004D1C4 25400C01 */  or         $t0, $t0, $t4
    /* 3D9C8 8004D1C8 25482D01 */  or         $t1, $t1, $t5
    /* 3D9CC 8004D1CC 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3D9D0 8004D1D0 1800C9AE */  sw         $t1, 0x18($s6)
    /* 3D9D4 8004D1D4 2400CAA6 */  sh         $t2, 0x24($s6)
    /* 3D9D8 8004D1D8 3000CBA6 */  sh         $t3, 0x30($s6)
    /* 3D9DC 8004D1DC 80100200 */  sll        $v0, $v0, 2
    /* 3D9E0 8004D1E0 21105400 */  addu       $v0, $v0, $s4
    /* 3D9E4 8004D1E4 0000498C */  lw         $t1, 0x0($v0)
    /* 3D9E8 8004D1E8 2440DE02 */  and        $t0, $s6, $fp
    /* 3D9EC 8004D1EC 000048AC */  sw         $t0, 0x0($v0)
    /* 3D9F0 8004D1F0 25483501 */  or         $t1, $t1, $s5
    /* 3D9F4 8004D1F4 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3D9F8 8004D1F8 1000D4EA */  swc2       $20, 0x10($s6)
    /* 3D9FC 8004D1FC 1C00D5EA */  swc2       $21, 0x1C($s6)
    /* 3DA00 8004D200 2800D6EA */  swc2       $22, 0x28($s6)
    /* 3DA04 8004D204 3400D626 */  addiu      $s6, $s6, 0x34
  .L8004D208:
    /* 3DA08 8004D208 2C001026 */  addiu      $s0, $s0, 0x2C
    /* 3DA0C 8004D20C 8DFF6016 */  bnez       $s3, .L8004D044
    /* 3DA10 8004D210 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004D214:
    /* 3DA14 8004D214 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3DA18 8004D218 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3DA1C 8004D21C 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3DA20 8004D220 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3DA24 8004D224 2400B68F */  lw         $s6, 0x24($sp)
    /* 3DA28 8004D228 2000B58F */  lw         $s5, 0x20($sp)
    /* 3DA2C 8004D22C 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3DA30 8004D230 1800B38F */  lw         $s3, 0x18($sp)
    /* 3DA34 8004D234 1400B28F */  lw         $s2, 0x14($sp)
    /* 3DA38 8004D238 1000B18F */  lw         $s1, 0x10($sp)
    /* 3DA3C 8004D23C 0800E003 */  jr         $ra
    /* 3DA40 8004D240 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004CFE0
