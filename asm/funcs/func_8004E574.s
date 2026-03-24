glabel func_8004E574
    /* 3ED74 8004E574 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3ED78 8004E578 3000BFAF */  sw         $ra, 0x30($sp)
    /* 3ED7C 8004E57C 2C00BEAF */  sw         $fp, 0x2C($sp)
    /* 3ED80 8004E580 2800B7AF */  sw         $s7, 0x28($sp)
    /* 3ED84 8004E584 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3ED88 8004E588 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3ED8C 8004E58C 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3ED90 8004E590 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3ED94 8004E594 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3ED98 8004E598 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3ED9C 8004E59C 84008010 */  beqz       $a0, .L8004E7B0
    /* 3EDA0 8004E5A0 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3EDA4 8004E5A4 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3EDA8 8004E5A8 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3EDAC 8004E5AC 801F123C */  lui        $s2, (0x1F800244 >> 16)
    /* 3EDB0 8004E5B0 44025236 */  ori        $s2, $s2, (0x1F800244 & 0xFFFF)
    /* 3EDB4 8004E5B4 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3EDB8 8004E5B8 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3EDBC 8004E5BC 0007153C */  lui        $s5, (0x7000000 >> 16)
    /* 3EDC0 8004E5C0 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3EDC4 8004E5C4 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3EDC8 8004E5C8 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3EDCC 8004E5CC B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3EDD0 8004E5D0 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3EDD4 8004E5D4 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
  .L8004E5D8:
    /* 3EDD8 8004E5D8 0C00048E */  lw         $a0, 0xC($s0)
    /* 3EDDC 8004E5DC 080006CA */  lwc2       $6, 0x8($s0)
    /* 3EDE0 8004E5E0 80280400 */  sll        $a1, $a0, 2
    /* 3EDE4 8004E5E4 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3EDE8 8004E5E8 21287101 */  addu       $a1, $t3, $s1
    /* 3EDEC 8004E5EC 0000A88C */  lw         $t0, 0x0($a1)
    /* 3EDF0 8004E5F0 82290400 */  srl        $a1, $a0, 6
    /* 3EDF4 8004E5F4 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3EDF8 8004E5F8 21289101 */  addu       $a1, $t4, $s1
    /* 3EDFC 8004E5FC 0000A98C */  lw         $t1, 0x0($a1)
    /* 3EE00 8004E600 822B0400 */  srl        $a1, $a0, 14
    /* 3EE04 8004E604 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3EE08 8004E608 2128B101 */  addu       $a1, $t5, $s1
    /* 3EE0C 8004E60C 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3EE10 8004E610 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3EE14 8004E614 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3EE18 8004E618 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3EE1C 8004E61C 24700901 */  and        $t6, $t0, $t1
    /* 3EE20 8004E620 2470CA01 */  and        $t6, $t6, $t2
    /* 3EE24 8004E624 0600404B */  nclip
    /* 3EE28 8004E628 5E00C005 */  bltz       $t6, .L8004E7A4
    /* 3EE2C 8004E62C 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3EE30 8004E630 0080CE31 */  andi       $t6, $t6, 0x8000
    /* 3EE34 8004E634 5B004104 */  bgez       $v0, .L8004E7A4
    /* 3EE38 8004E638 00000000 */   nop
    /* 3EE3C 8004E63C 5900C015 */  bnez       $t6, .L8004E7A4
    /* 3EE40 8004E640 42100B00 */   srl       $v0, $t3, 1
    /* 3EE44 8004E644 21105200 */  addu       $v0, $v0, $s2
    /* 3EE48 8004E648 00004484 */  lh         $a0, 0x0($v0)
    /* 3EE4C 8004E64C 42100C00 */  srl        $v0, $t4, 1
    /* 3EE50 8004E650 21105200 */  addu       $v0, $v0, $s2
    /* 3EE54 8004E654 00004584 */  lh         $a1, 0x0($v0)
    /* 3EE58 8004E658 42100D00 */  srl        $v0, $t5, 1
    /* 3EE5C 8004E65C 21105200 */  addu       $v0, $v0, $s2
    /* 3EE60 8004E660 00004684 */  lh         $a2, 0x0($v0)
    /* 3EE64 8004E664 00888448 */  mtc2       $a0, $17 /* handwritten instruction */
    /* 3EE68 8004E668 00908548 */  mtc2       $a1, $18 /* handwritten instruction */
    /* 3EE6C 8004E66C 00988648 */  mtc2       $a2, $19 /* handwritten instruction */
    /* 3EE70 8004E670 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3EE74 8004E674 00000000 */  nop
    /* 3EE78 8004E678 2D00584B */  avsz3
    /* 3EE7C 8004E67C 1000C9AE */  sw         $t1, 0x10($s6)
    /* 3EE80 8004E680 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 3EE84 8004E684 1800CAAE */  sw         $t2, 0x18($s6)
    /* 3EE88 8004E688 46004018 */  blez       $v0, .L8004E7A4
    /* 3EE8C 8004E68C 80100200 */   sll       $v0, $v0, 2
    /* 3EE90 8004E690 1000048E */  lw         $a0, 0x10($s0)
    /* 3EE94 8004E694 55050824 */  addiu      $t0, $zero, 0x555
    /* 3EE98 8004E698 00408848 */  mtc2       $t0, $8 /* handwritten instruction */
    /* 3EE9C 8004E69C 0F80083C */  lui        $t0, %hi(D_800F2B70)
    /* 3EEA0 8004E6A0 702B0825 */  addiu      $t0, $t0, %lo(D_800F2B70)
    /* 3EEA4 8004E6A4 82640400 */  srl        $t4, $a0, 18
    /* 3EEA8 8004E6A8 FC0F8C31 */  andi       $t4, $t4, 0xFFC
    /* 3EEAC 8004E6AC 20608801 */  add        $t4, $t4, $t0 /* handwritten instruction */
    /* 3EEB0 8004E6B0 00008C8D */  lw         $t4, 0x0($t4)
    /* 3EEB4 8004E6B4 00000000 */  nop
    /* 3EEB8 8004E6B8 02510C00 */  srl        $t2, $t4, 4
    /* 3EEBC 8004E6BC 025B0C00 */  srl        $t3, $t4, 12
    /* 3EEC0 8004E6C0 00490C00 */  sll        $t1, $t4, 4
    /* 3EEC4 8004E6C4 F00F2931 */  andi       $t1, $t1, 0xFF0
    /* 3EEC8 8004E6C8 F00F4A31 */  andi       $t2, $t2, 0xFF0
    /* 3EECC 8004E6CC F00F6B31 */  andi       $t3, $t3, 0xFF0
    /* 3EED0 8004E6D0 00488948 */  mtc2       $t1, $9 /* handwritten instruction */
    /* 3EED4 8004E6D4 00508A48 */  mtc2       $t2, $10 /* handwritten instruction */
    /* 3EED8 8004E6D8 00588B48 */  mtc2       $t3, $11 /* handwritten instruction */
    /* 3EEDC 8004E6DC 02620400 */  srl        $t4, $a0, 8
    /* 3EEE0 8004E6E0 FC0F8C31 */  andi       $t4, $t4, 0xFFC
    /* 3EEE4 8004E6E4 20608801 */  add        $t4, $t4, $t0 /* handwritten instruction */
    /* 3EEE8 8004E6E8 00008C8D */  lw         $t4, 0x0($t4)
    /* 3EEEC 8004E6EC 3D00984B */  gpf        1
    /* 3EEF0 8004E6F0 02510C00 */  srl        $t2, $t4, 4
    /* 3EEF4 8004E6F4 025B0C00 */  srl        $t3, $t4, 12
    /* 3EEF8 8004E6F8 00490C00 */  sll        $t1, $t4, 4
    /* 3EEFC 8004E6FC F00F2931 */  andi       $t1, $t1, 0xFF0
    /* 3EF00 8004E700 F00F4A31 */  andi       $t2, $t2, 0xFF0
    /* 3EF04 8004E704 F00F6B31 */  andi       $t3, $t3, 0xFF0
    /* 3EF08 8004E708 80600400 */  sll        $t4, $a0, 2
    /* 3EF0C 8004E70C FC0F8C31 */  andi       $t4, $t4, 0xFFC
    /* 3EF10 8004E710 00488948 */  mtc2       $t1, $9 /* handwritten instruction */
    /* 3EF14 8004E714 00508A48 */  mtc2       $t2, $10 /* handwritten instruction */
    /* 3EF18 8004E718 00588B48 */  mtc2       $t3, $11 /* handwritten instruction */
    /* 3EF1C 8004E71C 20608801 */  add        $t4, $t4, $t0 /* handwritten instruction */
    /* 3EF20 8004E720 00008C8D */  lw         $t4, 0x0($t4)
    /* 3EF24 8004E724 3E00A84B */  gpl        1
    /* 3EF28 8004E728 02510C00 */  srl        $t2, $t4, 4
    /* 3EF2C 8004E72C 025B0C00 */  srl        $t3, $t4, 12
    /* 3EF30 8004E730 00490C00 */  sll        $t1, $t4, 4
    /* 3EF34 8004E734 F00F2931 */  andi       $t1, $t1, 0xFF0
    /* 3EF38 8004E738 F00F4A31 */  andi       $t2, $t2, 0xFF0
    /* 3EF3C 8004E73C F00F6B31 */  andi       $t3, $t3, 0xFF0
    /* 3EF40 8004E740 00488948 */  mtc2       $t1, $9 /* handwritten instruction */
    /* 3EF44 8004E744 00508A48 */  mtc2       $t2, $10 /* handwritten instruction */
    /* 3EF48 8004E748 00588B48 */  mtc2       $t3, $11 /* handwritten instruction */
    /* 3EF4C 8004E74C 06106200 */  srlv       $v0, $v0, $v1
    /* 3EF50 8004E750 C2420200 */  srl        $t0, $v0, 11
    /* 3EF54 8004E754 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3EF58 8004E758 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3EF5C 8004E75C 07100201 */  srav       $v0, $v0, $t0
    /* 3EF60 8004E760 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3EF64 8004E764 3E00A84B */  gpl        1
    /* 3EF68 8004E768 0000088E */  lw         $t0, 0x0($s0)
    /* 3EF6C 8004E76C 0400098E */  lw         $t1, 0x4($s0)
    /* 3EF70 8004E770 08000A96 */  lhu        $t2, 0x8($s0)
    /* 3EF74 8004E774 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3EF78 8004E778 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3EF7C 8004E77C 1C00CAA6 */  sh         $t2, 0x1C($s6)
    /* 3EF80 8004E780 80100200 */  sll        $v0, $v0, 2
    /* 3EF84 8004E784 21105400 */  addu       $v0, $v0, $s4
    /* 3EF88 8004E788 0000498C */  lw         $t1, 0x0($v0)
    /* 3EF8C 8004E78C 2440DE02 */  and        $t0, $s6, $fp
    /* 3EF90 8004E790 000048AC */  sw         $t0, 0x0($v0)
    /* 3EF94 8004E794 25483501 */  or         $t1, $t1, $s5
    /* 3EF98 8004E798 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3EF9C 8004E79C 0400D6EA */  swc2       $22, 0x4($s6)
    /* 3EFA0 8004E7A0 2000D626 */  addiu      $s6, $s6, 0x20
  .L8004E7A4:
    /* 3EFA4 8004E7A4 14001026 */  addiu      $s0, $s0, 0x14
    /* 3EFA8 8004E7A8 8BFF6016 */  bnez       $s3, .L8004E5D8
    /* 3EFAC 8004E7AC FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004E7B0:
    /* 3EFB0 8004E7B0 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3EFB4 8004E7B4 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3EFB8 8004E7B8 3000BF8F */  lw         $ra, 0x30($sp)
    /* 3EFBC 8004E7BC 2C00BE8F */  lw         $fp, 0x2C($sp)
    /* 3EFC0 8004E7C0 2800B78F */  lw         $s7, 0x28($sp)
    /* 3EFC4 8004E7C4 2400B68F */  lw         $s6, 0x24($sp)
    /* 3EFC8 8004E7C8 2000B58F */  lw         $s5, 0x20($sp)
    /* 3EFCC 8004E7CC 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3EFD0 8004E7D0 1800B38F */  lw         $s3, 0x18($sp)
    /* 3EFD4 8004E7D4 1400B28F */  lw         $s2, 0x14($sp)
    /* 3EFD8 8004E7D8 1000B18F */  lw         $s1, 0x10($sp)
    /* 3EFDC 8004E7DC 0800E003 */  jr         $ra
    /* 3EFE0 8004E7E0 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004E574
