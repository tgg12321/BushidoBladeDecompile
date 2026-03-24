glabel func_8004CB8C
    /* 3D38C 8004CB8C C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3D390 8004CB90 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3D394 8004CB94 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3D398 8004CB98 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3D39C 8004CB9C 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3D3A0 8004CBA0 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3D3A4 8004CBA4 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3D3A8 8004CBA8 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3D3AC 8004CBAC 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3D3B0 8004CBB0 73008010 */  beqz       $a0, .L8004CD80
    /* 3D3B4 8004CBB4 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3D3B8 8004CBB8 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3D3BC 8004CBBC 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3D3C0 8004CBC0 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3D3C4 8004CBC4 B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3D3C8 8004CBC8 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3D3CC 8004CBCC 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3D3D0 8004CBD0 0009153C */  lui        $s5, (0x9000000 >> 16)
    /* 3D3D4 8004CBD4 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3D3D8 8004CBD8 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3D3DC 8004CBDC 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3D3E0 8004CBE0 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3D3E4 8004CBE4 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3D3E8 8004CBE8 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
  .L8004CBEC:
    /* 3D3EC 8004CBEC 0E000496 */  lhu        $a0, 0xE($s0)
    /* 3D3F0 8004CBF0 10000596 */  lhu        $a1, 0x10($s0)
    /* 3D3F4 8004CBF4 000006CA */  lwc2       $6, 0x0($s0)
    /* 3D3F8 8004CBF8 002C0500 */  sll        $a1, $a1, 16
    /* 3D3FC 8004CBFC 25208500 */  or         $a0, $a0, $a1
    /* 3D400 8004CC00 80280400 */  sll        $a1, $a0, 2
    /* 3D404 8004CC04 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3D408 8004CC08 21287101 */  addu       $a1, $t3, $s1
    /* 3D40C 8004CC0C 0000A88C */  lw         $t0, 0x0($a1)
    /* 3D410 8004CC10 82290400 */  srl        $a1, $a0, 6
    /* 3D414 8004CC14 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3D418 8004CC18 21289101 */  addu       $a1, $t4, $s1
    /* 3D41C 8004CC1C 0000A98C */  lw         $t1, 0x0($a1)
    /* 3D420 8004CC20 822B0400 */  srl        $a1, $a0, 14
    /* 3D424 8004CC24 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3D428 8004CC28 2128B101 */  addu       $a1, $t5, $s1
    /* 3D42C 8004CC2C 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3D430 8004CC30 822D0400 */  srl        $a1, $a0, 22
    /* 3D434 8004CC34 FC03AE30 */  andi       $t6, $a1, 0x3FC
    /* 3D438 8004CC38 2128D101 */  addu       $a1, $t6, $s1
    /* 3D43C 8004CC3C 0000AF8C */  lw         $t7, 0x0($a1)
    /* 3D440 8004CC40 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3D444 8004CC44 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3D448 8004CC48 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3D44C 8004CC4C 24C00901 */  and        $t8, $t0, $t1
    /* 3D450 8004CC50 24C00A03 */  and        $t8, $t8, $t2
    /* 3D454 8004CC54 24C00F03 */  and        $t8, $t8, $t7
    /* 3D458 8004CC58 0600404B */  nclip
    /* 3D45C 8004CC5C 45000007 */  bltz       $t8, .L8004CD74
    /* 3D460 8004CC60 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3D464 8004CC64 00801833 */  andi       $t8, $t8, 0x8000
    /* 3D468 8004CC68 42000017 */  bnez       $t8, .L8004CD74
    /* 3D46C 8004CC6C 2A384000 */   slt       $a3, $v0, $zero
    /* 3D470 8004CC70 FFFFE720 */  addi       $a3, $a3, -0x1 /* handwritten instruction */
    /* 3D474 8004CC74 42100B00 */  srl        $v0, $t3, 1
    /* 3D478 8004CC78 21105200 */  addu       $v0, $v0, $s2
    /* 3D47C 8004CC7C 00004484 */  lh         $a0, 0x0($v0)
    /* 3D480 8004CC80 42100C00 */  srl        $v0, $t4, 1
    /* 3D484 8004CC84 21105200 */  addu       $v0, $v0, $s2
    /* 3D488 8004CC88 00004584 */  lh         $a1, 0x0($v0)
    /* 3D48C 8004CC8C 42100D00 */  srl        $v0, $t5, 1
    /* 3D490 8004CC90 21105200 */  addu       $v0, $v0, $s2
    /* 3D494 8004CC94 00004684 */  lh         $a2, 0x0($v0)
    /* 3D498 8004CC98 42100E00 */  srl        $v0, $t6, 1
    /* 3D49C 8004CC9C 21105200 */  addu       $v0, $v0, $s2
    /* 3D4A0 8004CCA0 00004B84 */  lh         $t3, 0x0($v0)
    /* 3D4A4 8004CCA4 20208500 */  add        $a0, $a0, $a1 /* handwritten instruction */
    /* 3D4A8 8004CCA8 20208600 */  add        $a0, $a0, $a2 /* handwritten instruction */
    /* 3D4AC 8004CCAC 20208B00 */  add        $a0, $a0, $t3 /* handwritten instruction */
    /* 3D4B0 8004CCB0 83100400 */  sra        $v0, $a0, 2
    /* 3D4B4 8004CCB4 2F004018 */  blez       $v0, .L8004CD74
    /* 3D4B8 8004CCB8 0100013C */   lui       $at, (0x10000 >> 16)
    /* 3D4BC 8004CCBC 2AC84100 */  slt        $t9, $v0, $at
    /* 3D4C0 8004CCC0 2C002013 */  beqz       $t9, .L8004CD74
    /* 3D4C4 8004CCC4 00000000 */   nop
    /* 3D4C8 8004CCC8 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3D4CC 8004CCCC 1000C9AE */  sw         $t1, 0x10($s6)
    /* 3D4D0 8004CCD0 1800CAAE */  sw         $t2, 0x18($s6)
    /* 3D4D4 8004CCD4 2000CFAE */  sw         $t7, 0x20($s6)
    /* 3D4D8 8004CCD8 12000896 */  lhu        $t0, 0x12($s0)
    /* 3D4DC 8004CCDC 1400098E */  lw         $t1, 0x14($s0)
    /* 3D4E0 8004CCE0 26400701 */  xor        $t0, $t0, $a3
    /* 3D4E4 8004CCE4 00088848 */  mtc2       $t0, $1 /* handwritten instruction */
    /* 3D4E8 8004CCE8 26482701 */  xor        $t1, $t1, $a3
    /* 3D4EC 8004CCEC 00008948 */  mtc2       $t1, $0 /* handwritten instruction */
    /* 3D4F0 8004CCF0 B0FF0124 */  addiu      $at, $zero, -0x50
    /* 3D4F4 8004CCF4 2640E100 */  xor        $t0, $a3, $at
    /* 3D4F8 8004CCF8 20104800 */  add        $v0, $v0, $t0 /* handwritten instruction */
    /* 3D4FC 8004CCFC 06106200 */  srlv       $v0, $v0, $v1
    /* 3D500 8004CD00 C2420200 */  srl        $t0, $v0, 11
    /* 3D504 8004CD04 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3D508 8004CD08 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3D50C 8004CD0C 07100201 */  srav       $v0, $v0, $t0
    /* 3D510 8004CD10 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3D514 8004CD14 1E04C84A */  ncs
    /* 3D518 8004CD18 04000C86 */  lh         $t4, 0x4($s0)
    /* 3D51C 8004CD1C 00000D86 */  lh         $t5, 0x0($s0)
    /* 3D520 8004CD20 00640C00 */  sll        $t4, $t4, 16
    /* 3D524 8004CD24 006C0D00 */  sll        $t5, $t5, 16
    /* 3D528 8004CD28 0400D6EA */  swc2       $22, 0x4($s6)
    /* 3D52C 8004CD2C 06000896 */  lhu        $t0, 0x6($s0)
    /* 3D530 8004CD30 08000996 */  lhu        $t1, 0x8($s0)
    /* 3D534 8004CD34 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3D538 8004CD38 0C000B96 */  lhu        $t3, 0xC($s0)
    /* 3D53C 8004CD3C 25400C01 */  or         $t0, $t0, $t4
    /* 3D540 8004CD40 25482D01 */  or         $t1, $t1, $t5
    /* 3D544 8004CD44 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3D548 8004CD48 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3D54C 8004CD4C 1C00CAA6 */  sh         $t2, 0x1C($s6)
    /* 3D550 8004CD50 2400CBA6 */  sh         $t3, 0x24($s6)
    /* 3D554 8004CD54 80100200 */  sll        $v0, $v0, 2
    /* 3D558 8004CD58 21105400 */  addu       $v0, $v0, $s4
    /* 3D55C 8004CD5C 0000498C */  lw         $t1, 0x0($v0)
    /* 3D560 8004CD60 2440DE02 */  and        $t0, $s6, $fp
    /* 3D564 8004CD64 000048AC */  sw         $t0, 0x0($v0)
    /* 3D568 8004CD68 25483501 */  or         $t1, $t1, $s5
    /* 3D56C 8004CD6C 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3D570 8004CD70 2800D626 */  addiu      $s6, $s6, 0x28
  .L8004CD74:
    /* 3D574 8004CD74 18001026 */  addiu      $s0, $s0, 0x18
    /* 3D578 8004CD78 9CFF6016 */  bnez       $s3, .L8004CBEC
    /* 3D57C 8004CD7C FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004CD80:
    /* 3D580 8004CD80 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3D584 8004CD84 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3D588 8004CD88 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3D58C 8004CD8C 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3D590 8004CD90 2400B68F */  lw         $s6, 0x24($sp)
    /* 3D594 8004CD94 2000B58F */  lw         $s5, 0x20($sp)
    /* 3D598 8004CD98 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3D59C 8004CD9C 1800B38F */  lw         $s3, 0x18($sp)
    /* 3D5A0 8004CDA0 1400B28F */  lw         $s2, 0x14($sp)
    /* 3D5A4 8004CDA4 1000B18F */  lw         $s1, 0x10($sp)
    /* 3D5A8 8004CDA8 0800E003 */  jr         $ra
    /* 3D5AC 8004CDAC 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004CB8C
