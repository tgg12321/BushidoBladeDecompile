glabel func_8004E7E4
    /* 3EFE4 8004E7E4 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3EFE8 8004E7E8 3000BFAF */  sw         $ra, 0x30($sp)
    /* 3EFEC 8004E7EC 2C00BEAF */  sw         $fp, 0x2C($sp)
    /* 3EFF0 8004E7F0 2800B7AF */  sw         $s7, 0x28($sp)
    /* 3EFF4 8004E7F4 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3EFF8 8004E7F8 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3EFFC 8004E7FC 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3F000 8004E800 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3F004 8004E804 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3F008 8004E808 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3F00C 8004E80C A1008010 */  beqz       $a0, .L8004EA94
    /* 3F010 8004E810 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3F014 8004E814 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3F018 8004E818 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3F01C 8004E81C 801F123C */  lui        $s2, (0x1F800244 >> 16)
    /* 3F020 8004E820 44025236 */  ori        $s2, $s2, (0x1F800244 & 0xFFFF)
    /* 3F024 8004E824 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3F028 8004E828 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3F02C 8004E82C 0009153C */  lui        $s5, (0x9000000 >> 16)
    /* 3F030 8004E830 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3F034 8004E834 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3F038 8004E838 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3F03C 8004E83C B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3F040 8004E840 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3F044 8004E844 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
  .L8004E848:
    /* 3F048 8004E848 1000048E */  lw         $a0, 0x10($s0)
    /* 3F04C 8004E84C 080006CA */  lwc2       $6, 0x8($s0)
    /* 3F050 8004E850 80280400 */  sll        $a1, $a0, 2
    /* 3F054 8004E854 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3F058 8004E858 21287101 */  addu       $a1, $t3, $s1
    /* 3F05C 8004E85C 0000A88C */  lw         $t0, 0x0($a1)
    /* 3F060 8004E860 82290400 */  srl        $a1, $a0, 6
    /* 3F064 8004E864 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3F068 8004E868 21289101 */  addu       $a1, $t4, $s1
    /* 3F06C 8004E86C 0000A98C */  lw         $t1, 0x0($a1)
    /* 3F070 8004E870 822B0400 */  srl        $a1, $a0, 14
    /* 3F074 8004E874 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3F078 8004E878 2128B101 */  addu       $a1, $t5, $s1
    /* 3F07C 8004E87C 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3F080 8004E880 822D0400 */  srl        $a1, $a0, 22
    /* 3F084 8004E884 FC03AE30 */  andi       $t6, $a1, 0x3FC
    /* 3F088 8004E888 2128D101 */  addu       $a1, $t6, $s1
    /* 3F08C 8004E88C 0000AF8C */  lw         $t7, 0x0($a1)
    /* 3F090 8004E890 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3F094 8004E894 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3F098 8004E898 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3F09C 8004E89C 24C00901 */  and        $t8, $t0, $t1
    /* 3F0A0 8004E8A0 24C00A03 */  and        $t8, $t8, $t2
    /* 3F0A4 8004E8A4 24C00F03 */  and        $t8, $t8, $t7
    /* 3F0A8 8004E8A8 0600404B */  nclip
    /* 3F0AC 8004E8AC 76000007 */  bltz       $t8, .L8004EA88
    /* 3F0B0 8004E8B0 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3F0B4 8004E8B4 00801833 */  andi       $t8, $t8, 0x8000
    /* 3F0B8 8004E8B8 73004104 */  bgez       $v0, .L8004EA88
    /* 3F0BC 8004E8BC 00000000 */   nop
    /* 3F0C0 8004E8C0 71000017 */  bnez       $t8, .L8004EA88
    /* 3F0C4 8004E8C4 42100B00 */   srl       $v0, $t3, 1
    /* 3F0C8 8004E8C8 21105200 */  addu       $v0, $v0, $s2
    /* 3F0CC 8004E8CC 00004484 */  lh         $a0, 0x0($v0)
    /* 3F0D0 8004E8D0 42100C00 */  srl        $v0, $t4, 1
    /* 3F0D4 8004E8D4 21105200 */  addu       $v0, $v0, $s2
    /* 3F0D8 8004E8D8 00004584 */  lh         $a1, 0x0($v0)
    /* 3F0DC 8004E8DC 42100D00 */  srl        $v0, $t5, 1
    /* 3F0E0 8004E8E0 21105200 */  addu       $v0, $v0, $s2
    /* 3F0E4 8004E8E4 00004684 */  lh         $a2, 0x0($v0)
    /* 3F0E8 8004E8E8 42100E00 */  srl        $v0, $t6, 1
    /* 3F0EC 8004E8EC 21105200 */  addu       $v0, $v0, $s2
    /* 3F0F0 8004E8F0 00004784 */  lh         $a3, 0x0($v0)
    /* 3F0F4 8004E8F4 20208500 */  add        $a0, $a0, $a1 /* handwritten instruction */
    /* 3F0F8 8004E8F8 20208600 */  add        $a0, $a0, $a2 /* handwritten instruction */
    /* 3F0FC 8004E8FC 20208700 */  add        $a0, $a0, $a3 /* handwritten instruction */
    /* 3F100 8004E900 83100400 */  sra        $v0, $a0, 2
    /* 3F104 8004E904 60004018 */  blez       $v0, .L8004EA88
    /* 3F108 8004E908 00000000 */   nop
    /* 3F10C 8004E90C 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3F110 8004E910 1000C9AE */  sw         $t1, 0x10($s6)
    /* 3F114 8004E914 1800CAAE */  sw         $t2, 0x18($s6)
    /* 3F118 8004E918 2000CFAE */  sw         $t7, 0x20($s6)
    /* 3F11C 8004E91C 1400068E */  lw         $a2, 0x14($s0)
    /* 3F120 8004E920 0E000786 */  lh         $a3, 0xE($s0)
    /* 3F124 8004E924 2120C000 */  addu       $a0, $a2, $zero
    /* 3F128 8004E928 FF038430 */  andi       $a0, $a0, 0x3FF
    /* 3F12C 8004E92C 82320600 */  srl        $a2, $a2, 10
    /* 3F130 8004E930 2128C000 */  addu       $a1, $a2, $zero
    /* 3F134 8004E934 FF03A530 */  andi       $a1, $a1, 0x3FF
    /* 3F138 8004E938 82320600 */  srl        $a2, $a2, 10
    /* 3F13C 8004E93C FF03C630 */  andi       $a2, $a2, 0x3FF
    /* 3F140 8004E940 00040824 */  addiu      $t0, $zero, 0x400
    /* 3F144 8004E944 00408848 */  mtc2       $t0, $8 /* handwritten instruction */
    /* 3F148 8004E948 0F80083C */  lui        $t0, %hi(D_800F2B70)
    /* 3F14C 8004E94C 702B0825 */  addiu      $t0, $t0, %lo(D_800F2B70)
    /* 3F150 8004E950 80200400 */  sll        $a0, $a0, 2
    /* 3F154 8004E954 20208800 */  add        $a0, $a0, $t0 /* handwritten instruction */
    /* 3F158 8004E958 00008C8C */  lw         $t4, 0x0($a0)
    /* 3F15C 8004E95C 00000000 */  nop
    /* 3F160 8004E960 02510C00 */  srl        $t2, $t4, 4
    /* 3F164 8004E964 025B0C00 */  srl        $t3, $t4, 12
    /* 3F168 8004E968 00490C00 */  sll        $t1, $t4, 4
    /* 3F16C 8004E96C F00F2931 */  andi       $t1, $t1, 0xFF0
    /* 3F170 8004E970 F00F4A31 */  andi       $t2, $t2, 0xFF0
    /* 3F174 8004E974 F00F6B31 */  andi       $t3, $t3, 0xFF0
    /* 3F178 8004E978 00488948 */  mtc2       $t1, $9 /* handwritten instruction */
    /* 3F17C 8004E97C 00508A48 */  mtc2       $t2, $10 /* handwritten instruction */
    /* 3F180 8004E980 00588B48 */  mtc2       $t3, $11 /* handwritten instruction */
    /* 3F184 8004E984 80280500 */  sll        $a1, $a1, 2
    /* 3F188 8004E988 2028A800 */  add        $a1, $a1, $t0 /* handwritten instruction */
    /* 3F18C 8004E98C 0000AC8C */  lw         $t4, 0x0($a1)
    /* 3F190 8004E990 3D00984B */  gpf        1
    /* 3F194 8004E994 02510C00 */  srl        $t2, $t4, 4
    /* 3F198 8004E998 025B0C00 */  srl        $t3, $t4, 12
    /* 3F19C 8004E99C 00490C00 */  sll        $t1, $t4, 4
    /* 3F1A0 8004E9A0 F00F2931 */  andi       $t1, $t1, 0xFF0
    /* 3F1A4 8004E9A4 F00F4A31 */  andi       $t2, $t2, 0xFF0
    /* 3F1A8 8004E9A8 F00F6B31 */  andi       $t3, $t3, 0xFF0
    /* 3F1AC 8004E9AC 00488948 */  mtc2       $t1, $9 /* handwritten instruction */
    /* 3F1B0 8004E9B0 00508A48 */  mtc2       $t2, $10 /* handwritten instruction */
    /* 3F1B4 8004E9B4 00588B48 */  mtc2       $t3, $11 /* handwritten instruction */
    /* 3F1B8 8004E9B8 80300600 */  sll        $a2, $a2, 2
    /* 3F1BC 8004E9BC 2030C800 */  add        $a2, $a2, $t0 /* handwritten instruction */
    /* 3F1C0 8004E9C0 0000CC8C */  lw         $t4, 0x0($a2)
    /* 3F1C4 8004E9C4 3E00A84B */  gpl        1
    /* 3F1C8 8004E9C8 02510C00 */  srl        $t2, $t4, 4
    /* 3F1CC 8004E9CC 025B0C00 */  srl        $t3, $t4, 12
    /* 3F1D0 8004E9D0 00490C00 */  sll        $t1, $t4, 4
    /* 3F1D4 8004E9D4 F00F2931 */  andi       $t1, $t1, 0xFF0
    /* 3F1D8 8004E9D8 F00F4A31 */  andi       $t2, $t2, 0xFF0
    /* 3F1DC 8004E9DC F00F6B31 */  andi       $t3, $t3, 0xFF0
    /* 3F1E0 8004E9E0 00488948 */  mtc2       $t1, $9 /* handwritten instruction */
    /* 3F1E4 8004E9E4 00508A48 */  mtc2       $t2, $10 /* handwritten instruction */
    /* 3F1E8 8004E9E8 00588B48 */  mtc2       $t3, $11 /* handwritten instruction */
    /* 3F1EC 8004E9EC 80380700 */  sll        $a3, $a3, 2
    /* 3F1F0 8004E9F0 2038E800 */  add        $a3, $a3, $t0 /* handwritten instruction */
    /* 3F1F4 8004E9F4 0000EC8C */  lw         $t4, 0x0($a3)
    /* 3F1F8 8004E9F8 3E00A84B */  gpl        1
    /* 3F1FC 8004E9FC 02510C00 */  srl        $t2, $t4, 4
    /* 3F200 8004EA00 025B0C00 */  srl        $t3, $t4, 12
    /* 3F204 8004EA04 00490C00 */  sll        $t1, $t4, 4
    /* 3F208 8004EA08 F00F2931 */  andi       $t1, $t1, 0xFF0
    /* 3F20C 8004EA0C F00F4A31 */  andi       $t2, $t2, 0xFF0
    /* 3F210 8004EA10 F00F6B31 */  andi       $t3, $t3, 0xFF0
    /* 3F214 8004EA14 00488948 */  mtc2       $t1, $9 /* handwritten instruction */
    /* 3F218 8004EA18 00508A48 */  mtc2       $t2, $10 /* handwritten instruction */
    /* 3F21C 8004EA1C 00588B48 */  mtc2       $t3, $11 /* handwritten instruction */
    /* 3F220 8004EA20 00000000 */  nop
    /* 3F224 8004EA24 00000000 */  nop
    /* 3F228 8004EA28 3E00A84B */  gpl        1
    /* 3F22C 8004EA2C 06106200 */  srlv       $v0, $v0, $v1
    /* 3F230 8004EA30 C2420200 */  srl        $t0, $v0, 11
    /* 3F234 8004EA34 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3F238 8004EA38 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3F23C 8004EA3C 07100201 */  srav       $v0, $v0, $t0
    /* 3F240 8004EA40 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3F244 8004EA44 0000088E */  lw         $t0, 0x0($s0)
    /* 3F248 8004EA48 0400098E */  lw         $t1, 0x4($s0)
    /* 3F24C 8004EA4C 08000A86 */  lh         $t2, 0x8($s0)
    /* 3F250 8004EA50 0C000B86 */  lh         $t3, 0xC($s0)
    /* 3F254 8004EA54 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3F258 8004EA58 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3F25C 8004EA5C 1C00CAA6 */  sh         $t2, 0x1C($s6)
    /* 3F260 8004EA60 2400CBA6 */  sh         $t3, 0x24($s6)
    /* 3F264 8004EA64 80100200 */  sll        $v0, $v0, 2
    /* 3F268 8004EA68 21105400 */  addu       $v0, $v0, $s4
    /* 3F26C 8004EA6C 0000498C */  lw         $t1, 0x0($v0)
    /* 3F270 8004EA70 2440DE02 */  and        $t0, $s6, $fp
    /* 3F274 8004EA74 000048AC */  sw         $t0, 0x0($v0)
    /* 3F278 8004EA78 25483501 */  or         $t1, $t1, $s5
    /* 3F27C 8004EA7C 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3F280 8004EA80 0400D6EA */  swc2       $22, 0x4($s6)
    /* 3F284 8004EA84 2800D626 */  addiu      $s6, $s6, 0x28
  .L8004EA88:
    /* 3F288 8004EA88 18001026 */  addiu      $s0, $s0, 0x18
    /* 3F28C 8004EA8C 6EFF6016 */  bnez       $s3, .L8004E848
    /* 3F290 8004EA90 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004EA94:
    /* 3F294 8004EA94 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3F298 8004EA98 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3F29C 8004EA9C 3000BF8F */  lw         $ra, 0x30($sp)
    /* 3F2A0 8004EAA0 2C00BE8F */  lw         $fp, 0x2C($sp)
    /* 3F2A4 8004EAA4 2800B78F */  lw         $s7, 0x28($sp)
    /* 3F2A8 8004EAA8 2400B68F */  lw         $s6, 0x24($sp)
    /* 3F2AC 8004EAAC 2000B58F */  lw         $s5, 0x20($sp)
    /* 3F2B0 8004EAB0 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3F2B4 8004EAB4 1800B38F */  lw         $s3, 0x18($sp)
    /* 3F2B8 8004EAB8 1400B28F */  lw         $s2, 0x14($sp)
    /* 3F2BC 8004EABC 1000B18F */  lw         $s1, 0x10($sp)
    /* 3F2C0 8004EAC0 0800E003 */  jr         $ra
    /* 3F2C4 8004EAC4 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004E7E4
