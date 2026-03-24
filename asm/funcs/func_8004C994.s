glabel func_8004C994
    /* 3D194 8004C994 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3D198 8004C998 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3D19C 8004C99C 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3D1A0 8004C9A0 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3D1A4 8004C9A4 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3D1A8 8004C9A8 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3D1AC 8004C9AC 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3D1B0 8004C9B0 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3D1B4 8004C9B4 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3D1B8 8004C9B8 68008010 */  beqz       $a0, .L8004CB5C
    /* 3D1BC 8004C9BC FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3D1C0 8004C9C0 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3D1C4 8004C9C4 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3D1C8 8004C9C8 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3D1CC 8004C9CC B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3D1D0 8004C9D0 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3D1D4 8004C9D4 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3D1D8 8004C9D8 0007153C */  lui        $s5, (0x7000000 >> 16)
    /* 3D1DC 8004C9DC FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3D1E0 8004C9E0 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3D1E4 8004C9E4 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3D1E8 8004C9E8 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3D1EC 8004C9EC 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3D1F0 8004C9F0 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
  .L8004C9F4:
    /* 3D1F4 8004C9F4 0C00048E */  lw         $a0, 0xC($s0)
    /* 3D1F8 8004C9F8 000006CA */  lwc2       $6, 0x0($s0)
    /* 3D1FC 8004C9FC 80280400 */  sll        $a1, $a0, 2
    /* 3D200 8004CA00 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3D204 8004CA04 21287101 */  addu       $a1, $t3, $s1
    /* 3D208 8004CA08 0000A88C */  lw         $t0, 0x0($a1)
    /* 3D20C 8004CA0C 82290400 */  srl        $a1, $a0, 6
    /* 3D210 8004CA10 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3D214 8004CA14 21289101 */  addu       $a1, $t4, $s1
    /* 3D218 8004CA18 0000A98C */  lw         $t1, 0x0($a1)
    /* 3D21C 8004CA1C 822B0400 */  srl        $a1, $a0, 14
    /* 3D220 8004CA20 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3D224 8004CA24 2128B101 */  addu       $a1, $t5, $s1
    /* 3D228 8004CA28 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3D22C 8004CA2C 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3D230 8004CA30 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3D234 8004CA34 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3D238 8004CA38 24700901 */  and        $t6, $t0, $t1
    /* 3D23C 8004CA3C 2470CA01 */  and        $t6, $t6, $t2
    /* 3D240 8004CA40 0600404B */  nclip
    /* 3D244 8004CA44 4200C005 */  bltz       $t6, .L8004CB50
    /* 3D248 8004CA48 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3D24C 8004CA4C 0080CE31 */  andi       $t6, $t6, 0x8000
    /* 3D250 8004CA50 3F00C015 */  bnez       $t6, .L8004CB50
    /* 3D254 8004CA54 2A384000 */   slt       $a3, $v0, $zero
    /* 3D258 8004CA58 FFFFE720 */  addi       $a3, $a3, -0x1 /* handwritten instruction */
    /* 3D25C 8004CA5C 42100B00 */  srl        $v0, $t3, 1
    /* 3D260 8004CA60 21105200 */  addu       $v0, $v0, $s2
    /* 3D264 8004CA64 00004484 */  lh         $a0, 0x0($v0)
    /* 3D268 8004CA68 42100C00 */  srl        $v0, $t4, 1
    /* 3D26C 8004CA6C 21105200 */  addu       $v0, $v0, $s2
    /* 3D270 8004CA70 00004584 */  lh         $a1, 0x0($v0)
    /* 3D274 8004CA74 42100D00 */  srl        $v0, $t5, 1
    /* 3D278 8004CA78 21105200 */  addu       $v0, $v0, $s2
    /* 3D27C 8004CA7C 00004684 */  lh         $a2, 0x0($v0)
    /* 3D280 8004CA80 00888448 */  mtc2       $a0, $17 /* handwritten instruction */
    /* 3D284 8004CA84 00908548 */  mtc2       $a1, $18 /* handwritten instruction */
    /* 3D288 8004CA88 00988648 */  mtc2       $a2, $19 /* handwritten instruction */
    /* 3D28C 8004CA8C 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3D290 8004CA90 00000000 */  nop
    /* 3D294 8004CA94 2D00584B */  avsz3
    /* 3D298 8004CA98 1000C9AE */  sw         $t1, 0x10($s6)
    /* 3D29C 8004CA9C 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 3D2A0 8004CAA0 1800CAAE */  sw         $t2, 0x18($s6)
    /* 3D2A4 8004CAA4 2A004018 */  blez       $v0, .L8004CB50
    /* 3D2A8 8004CAA8 80100200 */   sll       $v0, $v0, 2
    /* 3D2AC 8004CAAC 0100013C */  lui        $at, (0x10000 >> 16)
    /* 3D2B0 8004CAB0 2AC84100 */  slt        $t9, $v0, $at
    /* 3D2B4 8004CAB4 26002013 */  beqz       $t9, .L8004CB50
    /* 3D2B8 8004CAB8 00000000 */   nop
    /* 3D2BC 8004CABC 1000088E */  lw         $t0, 0x10($s0)
    /* 3D2C0 8004CAC0 1400098E */  lw         $t1, 0x14($s0)
    /* 3D2C4 8004CAC4 26400701 */  xor        $t0, $t0, $a3
    /* 3D2C8 8004CAC8 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 3D2CC 8004CACC 26482701 */  xor        $t1, $t1, $a3
    /* 3D2D0 8004CAD0 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 3D2D4 8004CAD4 B0FF0124 */  addiu      $at, $zero, -0x50
    /* 3D2D8 8004CAD8 2640E100 */  xor        $t0, $a3, $at
    /* 3D2DC 8004CADC 20104800 */  add        $v0, $v0, $t0 /* handwritten instruction */
    /* 3D2E0 8004CAE0 06106200 */  srlv       $v0, $v0, $v1
    /* 3D2E4 8004CAE4 C2420200 */  srl        $t0, $v0, 11
    /* 3D2E8 8004CAE8 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3D2EC 8004CAEC 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3D2F0 8004CAF0 07100201 */  srav       $v0, $v0, $t0
    /* 3D2F4 8004CAF4 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3D2F8 8004CAF8 1E04C84A */  ncs
    /* 3D2FC 8004CAFC 04000C86 */  lh         $t4, 0x4($s0)
    /* 3D300 8004CB00 00000D86 */  lh         $t5, 0x0($s0)
    /* 3D304 8004CB04 00640C00 */  sll        $t4, $t4, 16
    /* 3D308 8004CB08 006C0D00 */  sll        $t5, $t5, 16
    /* 3D30C 8004CB0C 0400D6EA */  swc2       $22, 0x4($s6)
    /* 3D310 8004CB10 06000896 */  lhu        $t0, 0x6($s0)
    /* 3D314 8004CB14 08000996 */  lhu        $t1, 0x8($s0)
    /* 3D318 8004CB18 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3D31C 8004CB1C 25400C01 */  or         $t0, $t0, $t4
    /* 3D320 8004CB20 25482D01 */  or         $t1, $t1, $t5
    /* 3D324 8004CB24 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3D328 8004CB28 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3D32C 8004CB2C 1C00CAA6 */  sh         $t2, 0x1C($s6)
    /* 3D330 8004CB30 80100200 */  sll        $v0, $v0, 2
    /* 3D334 8004CB34 21105400 */  addu       $v0, $v0, $s4
    /* 3D338 8004CB38 0000498C */  lw         $t1, 0x0($v0)
    /* 3D33C 8004CB3C 2440DE02 */  and        $t0, $s6, $fp
    /* 3D340 8004CB40 000048AC */  sw         $t0, 0x0($v0)
    /* 3D344 8004CB44 25483501 */  or         $t1, $t1, $s5
    /* 3D348 8004CB48 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3D34C 8004CB4C 2000D626 */  addiu      $s6, $s6, 0x20
  .L8004CB50:
    /* 3D350 8004CB50 18001026 */  addiu      $s0, $s0, 0x18
    /* 3D354 8004CB54 A7FF6016 */  bnez       $s3, .L8004C9F4
    /* 3D358 8004CB58 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004CB5C:
    /* 3D35C 8004CB5C 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3D360 8004CB60 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3D364 8004CB64 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3D368 8004CB68 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3D36C 8004CB6C 2400B68F */  lw         $s6, 0x24($sp)
    /* 3D370 8004CB70 2000B58F */  lw         $s5, 0x20($sp)
    /* 3D374 8004CB74 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3D378 8004CB78 1800B38F */  lw         $s3, 0x18($sp)
    /* 3D37C 8004CB7C 1400B28F */  lw         $s2, 0x14($sp)
    /* 3D380 8004CB80 1000B18F */  lw         $s1, 0x10($sp)
    /* 3D384 8004CB84 0800E003 */  jr         $ra
    /* 3D388 8004CB88 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004C994
