glabel func_8004FB74
    /* 40374 8004FB74 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 40378 8004FB78 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 4037C 8004FB7C 2800BEAF */  sw         $fp, 0x28($sp)
    /* 40380 8004FB80 2400B6AF */  sw         $s6, 0x24($sp)
    /* 40384 8004FB84 2000B5AF */  sw         $s5, 0x20($sp)
    /* 40388 8004FB88 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 4038C 8004FB8C 1800B3AF */  sw         $s3, 0x18($sp)
    /* 40390 8004FB90 1400B2AF */  sw         $s2, 0x14($sp)
    /* 40394 8004FB94 1000B1AF */  sw         $s1, 0x10($sp)
    /* 40398 8004FB98 5D008010 */  beqz       $a0, .L8004FD10
    /* 4039C 8004FB9C FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 403A0 8004FBA0 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 403A4 8004FBA4 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 403A8 8004FBA8 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 403AC 8004FBAC B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 403B0 8004FBB0 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 403B4 8004FBB4 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 403B8 8004FBB8 0007153C */  lui        $s5, (0x7000000 >> 16)
    /* 403BC 8004FBBC FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 403C0 8004FBC0 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 403C4 8004FBC4 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 403C8 8004FBC8 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 403CC 8004FBCC 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 403D0 8004FBD0 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
  .L8004FBD4:
    /* 403D4 8004FBD4 0C00048E */  lw         $a0, 0xC($s0)
    /* 403D8 8004FBD8 000006CA */  lwc2       $6, 0x0($s0)
    /* 403DC 8004FBDC 80280400 */  sll        $a1, $a0, 2
    /* 403E0 8004FBE0 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 403E4 8004FBE4 21287101 */  addu       $a1, $t3, $s1
    /* 403E8 8004FBE8 0000A88C */  lw         $t0, 0x0($a1)
    /* 403EC 8004FBEC 82290400 */  srl        $a1, $a0, 6
    /* 403F0 8004FBF0 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 403F4 8004FBF4 21289101 */  addu       $a1, $t4, $s1
    /* 403F8 8004FBF8 0000A98C */  lw         $t1, 0x0($a1)
    /* 403FC 8004FBFC 822B0400 */  srl        $a1, $a0, 14
    /* 40400 8004FC00 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 40404 8004FC04 2128B101 */  addu       $a1, $t5, $s1
    /* 40408 8004FC08 0000AA8C */  lw         $t2, 0x0($a1)
    /* 4040C 8004FC0C 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 40410 8004FC10 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 40414 8004FC14 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 40418 8004FC18 24700901 */  and        $t6, $t0, $t1
    /* 4041C 8004FC1C 2470CA01 */  and        $t6, $t6, $t2
    /* 40420 8004FC20 0600404B */  nclip
    /* 40424 8004FC24 3700C005 */  bltz       $t6, .L8004FD04
    /* 40428 8004FC28 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 4042C 8004FC2C 0080CE31 */  andi       $t6, $t6, 0x8000
    /* 40430 8004FC30 34004104 */  bgez       $v0, .L8004FD04
    /* 40434 8004FC34 00000000 */   nop
    /* 40438 8004FC38 3200C015 */  bnez       $t6, .L8004FD04
    /* 4043C 8004FC3C 42100B00 */   srl       $v0, $t3, 1
    /* 40440 8004FC40 21105200 */  addu       $v0, $v0, $s2
    /* 40444 8004FC44 00004484 */  lh         $a0, 0x0($v0)
    /* 40448 8004FC48 42100C00 */  srl        $v0, $t4, 1
    /* 4044C 8004FC4C 21105200 */  addu       $v0, $v0, $s2
    /* 40450 8004FC50 00004584 */  lh         $a1, 0x0($v0)
    /* 40454 8004FC54 42100D00 */  srl        $v0, $t5, 1
    /* 40458 8004FC58 21105200 */  addu       $v0, $v0, $s2
    /* 4045C 8004FC5C 00004684 */  lh         $a2, 0x0($v0)
    /* 40460 8004FC60 00888448 */  mtc2       $a0, $17 /* handwritten instruction */
    /* 40464 8004FC64 00908548 */  mtc2       $a1, $18 /* handwritten instruction */
    /* 40468 8004FC68 00988648 */  mtc2       $a2, $19 /* handwritten instruction */
    /* 4046C 8004FC6C 0800C8AE */  sw         $t0, 0x8($s6)
    /* 40470 8004FC70 00000000 */  nop
    /* 40474 8004FC74 2D00584B */  avsz3
    /* 40478 8004FC78 1000C9AE */  sw         $t1, 0x10($s6)
    /* 4047C 8004FC7C 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 40480 8004FC80 1800CAAE */  sw         $t2, 0x18($s6)
    /* 40484 8004FC84 1F004018 */  blez       $v0, .L8004FD04
    /* 40488 8004FC88 80100200 */   sll       $v0, $v0, 2
    /* 4048C 8004FC8C 100000CA */  lwc2       $0, 0x10($s0)
    /* 40490 8004FC90 140001CA */  lwc2       $1, 0x14($s0)
    /* 40494 8004FC94 06106200 */  srlv       $v0, $v0, $v1
    /* 40498 8004FC98 C2420200 */  srl        $t0, $v0, 11
    /* 4049C 8004FC9C FF074230 */  andi       $v0, $v0, 0x7FF
    /* 404A0 8004FCA0 1E04C84A */  ncs
    /* 404A4 8004FCA4 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 404A8 8004FCA8 07100201 */  srav       $v0, $v0, $t0
    /* 404AC 8004FCAC FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 404B0 8004FCB0 04000C86 */  lh         $t4, 0x4($s0)
    /* 404B4 8004FCB4 00000D86 */  lh         $t5, 0x0($s0)
    /* 404B8 8004FCB8 00640C00 */  sll        $t4, $t4, 16
    /* 404BC 8004FCBC 006C0D00 */  sll        $t5, $t5, 16
    /* 404C0 8004FCC0 06000896 */  lhu        $t0, 0x6($s0)
    /* 404C4 8004FCC4 08000996 */  lhu        $t1, 0x8($s0)
    /* 404C8 8004FCC8 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 404CC 8004FCCC 25400C01 */  or         $t0, $t0, $t4
    /* 404D0 8004FCD0 25482D01 */  or         $t1, $t1, $t5
    /* 404D4 8004FCD4 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 404D8 8004FCD8 1400C9AE */  sw         $t1, 0x14($s6)
    /* 404DC 8004FCDC 1C00CAA6 */  sh         $t2, 0x1C($s6)
    /* 404E0 8004FCE0 0400D6EA */  swc2       $22, 0x4($s6)
    /* 404E4 8004FCE4 80100200 */  sll        $v0, $v0, 2
    /* 404E8 8004FCE8 21105400 */  addu       $v0, $v0, $s4
    /* 404EC 8004FCEC 0000498C */  lw         $t1, 0x0($v0)
    /* 404F0 8004FCF0 2440DE02 */  and        $t0, $s6, $fp
    /* 404F4 8004FCF4 000048AC */  sw         $t0, 0x0($v0)
    /* 404F8 8004FCF8 25483501 */  or         $t1, $t1, $s5
    /* 404FC 8004FCFC 0000C9AE */  sw         $t1, 0x0($s6)
    /* 40500 8004FD00 2000D626 */  addiu      $s6, $s6, 0x20
  .L8004FD04:
    /* 40504 8004FD04 18001026 */  addiu      $s0, $s0, 0x18
    /* 40508 8004FD08 B2FF6016 */  bnez       $s3, .L8004FBD4
    /* 4050C 8004FD0C FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004FD10:
    /* 40510 8004FD10 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 40514 8004FD14 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 40518 8004FD18 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 4051C 8004FD1C 2800BE8F */  lw         $fp, 0x28($sp)
    /* 40520 8004FD20 2400B68F */  lw         $s6, 0x24($sp)
    /* 40524 8004FD24 2000B58F */  lw         $s5, 0x20($sp)
    /* 40528 8004FD28 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 4052C 8004FD2C 1800B38F */  lw         $s3, 0x18($sp)
    /* 40530 8004FD30 1400B28F */  lw         $s2, 0x14($sp)
    /* 40534 8004FD34 1000B18F */  lw         $s1, 0x10($sp)
    /* 40538 8004FD38 0800E003 */  jr         $ra
    /* 4053C 8004FD3C 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004FB74
