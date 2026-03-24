glabel func_8004EAC8
    /* 3F2C8 8004EAC8 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3F2CC 8004EACC 3000BFAF */  sw         $ra, 0x30($sp)
    /* 3F2D0 8004EAD0 2C00BEAF */  sw         $fp, 0x2C($sp)
    /* 3F2D4 8004EAD4 2800B7AF */  sw         $s7, 0x28($sp)
    /* 3F2D8 8004EAD8 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3F2DC 8004EADC 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3F2E0 8004EAE0 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3F2E4 8004EAE4 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3F2E8 8004EAE8 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3F2EC 8004EAEC 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3F2F0 8004EAF0 68008010 */  beqz       $a0, .L8004EC94
    /* 3F2F4 8004EAF4 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3F2F8 8004EAF8 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3F2FC 8004EAFC 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3F300 8004EB00 801F123C */  lui        $s2, (0x1F800244 >> 16)
    /* 3F304 8004EB04 44025236 */  ori        $s2, $s2, (0x1F800244 & 0xFFFF)
    /* 3F308 8004EB08 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3F30C 8004EB0C 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3F310 8004EB10 0009153C */  lui        $s5, (0x9000000 >> 16)
    /* 3F314 8004EB14 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3F318 8004EB18 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3F31C 8004EB1C 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3F320 8004EB20 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3F324 8004EB24 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3F328 8004EB28 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
  .L8004EB2C:
    /* 3F32C 8004EB2C 0C00048E */  lw         $a0, 0xC($s0)
    /* 3F330 8004EB30 00000000 */  nop
    /* 3F334 8004EB34 80280400 */  sll        $a1, $a0, 2
    /* 3F338 8004EB38 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3F33C 8004EB3C 21287101 */  addu       $a1, $t3, $s1
    /* 3F340 8004EB40 0000A88C */  lw         $t0, 0x0($a1)
    /* 3F344 8004EB44 82290400 */  srl        $a1, $a0, 6
    /* 3F348 8004EB48 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3F34C 8004EB4C 21289101 */  addu       $a1, $t4, $s1
    /* 3F350 8004EB50 0000A98C */  lw         $t1, 0x0($a1)
    /* 3F354 8004EB54 822B0400 */  srl        $a1, $a0, 14
    /* 3F358 8004EB58 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3F35C 8004EB5C 2128B101 */  addu       $a1, $t5, $s1
    /* 3F360 8004EB60 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3F364 8004EB64 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3F368 8004EB68 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3F36C 8004EB6C 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3F370 8004EB70 24700901 */  and        $t6, $t0, $t1
    /* 3F374 8004EB74 2470CA01 */  and        $t6, $t6, $t2
    /* 3F378 8004EB78 0600404B */  nclip
    /* 3F37C 8004EB7C 4200C005 */  bltz       $t6, .L8004EC88
    /* 3F380 8004EB80 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3F384 8004EB84 0080CE31 */  andi       $t6, $t6, 0x8000
    /* 3F388 8004EB88 3F004104 */  bgez       $v0, .L8004EC88
    /* 3F38C 8004EB8C 00000000 */   nop
    /* 3F390 8004EB90 3D00C015 */  bnez       $t6, .L8004EC88
    /* 3F394 8004EB94 42100B00 */   srl       $v0, $t3, 1
    /* 3F398 8004EB98 21105200 */  addu       $v0, $v0, $s2
    /* 3F39C 8004EB9C 00004484 */  lh         $a0, 0x0($v0)
    /* 3F3A0 8004EBA0 42100C00 */  srl        $v0, $t4, 1
    /* 3F3A4 8004EBA4 21105200 */  addu       $v0, $v0, $s2
    /* 3F3A8 8004EBA8 00004584 */  lh         $a1, 0x0($v0)
    /* 3F3AC 8004EBAC 42100D00 */  srl        $v0, $t5, 1
    /* 3F3B0 8004EBB0 21105200 */  addu       $v0, $v0, $s2
    /* 3F3B4 8004EBB4 00004684 */  lh         $a2, 0x0($v0)
    /* 3F3B8 8004EBB8 00888448 */  mtc2       $a0, $17 /* handwritten instruction */
    /* 3F3BC 8004EBBC 00908548 */  mtc2       $a1, $18 /* handwritten instruction */
    /* 3F3C0 8004EBC0 00988648 */  mtc2       $a2, $19 /* handwritten instruction */
    /* 3F3C4 8004EBC4 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3F3C8 8004EBC8 00000000 */  nop
    /* 3F3CC 8004EBCC 2D00584B */  avsz3
    /* 3F3D0 8004EBD0 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3F3D4 8004EBD4 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 3F3D8 8004EBD8 2000CAAE */  sw         $t2, 0x20($s6)
    /* 3F3DC 8004EBDC 2A004018 */  blez       $v0, .L8004EC88
    /* 3F3E0 8004EBE0 80100200 */   sll       $v0, $v0, 2
    /* 3F3E4 8004EBE4 1000048E */  lw         $a0, 0x10($s0)
    /* 3F3E8 8004EBE8 0F80083C */  lui        $t0, %hi(D_800F2B70)
    /* 3F3EC 8004EBEC 702B0825 */  addiu      $t0, $t0, %lo(D_800F2B70)
    /* 3F3F0 8004EBF0 824C0400 */  srl        $t1, $a0, 18
    /* 3F3F4 8004EBF4 02520400 */  srl        $t2, $a0, 8
    /* 3F3F8 8004EBF8 80200400 */  sll        $a0, $a0, 2
    /* 3F3FC 8004EBFC FC0F2931 */  andi       $t1, $t1, 0xFFC
    /* 3F400 8004EC00 FC0F4A31 */  andi       $t2, $t2, 0xFFC
    /* 3F404 8004EC04 FC0F8430 */  andi       $a0, $a0, 0xFFC
    /* 3F408 8004EC08 20482801 */  add        $t1, $t1, $t0 /* handwritten instruction */
    /* 3F40C 8004EC0C 20504801 */  add        $t2, $t2, $t0 /* handwritten instruction */
    /* 3F410 8004EC10 20208800 */  add        $a0, $a0, $t0 /* handwritten instruction */
    /* 3F414 8004EC14 0000288D */  lw         $t0, 0x0($t1)
    /* 3F418 8004EC18 0000498D */  lw         $t1, 0x0($t2)
    /* 3F41C 8004EC1C 00008A8C */  lw         $t2, 0x0($a0)
    /* 3F420 8004EC20 0034043C */  lui        $a0, (0x34000000 >> 16)
    /* 3F424 8004EC24 25400401 */  or         $t0, $t0, $a0
    /* 3F428 8004EC28 0400C8AE */  sw         $t0, 0x4($s6)
    /* 3F42C 8004EC2C 1000C9AE */  sw         $t1, 0x10($s6)
    /* 3F430 8004EC30 1C00CAAE */  sw         $t2, 0x1C($s6)
    /* 3F434 8004EC34 06106200 */  srlv       $v0, $v0, $v1
    /* 3F438 8004EC38 C2420200 */  srl        $t0, $v0, 11
    /* 3F43C 8004EC3C FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3F440 8004EC40 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3F444 8004EC44 07100201 */  srav       $v0, $v0, $t0
    /* 3F448 8004EC48 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3F44C 8004EC4C 0000088E */  lw         $t0, 0x0($s0)
    /* 3F450 8004EC50 0400098E */  lw         $t1, 0x4($s0)
    /* 3F454 8004EC54 08000A86 */  lh         $t2, 0x8($s0)
    /* 3F458 8004EC58 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3F45C 8004EC5C 1800C9AE */  sw         $t1, 0x18($s6)
    /* 3F460 8004EC60 2400CAA6 */  sh         $t2, 0x24($s6)
    /* 3F464 8004EC64 80100200 */  sll        $v0, $v0, 2
    /* 3F468 8004EC68 21105400 */  addu       $v0, $v0, $s4
    /* 3F46C 8004EC6C 0000498C */  lw         $t1, 0x0($v0)
    /* 3F470 8004EC70 2440DE02 */  and        $t0, $s6, $fp
    /* 3F474 8004EC74 000048AC */  sw         $t0, 0x0($v0)
    /* 3F478 8004EC78 25483501 */  or         $t1, $t1, $s5
    /* 3F47C 8004EC7C 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3F480 8004EC80 00000000 */  nop
    /* 3F484 8004EC84 2800D626 */  addiu      $s6, $s6, 0x28
  .L8004EC88:
    /* 3F488 8004EC88 14001026 */  addiu      $s0, $s0, 0x14
    /* 3F48C 8004EC8C A7FF6016 */  bnez       $s3, .L8004EB2C
    /* 3F490 8004EC90 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004EC94:
    /* 3F494 8004EC94 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3F498 8004EC98 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3F49C 8004EC9C 3000BF8F */  lw         $ra, 0x30($sp)
    /* 3F4A0 8004ECA0 2C00BE8F */  lw         $fp, 0x2C($sp)
    /* 3F4A4 8004ECA4 2800B78F */  lw         $s7, 0x28($sp)
    /* 3F4A8 8004ECA8 2400B68F */  lw         $s6, 0x24($sp)
    /* 3F4AC 8004ECAC 2000B58F */  lw         $s5, 0x20($sp)
    /* 3F4B0 8004ECB0 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3F4B4 8004ECB4 1800B38F */  lw         $s3, 0x18($sp)
    /* 3F4B8 8004ECB8 1400B28F */  lw         $s2, 0x14($sp)
    /* 3F4BC 8004ECBC 1000B18F */  lw         $s1, 0x10($sp)
    /* 3F4C0 8004ECC0 0800E003 */  jr         $ra
    /* 3F4C4 8004ECC4 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004EAC8
