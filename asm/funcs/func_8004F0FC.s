glabel func_8004F0FC
    /* 3F8FC 8004F0FC C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3F900 8004F100 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3F904 8004F104 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3F908 8004F108 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3F90C 8004F10C 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3F910 8004F110 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3F914 8004F114 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3F918 8004F118 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3F91C 8004F11C 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3F920 8004F120 70008010 */  beqz       $a0, .L8004F2E4
    /* 3F924 8004F124 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3F928 8004F128 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3F92C 8004F12C 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3F930 8004F130 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3F934 8004F134 B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3F938 8004F138 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3F93C 8004F13C 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3F940 8004F140 0009153C */  lui        $s5, (0x9000000 >> 16)
    /* 3F944 8004F144 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3F948 8004F148 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3F94C 8004F14C 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3F950 8004F150 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3F954 8004F154 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3F958 8004F158 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
  .L8004F15C:
    /* 3F95C 8004F15C 0E000496 */  lhu        $a0, 0xE($s0)
    /* 3F960 8004F160 10000596 */  lhu        $a1, 0x10($s0)
    /* 3F964 8004F164 000006CA */  lwc2       $6, 0x0($s0)
    /* 3F968 8004F168 002C0500 */  sll        $a1, $a1, 16
    /* 3F96C 8004F16C 25208500 */  or         $a0, $a0, $a1
    /* 3F970 8004F170 80280400 */  sll        $a1, $a0, 2
    /* 3F974 8004F174 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3F978 8004F178 21287101 */  addu       $a1, $t3, $s1
    /* 3F97C 8004F17C 0000A88C */  lw         $t0, 0x0($a1)
    /* 3F980 8004F180 82290400 */  srl        $a1, $a0, 6
    /* 3F984 8004F184 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3F988 8004F188 21289101 */  addu       $a1, $t4, $s1
    /* 3F98C 8004F18C 0000A98C */  lw         $t1, 0x0($a1)
    /* 3F990 8004F190 822B0400 */  srl        $a1, $a0, 14
    /* 3F994 8004F194 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3F998 8004F198 2128B101 */  addu       $a1, $t5, $s1
    /* 3F99C 8004F19C 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3F9A0 8004F1A0 822D0400 */  srl        $a1, $a0, 22
    /* 3F9A4 8004F1A4 FC03AE30 */  andi       $t6, $a1, 0x3FC
    /* 3F9A8 8004F1A8 2128D101 */  addu       $a1, $t6, $s1
    /* 3F9AC 8004F1AC 0000AF8C */  lw         $t7, 0x0($a1)
    /* 3F9B0 8004F1B0 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3F9B4 8004F1B4 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3F9B8 8004F1B8 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3F9BC 8004F1BC 24C00901 */  and        $t8, $t0, $t1
    /* 3F9C0 8004F1C0 24C00A03 */  and        $t8, $t8, $t2
    /* 3F9C4 8004F1C4 24C00F03 */  and        $t8, $t8, $t7
    /* 3F9C8 8004F1C8 0600404B */  nclip
    /* 3F9CC 8004F1CC 42000007 */  bltz       $t8, .L8004F2D8
    /* 3F9D0 8004F1D0 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3F9D4 8004F1D4 00801833 */  andi       $t8, $t8, 0x8000
    /* 3F9D8 8004F1D8 3F000017 */  bnez       $t8, .L8004F2D8
    /* 3F9DC 8004F1DC 2A384000 */   slt       $a3, $v0, $zero
    /* 3F9E0 8004F1E0 FFFFE720 */  addi       $a3, $a3, -0x1 /* handwritten instruction */
    /* 3F9E4 8004F1E4 42100B00 */  srl        $v0, $t3, 1
    /* 3F9E8 8004F1E8 21105200 */  addu       $v0, $v0, $s2
    /* 3F9EC 8004F1EC 00004484 */  lh         $a0, 0x0($v0)
    /* 3F9F0 8004F1F0 42100C00 */  srl        $v0, $t4, 1
    /* 3F9F4 8004F1F4 21105200 */  addu       $v0, $v0, $s2
    /* 3F9F8 8004F1F8 00004584 */  lh         $a1, 0x0($v0)
    /* 3F9FC 8004F1FC 42100D00 */  srl        $v0, $t5, 1
    /* 3FA00 8004F200 21105200 */  addu       $v0, $v0, $s2
    /* 3FA04 8004F204 00004684 */  lh         $a2, 0x0($v0)
    /* 3FA08 8004F208 42100E00 */  srl        $v0, $t6, 1
    /* 3FA0C 8004F20C 21105200 */  addu       $v0, $v0, $s2
    /* 3FA10 8004F210 00004B84 */  lh         $t3, 0x0($v0)
    /* 3FA14 8004F214 20208500 */  add        $a0, $a0, $a1 /* handwritten instruction */
    /* 3FA18 8004F218 20208600 */  add        $a0, $a0, $a2 /* handwritten instruction */
    /* 3FA1C 8004F21C 20208B00 */  add        $a0, $a0, $t3 /* handwritten instruction */
    /* 3FA20 8004F220 83100400 */  sra        $v0, $a0, 2
    /* 3FA24 8004F224 2C004018 */  blez       $v0, .L8004F2D8
    /* 3FA28 8004F228 0100013C */   lui       $at, (0x10000 >> 16)
    /* 3FA2C 8004F22C 2AC84100 */  slt        $t9, $v0, $at
    /* 3FA30 8004F230 29002013 */  beqz       $t9, .L8004F2D8
    /* 3FA34 8004F234 00000000 */   nop
    /* 3FA38 8004F238 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3FA3C 8004F23C 1000C9AE */  sw         $t1, 0x10($s6)
    /* 3FA40 8004F240 1800CAAE */  sw         $t2, 0x18($s6)
    /* 3FA44 8004F244 2000CFAE */  sw         $t7, 0x20($s6)
    /* 3FA48 8004F248 12000896 */  lhu        $t0, 0x12($s0)
    /* 3FA4C 8004F24C 1400098E */  lw         $t1, 0x14($s0)
    /* 3FA50 8004F250 26400701 */  xor        $t0, $t0, $a3
    /* 3FA54 8004F254 00088848 */  mtc2       $t0, $1 /* handwritten instruction */
    /* 3FA58 8004F258 26482701 */  xor        $t1, $t1, $a3
    /* 3FA5C 8004F25C 00008948 */  mtc2       $t1, $0 /* handwritten instruction */
    /* 3FA60 8004F260 06106200 */  srlv       $v0, $v0, $v1
    /* 3FA64 8004F264 C2420200 */  srl        $t0, $v0, 11
    /* 3FA68 8004F268 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3FA6C 8004F26C 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3FA70 8004F270 07100201 */  srav       $v0, $v0, $t0
    /* 3FA74 8004F274 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3FA78 8004F278 1E04C84A */  ncs
    /* 3FA7C 8004F27C 04000C86 */  lh         $t4, 0x4($s0)
    /* 3FA80 8004F280 00000D86 */  lh         $t5, 0x0($s0)
    /* 3FA84 8004F284 00640C00 */  sll        $t4, $t4, 16
    /* 3FA88 8004F288 006C0D00 */  sll        $t5, $t5, 16
    /* 3FA8C 8004F28C 0400D6EA */  swc2       $22, 0x4($s6)
    /* 3FA90 8004F290 06000896 */  lhu        $t0, 0x6($s0)
    /* 3FA94 8004F294 08000996 */  lhu        $t1, 0x8($s0)
    /* 3FA98 8004F298 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3FA9C 8004F29C 0C000B96 */  lhu        $t3, 0xC($s0)
    /* 3FAA0 8004F2A0 25400C01 */  or         $t0, $t0, $t4
    /* 3FAA4 8004F2A4 25482D01 */  or         $t1, $t1, $t5
    /* 3FAA8 8004F2A8 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3FAAC 8004F2AC 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3FAB0 8004F2B0 1C00CAA6 */  sh         $t2, 0x1C($s6)
    /* 3FAB4 8004F2B4 2400CBA6 */  sh         $t3, 0x24($s6)
    /* 3FAB8 8004F2B8 80100200 */  sll        $v0, $v0, 2
    /* 3FABC 8004F2BC 21105400 */  addu       $v0, $v0, $s4
    /* 3FAC0 8004F2C0 0000498C */  lw         $t1, 0x0($v0)
    /* 3FAC4 8004F2C4 2440DE02 */  and        $t0, $s6, $fp
    /* 3FAC8 8004F2C8 000048AC */  sw         $t0, 0x0($v0)
    /* 3FACC 8004F2CC 25483501 */  or         $t1, $t1, $s5
    /* 3FAD0 8004F2D0 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3FAD4 8004F2D4 2800D626 */  addiu      $s6, $s6, 0x28
  .L8004F2D8:
    /* 3FAD8 8004F2D8 18001026 */  addiu      $s0, $s0, 0x18
    /* 3FADC 8004F2DC 9FFF6016 */  bnez       $s3, .L8004F15C
    /* 3FAE0 8004F2E0 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004F2E4:
    /* 3FAE4 8004F2E4 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3FAE8 8004F2E8 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3FAEC 8004F2EC 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3FAF0 8004F2F0 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3FAF4 8004F2F4 2400B68F */  lw         $s6, 0x24($sp)
    /* 3FAF8 8004F2F8 2000B58F */  lw         $s5, 0x20($sp)
    /* 3FAFC 8004F2FC 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3FB00 8004F300 1800B38F */  lw         $s3, 0x18($sp)
    /* 3FB04 8004F304 1400B28F */  lw         $s2, 0x14($sp)
    /* 3FB08 8004F308 1000B18F */  lw         $s1, 0x10($sp)
    /* 3FB0C 8004F30C 0800E003 */  jr         $ra
    /* 3FB10 8004F310 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004F0FC
