glabel func_8004F314
    /* 3FB14 8004F314 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3FB18 8004F318 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3FB1C 8004F31C 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3FB20 8004F320 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3FB24 8004F324 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3FB28 8004F328 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3FB2C 8004F32C 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3FB30 8004F330 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3FB34 8004F334 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3FB38 8004F338 74008010 */  beqz       $a0, .L8004F50C
    /* 3FB3C 8004F33C FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3FB40 8004F340 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3FB44 8004F344 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3FB48 8004F348 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3FB4C 8004F34C B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3FB50 8004F350 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3FB54 8004F354 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3FB58 8004F358 0009153C */  lui        $s5, (0x9000000 >> 16)
    /* 3FB5C 8004F35C FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3FB60 8004F360 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3FB64 8004F364 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3FB68 8004F368 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3FB6C 8004F36C 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3FB70 8004F370 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
    /* 3FB74 8004F374 000006CA */  lwc2       $6, 0x0($s0)
  .L8004F378:
    /* 3FB78 8004F378 0C00048E */  lw         $a0, 0xC($s0)
    /* 3FB7C 8004F37C 00000000 */  nop
    /* 3FB80 8004F380 80280400 */  sll        $a1, $a0, 2
    /* 3FB84 8004F384 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3FB88 8004F388 21287101 */  addu       $a1, $t3, $s1
    /* 3FB8C 8004F38C 0000A88C */  lw         $t0, 0x0($a1)
    /* 3FB90 8004F390 82290400 */  srl        $a1, $a0, 6
    /* 3FB94 8004F394 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3FB98 8004F398 21289101 */  addu       $a1, $t4, $s1
    /* 3FB9C 8004F39C 0000A98C */  lw         $t1, 0x0($a1)
    /* 3FBA0 8004F3A0 822B0400 */  srl        $a1, $a0, 14
    /* 3FBA4 8004F3A4 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3FBA8 8004F3A8 2128B101 */  addu       $a1, $t5, $s1
    /* 3FBAC 8004F3AC 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3FBB0 8004F3B0 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3FBB4 8004F3B4 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3FBB8 8004F3B8 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3FBBC 8004F3BC 24700901 */  and        $t6, $t0, $t1
    /* 3FBC0 8004F3C0 2470CA01 */  and        $t6, $t6, $t2
    /* 3FBC4 8004F3C4 0600404B */  nclip
    /* 3FBC8 8004F3C8 4D00C005 */  bltz       $t6, .L8004F500
    /* 3FBCC 8004F3CC 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3FBD0 8004F3D0 0080CE31 */  andi       $t6, $t6, 0x8000
    /* 3FBD4 8004F3D4 4A00C015 */  bnez       $t6, .L8004F500
    /* 3FBD8 8004F3D8 2A384000 */   slt       $a3, $v0, $zero
    /* 3FBDC 8004F3DC FFFFE720 */  addi       $a3, $a3, -0x1 /* handwritten instruction */
    /* 3FBE0 8004F3E0 42100B00 */  srl        $v0, $t3, 1
    /* 3FBE4 8004F3E4 21105200 */  addu       $v0, $v0, $s2
    /* 3FBE8 8004F3E8 00004484 */  lh         $a0, 0x0($v0)
    /* 3FBEC 8004F3EC 42100C00 */  srl        $v0, $t4, 1
    /* 3FBF0 8004F3F0 21105200 */  addu       $v0, $v0, $s2
    /* 3FBF4 8004F3F4 00004584 */  lh         $a1, 0x0($v0)
    /* 3FBF8 8004F3F8 42100D00 */  srl        $v0, $t5, 1
    /* 3FBFC 8004F3FC 21105200 */  addu       $v0, $v0, $s2
    /* 3FC00 8004F400 00004684 */  lh         $a2, 0x0($v0)
    /* 3FC04 8004F404 00888448 */  mtc2       $a0, $17 /* handwritten instruction */
    /* 3FC08 8004F408 00908548 */  mtc2       $a1, $18 /* handwritten instruction */
    /* 3FC0C 8004F40C 00988648 */  mtc2       $a2, $19 /* handwritten instruction */
    /* 3FC10 8004F410 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3FC14 8004F414 00000000 */  nop
    /* 3FC18 8004F418 2D00584B */  avsz3
    /* 3FC1C 8004F41C 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3FC20 8004F420 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 3FC24 8004F424 2000CAAE */  sw         $t2, 0x20($s6)
    /* 3FC28 8004F428 35004018 */  blez       $v0, .L8004F500
    /* 3FC2C 8004F42C 80100200 */   sll       $v0, $v0, 2
    /* 3FC30 8004F430 0100013C */  lui        $at, (0x10000 >> 16)
    /* 3FC34 8004F434 2AC84100 */  slt        $t9, $v0, $at
    /* 3FC38 8004F438 31002013 */  beqz       $t9, .L8004F500
    /* 3FC3C 8004F43C 00000000 */   nop
    /* 3FC40 8004F440 1000088E */  lw         $t0, 0x10($s0)
    /* 3FC44 8004F444 1400098E */  lw         $t1, 0x14($s0)
    /* 3FC48 8004F448 26400701 */  xor        $t0, $t0, $a3
    /* 3FC4C 8004F44C 26482701 */  xor        $t1, $t1, $a3
    /* 3FC50 8004F450 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 3FC54 8004F454 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 3FC58 8004F458 18000A8E */  lw         $t2, 0x18($s0)
    /* 3FC5C 8004F45C 024C0900 */  srl        $t1, $t1, 16
    /* 3FC60 8004F460 26504701 */  xor        $t2, $t2, $a3
    /* 3FC64 8004F464 00108A48 */  mtc2       $t2, $2 /* handwritten instruction */
    /* 3FC68 8004F468 00188948 */  mtc2       $t1, $3 /* handwritten instruction */
    /* 3FC6C 8004F46C 1C00088E */  lw         $t0, 0x1C($s0)
    /* 3FC70 8004F470 2000098E */  lw         $t1, 0x20($s0)
    /* 3FC74 8004F474 26400701 */  xor        $t0, $t0, $a3
    /* 3FC78 8004F478 26482701 */  xor        $t1, $t1, $a3
    /* 3FC7C 8004F47C 00208848 */  mtc2       $t0, $4 /* handwritten instruction */
    /* 3FC80 8004F480 00288948 */  mtc2       $t1, $5 /* handwritten instruction */
    /* 3FC84 8004F484 06106200 */  srlv       $v0, $v0, $v1
    /* 3FC88 8004F488 C2420200 */  srl        $t0, $v0, 11
    /* 3FC8C 8004F48C FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3FC90 8004F490 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3FC94 8004F494 07100201 */  srav       $v0, $v0, $t0
    /* 3FC98 8004F498 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3FC9C 8004F49C 2004D84A */  nct
    /* 3FCA0 8004F4A0 04000C86 */  lh         $t4, 0x4($s0)
    /* 3FCA4 8004F4A4 00000D86 */  lh         $t5, 0x0($s0)
    /* 3FCA8 8004F4A8 00640C00 */  sll        $t4, $t4, 16
    /* 3FCAC 8004F4AC 006C0D00 */  sll        $t5, $t5, 16
    /* 3FCB0 8004F4B0 06000896 */  lhu        $t0, 0x6($s0)
    /* 3FCB4 8004F4B4 08000996 */  lhu        $t1, 0x8($s0)
    /* 3FCB8 8004F4B8 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3FCBC 8004F4BC 25400C01 */  or         $t0, $t0, $t4
    /* 3FCC0 8004F4C0 25482D01 */  or         $t1, $t1, $t5
    /* 3FCC4 8004F4C4 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3FCC8 8004F4C8 1800C9AE */  sw         $t1, 0x18($s6)
    /* 3FCCC 8004F4CC 2400CAA6 */  sh         $t2, 0x24($s6)
    /* 3FCD0 8004F4D0 80100200 */  sll        $v0, $v0, 2
    /* 3FCD4 8004F4D4 21105400 */  addu       $v0, $v0, $s4
    /* 3FCD8 8004F4D8 0000498C */  lw         $t1, 0x0($v0)
    /* 3FCDC 8004F4DC 2440DE02 */  and        $t0, $s6, $fp
    /* 3FCE0 8004F4E0 000048AC */  sw         $t0, 0x0($v0)
    /* 3FCE4 8004F4E4 25483501 */  or         $t1, $t1, $s5
    /* 3FCE8 8004F4E8 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3FCEC 8004F4EC 0400D4EA */  swc2       $20, 0x4($s6)
    /* 3FCF0 8004F4F0 1000D5EA */  swc2       $21, 0x10($s6)
    /* 3FCF4 8004F4F4 1C00D6EA */  swc2       $22, 0x1C($s6)
    /* 3FCF8 8004F4F8 00000000 */  nop
    /* 3FCFC 8004F4FC 2800D626 */  addiu      $s6, $s6, 0x28
  .L8004F500:
    /* 3FD00 8004F500 24001026 */  addiu      $s0, $s0, 0x24
    /* 3FD04 8004F504 9CFF6016 */  bnez       $s3, .L8004F378
    /* 3FD08 8004F508 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004F50C:
    /* 3FD0C 8004F50C 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3FD10 8004F510 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3FD14 8004F514 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3FD18 8004F518 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3FD1C 8004F51C 2400B68F */  lw         $s6, 0x24($sp)
    /* 3FD20 8004F520 2000B58F */  lw         $s5, 0x20($sp)
    /* 3FD24 8004F524 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3FD28 8004F528 1800B38F */  lw         $s3, 0x18($sp)
    /* 3FD2C 8004F52C 1400B28F */  lw         $s2, 0x14($sp)
    /* 3FD30 8004F530 1000B18F */  lw         $s1, 0x10($sp)
    /* 3FD34 8004F534 0800E003 */  jr         $ra
    /* 3FD38 8004F538 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004F314
