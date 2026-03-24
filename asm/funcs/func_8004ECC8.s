glabel func_8004ECC8
    /* 3F4C8 8004ECC8 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3F4CC 8004ECCC 3000BFAF */  sw         $ra, 0x30($sp)
    /* 3F4D0 8004ECD0 2C00BEAF */  sw         $fp, 0x2C($sp)
    /* 3F4D4 8004ECD4 2800B7AF */  sw         $s7, 0x28($sp)
    /* 3F4D8 8004ECD8 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3F4DC 8004ECDC 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3F4E0 8004ECE0 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3F4E4 8004ECE4 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3F4E8 8004ECE8 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3F4EC 8004ECEC 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3F4F0 8004ECF0 7A008010 */  beqz       $a0, .L8004EEDC
    /* 3F4F4 8004ECF4 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3F4F8 8004ECF8 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3F4FC 8004ECFC 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3F500 8004ED00 801F123C */  lui        $s2, (0x1F800244 >> 16)
    /* 3F504 8004ED04 44025236 */  ori        $s2, $s2, (0x1F800244 & 0xFFFF)
    /* 3F508 8004ED08 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3F50C 8004ED0C 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3F510 8004ED10 000C153C */  lui        $s5, (0xC000000 >> 16)
    /* 3F514 8004ED14 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3F518 8004ED18 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3F51C 8004ED1C 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3F520 8004ED20 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3F524 8004ED24 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3F528 8004ED28 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
  .L8004ED2C:
    /* 3F52C 8004ED2C 1000048E */  lw         $a0, 0x10($s0)
    /* 3F530 8004ED30 00000000 */  nop
    /* 3F534 8004ED34 80280400 */  sll        $a1, $a0, 2
    /* 3F538 8004ED38 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3F53C 8004ED3C 21287101 */  addu       $a1, $t3, $s1
    /* 3F540 8004ED40 0000A88C */  lw         $t0, 0x0($a1)
    /* 3F544 8004ED44 82290400 */  srl        $a1, $a0, 6
    /* 3F548 8004ED48 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3F54C 8004ED4C 21289101 */  addu       $a1, $t4, $s1
    /* 3F550 8004ED50 0000A98C */  lw         $t1, 0x0($a1)
    /* 3F554 8004ED54 822B0400 */  srl        $a1, $a0, 14
    /* 3F558 8004ED58 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3F55C 8004ED5C 2128B101 */  addu       $a1, $t5, $s1
    /* 3F560 8004ED60 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3F564 8004ED64 822D0400 */  srl        $a1, $a0, 22
    /* 3F568 8004ED68 FC03AE30 */  andi       $t6, $a1, 0x3FC
    /* 3F56C 8004ED6C 2128D101 */  addu       $a1, $t6, $s1
    /* 3F570 8004ED70 0000AF8C */  lw         $t7, 0x0($a1)
    /* 3F574 8004ED74 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3F578 8004ED78 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3F57C 8004ED7C 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3F580 8004ED80 24C00901 */  and        $t8, $t0, $t1
    /* 3F584 8004ED84 24C00A03 */  and        $t8, $t8, $t2
    /* 3F588 8004ED88 24C00F03 */  and        $t8, $t8, $t7
    /* 3F58C 8004ED8C 0600404B */  nclip
    /* 3F590 8004ED90 4F000007 */  bltz       $t8, .L8004EED0
    /* 3F594 8004ED94 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3F598 8004ED98 00801833 */  andi       $t8, $t8, 0x8000
    /* 3F59C 8004ED9C 4C004104 */  bgez       $v0, .L8004EED0
    /* 3F5A0 8004EDA0 00000000 */   nop
    /* 3F5A4 8004EDA4 4A000017 */  bnez       $t8, .L8004EED0
    /* 3F5A8 8004EDA8 42100B00 */   srl       $v0, $t3, 1
    /* 3F5AC 8004EDAC 21105200 */  addu       $v0, $v0, $s2
    /* 3F5B0 8004EDB0 00004484 */  lh         $a0, 0x0($v0)
    /* 3F5B4 8004EDB4 42100C00 */  srl        $v0, $t4, 1
    /* 3F5B8 8004EDB8 21105200 */  addu       $v0, $v0, $s2
    /* 3F5BC 8004EDBC 00004584 */  lh         $a1, 0x0($v0)
    /* 3F5C0 8004EDC0 42100D00 */  srl        $v0, $t5, 1
    /* 3F5C4 8004EDC4 21105200 */  addu       $v0, $v0, $s2
    /* 3F5C8 8004EDC8 00004684 */  lh         $a2, 0x0($v0)
    /* 3F5CC 8004EDCC 42100E00 */  srl        $v0, $t6, 1
    /* 3F5D0 8004EDD0 21105200 */  addu       $v0, $v0, $s2
    /* 3F5D4 8004EDD4 00004784 */  lh         $a3, 0x0($v0)
    /* 3F5D8 8004EDD8 20208500 */  add        $a0, $a0, $a1 /* handwritten instruction */
    /* 3F5DC 8004EDDC 20208600 */  add        $a0, $a0, $a2 /* handwritten instruction */
    /* 3F5E0 8004EDE0 20208700 */  add        $a0, $a0, $a3 /* handwritten instruction */
    /* 3F5E4 8004EDE4 83100400 */  sra        $v0, $a0, 2
    /* 3F5E8 8004EDE8 39004018 */  blez       $v0, .L8004EED0
    /* 3F5EC 8004EDEC 00000000 */   nop
    /* 3F5F0 8004EDF0 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3F5F4 8004EDF4 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3F5F8 8004EDF8 2000CAAE */  sw         $t2, 0x20($s6)
    /* 3F5FC 8004EDFC 2C00CFAE */  sw         $t7, 0x2C($s6)
    /* 3F600 8004EE00 1400068E */  lw         $a2, 0x14($s0)
    /* 3F604 8004EE04 0E000786 */  lh         $a3, 0xE($s0)
    /* 3F608 8004EE08 2120C000 */  addu       $a0, $a2, $zero
    /* 3F60C 8004EE0C FF038430 */  andi       $a0, $a0, 0x3FF
    /* 3F610 8004EE10 82320600 */  srl        $a2, $a2, 10
    /* 3F614 8004EE14 2128C000 */  addu       $a1, $a2, $zero
    /* 3F618 8004EE18 FF03A530 */  andi       $a1, $a1, 0x3FF
    /* 3F61C 8004EE1C 82320600 */  srl        $a2, $a2, 10
    /* 3F620 8004EE20 FF03C630 */  andi       $a2, $a2, 0x3FF
    /* 3F624 8004EE24 0F80083C */  lui        $t0, %hi(D_800F2B70)
    /* 3F628 8004EE28 702B0825 */  addiu      $t0, $t0, %lo(D_800F2B70)
    /* 3F62C 8004EE2C 80200400 */  sll        $a0, $a0, 2
    /* 3F630 8004EE30 80280500 */  sll        $a1, $a1, 2
    /* 3F634 8004EE34 80300600 */  sll        $a2, $a2, 2
    /* 3F638 8004EE38 80380700 */  sll        $a3, $a3, 2
    /* 3F63C 8004EE3C 20208800 */  add        $a0, $a0, $t0 /* handwritten instruction */
    /* 3F640 8004EE40 2028A800 */  add        $a1, $a1, $t0 /* handwritten instruction */
    /* 3F644 8004EE44 2030C800 */  add        $a2, $a2, $t0 /* handwritten instruction */
    /* 3F648 8004EE48 2038E800 */  add        $a3, $a3, $t0 /* handwritten instruction */
    /* 3F64C 8004EE4C 0000888C */  lw         $t0, 0x0($a0)
    /* 3F650 8004EE50 0000A98C */  lw         $t1, 0x0($a1)
    /* 3F654 8004EE54 0000CA8C */  lw         $t2, 0x0($a2)
    /* 3F658 8004EE58 0000EB8C */  lw         $t3, 0x0($a3)
    /* 3F65C 8004EE5C 003C043C */  lui        $a0, (0x3C000000 >> 16)
    /* 3F660 8004EE60 25400401 */  or         $t0, $t0, $a0
    /* 3F664 8004EE64 0400C8AE */  sw         $t0, 0x4($s6)
    /* 3F668 8004EE68 1000C9AE */  sw         $t1, 0x10($s6)
    /* 3F66C 8004EE6C 1C00CAAE */  sw         $t2, 0x1C($s6)
    /* 3F670 8004EE70 2800CBAE */  sw         $t3, 0x28($s6)
    /* 3F674 8004EE74 06106200 */  srlv       $v0, $v0, $v1
    /* 3F678 8004EE78 C2420200 */  srl        $t0, $v0, 11
    /* 3F67C 8004EE7C FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3F680 8004EE80 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3F684 8004EE84 07100201 */  srav       $v0, $v0, $t0
    /* 3F688 8004EE88 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3F68C 8004EE8C 0000088E */  lw         $t0, 0x0($s0)
    /* 3F690 8004EE90 0400098E */  lw         $t1, 0x4($s0)
    /* 3F694 8004EE94 08000A86 */  lh         $t2, 0x8($s0)
    /* 3F698 8004EE98 0C000B86 */  lh         $t3, 0xC($s0)
    /* 3F69C 8004EE9C 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3F6A0 8004EEA0 1800C9AE */  sw         $t1, 0x18($s6)
    /* 3F6A4 8004EEA4 2400CAA6 */  sh         $t2, 0x24($s6)
    /* 3F6A8 8004EEA8 3000CBA6 */  sh         $t3, 0x30($s6)
    /* 3F6AC 8004EEAC 80100200 */  sll        $v0, $v0, 2
    /* 3F6B0 8004EEB0 21105400 */  addu       $v0, $v0, $s4
    /* 3F6B4 8004EEB4 0000498C */  lw         $t1, 0x0($v0)
    /* 3F6B8 8004EEB8 2440DE02 */  and        $t0, $s6, $fp
    /* 3F6BC 8004EEBC 000048AC */  sw         $t0, 0x0($v0)
    /* 3F6C0 8004EEC0 25483501 */  or         $t1, $t1, $s5
    /* 3F6C4 8004EEC4 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3F6C8 8004EEC8 00000000 */  nop
    /* 3F6CC 8004EECC 3400D626 */  addiu      $s6, $s6, 0x34
  .L8004EED0:
    /* 3F6D0 8004EED0 18001026 */  addiu      $s0, $s0, 0x18
    /* 3F6D4 8004EED4 95FF6016 */  bnez       $s3, .L8004ED2C
    /* 3F6D8 8004EED8 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004EEDC:
    /* 3F6DC 8004EEDC 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3F6E0 8004EEE0 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3F6E4 8004EEE4 3000BF8F */  lw         $ra, 0x30($sp)
    /* 3F6E8 8004EEE8 2C00BE8F */  lw         $fp, 0x2C($sp)
    /* 3F6EC 8004EEEC 2800B78F */  lw         $s7, 0x28($sp)
    /* 3F6F0 8004EEF0 2400B68F */  lw         $s6, 0x24($sp)
    /* 3F6F4 8004EEF4 2000B58F */  lw         $s5, 0x20($sp)
    /* 3F6F8 8004EEF8 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3F6FC 8004EEFC 1800B38F */  lw         $s3, 0x18($sp)
    /* 3F700 8004EF00 1400B28F */  lw         $s2, 0x14($sp)
    /* 3F704 8004EF04 1000B18F */  lw         $s1, 0x10($sp)
    /* 3F708 8004EF08 0800E003 */  jr         $ra
    /* 3F70C 8004EF0C 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004ECC8
