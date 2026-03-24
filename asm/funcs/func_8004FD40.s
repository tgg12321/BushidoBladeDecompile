glabel func_8004FD40
    /* 40540 8004FD40 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 40544 8004FD44 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 40548 8004FD48 2800BEAF */  sw         $fp, 0x28($sp)
    /* 4054C 8004FD4C 2400B6AF */  sw         $s6, 0x24($sp)
    /* 40550 8004FD50 2000B5AF */  sw         $s5, 0x20($sp)
    /* 40554 8004FD54 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 40558 8004FD58 1800B3AF */  sw         $s3, 0x18($sp)
    /* 4055C 8004FD5C 1400B2AF */  sw         $s2, 0x14($sp)
    /* 40560 8004FD60 1000B1AF */  sw         $s1, 0x10($sp)
    /* 40564 8004FD64 6A008010 */  beqz       $a0, .L8004FF10
    /* 40568 8004FD68 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 4056C 8004FD6C 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 40570 8004FD70 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 40574 8004FD74 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 40578 8004FD78 B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 4057C 8004FD7C 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 40580 8004FD80 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 40584 8004FD84 0009153C */  lui        $s5, (0x9000000 >> 16)
    /* 40588 8004FD88 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 4058C 8004FD8C FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 40590 8004FD90 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 40594 8004FD94 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 40598 8004FD98 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 4059C 8004FD9C 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
  .L8004FDA0:
    /* 405A0 8004FDA0 0E000496 */  lhu        $a0, 0xE($s0)
    /* 405A4 8004FDA4 10000596 */  lhu        $a1, 0x10($s0)
    /* 405A8 8004FDA8 000006CA */  lwc2       $6, 0x0($s0)
    /* 405AC 8004FDAC 002C0500 */  sll        $a1, $a1, 16
    /* 405B0 8004FDB0 25208500 */  or         $a0, $a0, $a1
    /* 405B4 8004FDB4 80280400 */  sll        $a1, $a0, 2
    /* 405B8 8004FDB8 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 405BC 8004FDBC 21287101 */  addu       $a1, $t3, $s1
    /* 405C0 8004FDC0 0000A88C */  lw         $t0, 0x0($a1)
    /* 405C4 8004FDC4 82290400 */  srl        $a1, $a0, 6
    /* 405C8 8004FDC8 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 405CC 8004FDCC 21289101 */  addu       $a1, $t4, $s1
    /* 405D0 8004FDD0 0000A98C */  lw         $t1, 0x0($a1)
    /* 405D4 8004FDD4 822B0400 */  srl        $a1, $a0, 14
    /* 405D8 8004FDD8 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 405DC 8004FDDC 2128B101 */  addu       $a1, $t5, $s1
    /* 405E0 8004FDE0 0000AA8C */  lw         $t2, 0x0($a1)
    /* 405E4 8004FDE4 822D0400 */  srl        $a1, $a0, 22
    /* 405E8 8004FDE8 FC03AE30 */  andi       $t6, $a1, 0x3FC
    /* 405EC 8004FDEC 2128D101 */  addu       $a1, $t6, $s1
    /* 405F0 8004FDF0 0000AF8C */  lw         $t7, 0x0($a1)
    /* 405F4 8004FDF4 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 405F8 8004FDF8 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 405FC 8004FDFC 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 40600 8004FE00 24C00901 */  and        $t8, $t0, $t1
    /* 40604 8004FE04 24C00A03 */  and        $t8, $t8, $t2
    /* 40608 8004FE08 24C00F03 */  and        $t8, $t8, $t7
    /* 4060C 8004FE0C 0600404B */  nclip
    /* 40610 8004FE10 3C000007 */  bltz       $t8, .L8004FF04
    /* 40614 8004FE14 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 40618 8004FE18 00801833 */  andi       $t8, $t8, 0x8000
    /* 4061C 8004FE1C 39004104 */  bgez       $v0, .L8004FF04
    /* 40620 8004FE20 00000000 */   nop
    /* 40624 8004FE24 37000017 */  bnez       $t8, .L8004FF04
    /* 40628 8004FE28 42100B00 */   srl       $v0, $t3, 1
    /* 4062C 8004FE2C 21105200 */  addu       $v0, $v0, $s2
    /* 40630 8004FE30 00004484 */  lh         $a0, 0x0($v0)
    /* 40634 8004FE34 42100C00 */  srl        $v0, $t4, 1
    /* 40638 8004FE38 21105200 */  addu       $v0, $v0, $s2
    /* 4063C 8004FE3C 00004584 */  lh         $a1, 0x0($v0)
    /* 40640 8004FE40 42100D00 */  srl        $v0, $t5, 1
    /* 40644 8004FE44 21105200 */  addu       $v0, $v0, $s2
    /* 40648 8004FE48 00004684 */  lh         $a2, 0x0($v0)
    /* 4064C 8004FE4C 42100E00 */  srl        $v0, $t6, 1
    /* 40650 8004FE50 21105200 */  addu       $v0, $v0, $s2
    /* 40654 8004FE54 00004784 */  lh         $a3, 0x0($v0)
    /* 40658 8004FE58 20208500 */  add        $a0, $a0, $a1 /* handwritten instruction */
    /* 4065C 8004FE5C 20208600 */  add        $a0, $a0, $a2 /* handwritten instruction */
    /* 40660 8004FE60 20208700 */  add        $a0, $a0, $a3 /* handwritten instruction */
    /* 40664 8004FE64 83100400 */  sra        $v0, $a0, 2
    /* 40668 8004FE68 26004018 */  blez       $v0, .L8004FF04
    /* 4066C 8004FE6C 00000000 */   nop
    /* 40670 8004FE70 0800C8AE */  sw         $t0, 0x8($s6)
    /* 40674 8004FE74 1000C9AE */  sw         $t1, 0x10($s6)
    /* 40678 8004FE78 1800CAAE */  sw         $t2, 0x18($s6)
    /* 4067C 8004FE7C 2000CFAE */  sw         $t7, 0x20($s6)
    /* 40680 8004FE80 12000896 */  lhu        $t0, 0x12($s0)
    /* 40684 8004FE84 140000CA */  lwc2       $0, 0x14($s0)
    /* 40688 8004FE88 00088848 */  mtc2       $t0, $1 /* handwritten instruction */
    /* 4068C 8004FE8C 06106200 */  srlv       $v0, $v0, $v1
    /* 40690 8004FE90 C2420200 */  srl        $t0, $v0, 11
    /* 40694 8004FE94 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 40698 8004FE98 1E04C84A */  ncs
    /* 4069C 8004FE9C 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 406A0 8004FEA0 07100201 */  srav       $v0, $v0, $t0
    /* 406A4 8004FEA4 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 406A8 8004FEA8 04000C86 */  lh         $t4, 0x4($s0)
    /* 406AC 8004FEAC 00000D86 */  lh         $t5, 0x0($s0)
    /* 406B0 8004FEB0 00640C00 */  sll        $t4, $t4, 16
    /* 406B4 8004FEB4 006C0D00 */  sll        $t5, $t5, 16
    /* 406B8 8004FEB8 06000896 */  lhu        $t0, 0x6($s0)
    /* 406BC 8004FEBC 08000996 */  lhu        $t1, 0x8($s0)
    /* 406C0 8004FEC0 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 406C4 8004FEC4 0C000B96 */  lhu        $t3, 0xC($s0)
    /* 406C8 8004FEC8 25400C01 */  or         $t0, $t0, $t4
    /* 406CC 8004FECC 25482D01 */  or         $t1, $t1, $t5
    /* 406D0 8004FED0 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 406D4 8004FED4 1400C9AE */  sw         $t1, 0x14($s6)
    /* 406D8 8004FED8 1C00CAA6 */  sh         $t2, 0x1C($s6)
    /* 406DC 8004FEDC 2400CBA6 */  sh         $t3, 0x24($s6)
    /* 406E0 8004FEE0 0400D6EA */  swc2       $22, 0x4($s6)
    /* 406E4 8004FEE4 80100200 */  sll        $v0, $v0, 2
    /* 406E8 8004FEE8 21105400 */  addu       $v0, $v0, $s4
    /* 406EC 8004FEEC 0000498C */  lw         $t1, 0x0($v0)
    /* 406F0 8004FEF0 2440DE02 */  and        $t0, $s6, $fp
    /* 406F4 8004FEF4 000048AC */  sw         $t0, 0x0($v0)
    /* 406F8 8004FEF8 25483501 */  or         $t1, $t1, $s5
    /* 406FC 8004FEFC 0000C9AE */  sw         $t1, 0x0($s6)
    /* 40700 8004FF00 2800D626 */  addiu      $s6, $s6, 0x28
  .L8004FF04:
    /* 40704 8004FF04 18001026 */  addiu      $s0, $s0, 0x18
    /* 40708 8004FF08 A5FF6016 */  bnez       $s3, .L8004FDA0
    /* 4070C 8004FF0C FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004FF10:
    /* 40710 8004FF10 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 40714 8004FF14 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 40718 8004FF18 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 4071C 8004FF1C 2800BE8F */  lw         $fp, 0x28($sp)
    /* 40720 8004FF20 2400B68F */  lw         $s6, 0x24($sp)
    /* 40724 8004FF24 2000B58F */  lw         $s5, 0x20($sp)
    /* 40728 8004FF28 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 4072C 8004FF2C 1800B38F */  lw         $s3, 0x18($sp)
    /* 40730 8004FF30 1400B28F */  lw         $s2, 0x14($sp)
    /* 40734 8004FF34 1000B18F */  lw         $s1, 0x10($sp)
    /* 40738 8004FF38 0800E003 */  jr         $ra
    /* 4073C 8004FF3C 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004FD40
