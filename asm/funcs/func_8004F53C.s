glabel func_8004F53C
    /* 3FD3C 8004F53C C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3FD40 8004F540 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3FD44 8004F544 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3FD48 8004F548 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3FD4C 8004F54C 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3FD50 8004F550 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3FD54 8004F554 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3FD58 8004F558 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3FD5C 8004F55C 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3FD60 8004F560 81008010 */  beqz       $a0, .L8004F768
    /* 3FD64 8004F564 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3FD68 8004F568 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3FD6C 8004F56C 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3FD70 8004F570 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3FD74 8004F574 B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3FD78 8004F578 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3FD7C 8004F57C 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3FD80 8004F580 000C153C */  lui        $s5, (0xC000000 >> 16)
    /* 3FD84 8004F584 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3FD88 8004F588 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3FD8C 8004F58C 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3FD90 8004F590 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3FD94 8004F594 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3FD98 8004F598 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
    /* 3FD9C 8004F59C 000006CA */  lwc2       $6, 0x0($s0)
  .L8004F5A0:
    /* 3FDA0 8004F5A0 1000048E */  lw         $a0, 0x10($s0)
    /* 3FDA4 8004F5A4 00000000 */  nop
    /* 3FDA8 8004F5A8 80280400 */  sll        $a1, $a0, 2
    /* 3FDAC 8004F5AC FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3FDB0 8004F5B0 21287101 */  addu       $a1, $t3, $s1
    /* 3FDB4 8004F5B4 0000A88C */  lw         $t0, 0x0($a1)
    /* 3FDB8 8004F5B8 82290400 */  srl        $a1, $a0, 6
    /* 3FDBC 8004F5BC FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3FDC0 8004F5C0 21289101 */  addu       $a1, $t4, $s1
    /* 3FDC4 8004F5C4 0000A98C */  lw         $t1, 0x0($a1)
    /* 3FDC8 8004F5C8 822B0400 */  srl        $a1, $a0, 14
    /* 3FDCC 8004F5CC FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3FDD0 8004F5D0 2128B101 */  addu       $a1, $t5, $s1
    /* 3FDD4 8004F5D4 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3FDD8 8004F5D8 822D0400 */  srl        $a1, $a0, 22
    /* 3FDDC 8004F5DC FC03AE30 */  andi       $t6, $a1, 0x3FC
    /* 3FDE0 8004F5E0 2128D101 */  addu       $a1, $t6, $s1
    /* 3FDE4 8004F5E4 0000AF8C */  lw         $t7, 0x0($a1)
    /* 3FDE8 8004F5E8 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3FDEC 8004F5EC 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3FDF0 8004F5F0 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3FDF4 8004F5F4 24C00901 */  and        $t8, $t0, $t1
    /* 3FDF8 8004F5F8 24C00A03 */  and        $t8, $t8, $t2
    /* 3FDFC 8004F5FC 24C00F03 */  and        $t8, $t8, $t7
    /* 3FE00 8004F600 0600404B */  nclip
    /* 3FE04 8004F604 55000007 */  bltz       $t8, .L8004F75C
    /* 3FE08 8004F608 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3FE0C 8004F60C 00801833 */  andi       $t8, $t8, 0x8000
    /* 3FE10 8004F610 52000017 */  bnez       $t8, .L8004F75C
    /* 3FE14 8004F614 2A384000 */   slt       $a3, $v0, $zero
    /* 3FE18 8004F618 FFFFE720 */  addi       $a3, $a3, -0x1 /* handwritten instruction */
    /* 3FE1C 8004F61C 42100B00 */  srl        $v0, $t3, 1
    /* 3FE20 8004F620 21105200 */  addu       $v0, $v0, $s2
    /* 3FE24 8004F624 00004484 */  lh         $a0, 0x0($v0)
    /* 3FE28 8004F628 42100C00 */  srl        $v0, $t4, 1
    /* 3FE2C 8004F62C 21105200 */  addu       $v0, $v0, $s2
    /* 3FE30 8004F630 00004584 */  lh         $a1, 0x0($v0)
    /* 3FE34 8004F634 42100D00 */  srl        $v0, $t5, 1
    /* 3FE38 8004F638 21105200 */  addu       $v0, $v0, $s2
    /* 3FE3C 8004F63C 00004684 */  lh         $a2, 0x0($v0)
    /* 3FE40 8004F640 42100E00 */  srl        $v0, $t6, 1
    /* 3FE44 8004F644 21105200 */  addu       $v0, $v0, $s2
    /* 3FE48 8004F648 00004B84 */  lh         $t3, 0x0($v0)
    /* 3FE4C 8004F64C 20208500 */  add        $a0, $a0, $a1 /* handwritten instruction */
    /* 3FE50 8004F650 20208600 */  add        $a0, $a0, $a2 /* handwritten instruction */
    /* 3FE54 8004F654 20208B00 */  add        $a0, $a0, $t3 /* handwritten instruction */
    /* 3FE58 8004F658 83100400 */  sra        $v0, $a0, 2
    /* 3FE5C 8004F65C 3F004018 */  blez       $v0, .L8004F75C
    /* 3FE60 8004F660 0100013C */   lui       $at, (0x10000 >> 16)
    /* 3FE64 8004F664 2AC84100 */  slt        $t9, $v0, $at
    /* 3FE68 8004F668 3C002013 */  beqz       $t9, .L8004F75C
    /* 3FE6C 8004F66C 00000000 */   nop
    /* 3FE70 8004F670 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3FE74 8004F674 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3FE78 8004F678 2000CAAE */  sw         $t2, 0x20($s6)
    /* 3FE7C 8004F67C 2C00CFAE */  sw         $t7, 0x2C($s6)
    /* 3FE80 8004F680 14000A8E */  lw         $t2, 0x14($s0)
    /* 3FE84 8004F684 1800098E */  lw         $t1, 0x18($s0)
    /* 3FE88 8004F688 26504701 */  xor        $t2, $t2, $a3
    /* 3FE8C 8004F68C 26482701 */  xor        $t1, $t1, $a3
    /* 3FE90 8004F690 00008A48 */  mtc2       $t2, $0 /* handwritten instruction */
    /* 3FE94 8004F694 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 3FE98 8004F698 06106200 */  srlv       $v0, $v0, $v1
    /* 3FE9C 8004F69C C2420200 */  srl        $t0, $v0, 11
    /* 3FEA0 8004F6A0 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3FEA4 8004F6A4 1E04C84A */  ncs
    /* 3FEA8 8004F6A8 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3FEAC 8004F6AC 07100201 */  srav       $v0, $v0, $t0
    /* 3FEB0 8004F6B0 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3FEB4 8004F6B4 0400D6EA */  swc2       $22, 0x4($s6)
    /* 3FEB8 8004F6B8 1C00088E */  lw         $t0, 0x1C($s0)
    /* 3FEBC 8004F6BC 024C0900 */  srl        $t1, $t1, 16
    /* 3FEC0 8004F6C0 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 3FEC4 8004F6C4 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 3FEC8 8004F6C8 20000A8E */  lw         $t2, 0x20($s0)
    /* 3FECC 8004F6CC 2400098E */  lw         $t1, 0x24($s0)
    /* 3FED0 8004F6D0 26504701 */  xor        $t2, $t2, $a3
    /* 3FED4 8004F6D4 26482701 */  xor        $t1, $t1, $a3
    /* 3FED8 8004F6D8 00108A48 */  mtc2       $t2, $2 /* handwritten instruction */
    /* 3FEDC 8004F6DC 00188948 */  mtc2       $t1, $3 /* handwritten instruction */
    /* 3FEE0 8004F6E0 2800088E */  lw         $t0, 0x28($s0)
    /* 3FEE4 8004F6E4 024C0900 */  srl        $t1, $t1, 16
    /* 3FEE8 8004F6E8 00208848 */  mtc2       $t0, $4 /* handwritten instruction */
    /* 3FEEC 8004F6EC 00288948 */  mtc2       $t1, $5 /* handwritten instruction */
    /* 3FEF0 8004F6F0 04000C86 */  lh         $t4, 0x4($s0)
    /* 3FEF4 8004F6F4 00000D86 */  lh         $t5, 0x0($s0)
    /* 3FEF8 8004F6F8 00640C00 */  sll        $t4, $t4, 16
    /* 3FEFC 8004F6FC 006C0D00 */  sll        $t5, $t5, 16
    /* 3FF00 8004F700 2004D84A */  nct
    /* 3FF04 8004F704 06000896 */  lhu        $t0, 0x6($s0)
    /* 3FF08 8004F708 08000996 */  lhu        $t1, 0x8($s0)
    /* 3FF0C 8004F70C 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3FF10 8004F710 0C000B96 */  lhu        $t3, 0xC($s0)
    /* 3FF14 8004F714 25400C01 */  or         $t0, $t0, $t4
    /* 3FF18 8004F718 25482D01 */  or         $t1, $t1, $t5
    /* 3FF1C 8004F71C 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3FF20 8004F720 1800C9AE */  sw         $t1, 0x18($s6)
    /* 3FF24 8004F724 2400CAA6 */  sh         $t2, 0x24($s6)
    /* 3FF28 8004F728 3000CBA6 */  sh         $t3, 0x30($s6)
    /* 3FF2C 8004F72C 80100200 */  sll        $v0, $v0, 2
    /* 3FF30 8004F730 21105400 */  addu       $v0, $v0, $s4
    /* 3FF34 8004F734 0000498C */  lw         $t1, 0x0($v0)
    /* 3FF38 8004F738 2440DE02 */  and        $t0, $s6, $fp
    /* 3FF3C 8004F73C 000048AC */  sw         $t0, 0x0($v0)
    /* 3FF40 8004F740 25483501 */  or         $t1, $t1, $s5
    /* 3FF44 8004F744 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3FF48 8004F748 1000D4EA */  swc2       $20, 0x10($s6)
    /* 3FF4C 8004F74C 1C00D5EA */  swc2       $21, 0x1C($s6)
    /* 3FF50 8004F750 2800D6EA */  swc2       $22, 0x28($s6)
    /* 3FF54 8004F754 00000000 */  nop
    /* 3FF58 8004F758 3400D626 */  addiu      $s6, $s6, 0x34
  .L8004F75C:
    /* 3FF5C 8004F75C 2C001026 */  addiu      $s0, $s0, 0x2C
    /* 3FF60 8004F760 8FFF6016 */  bnez       $s3, .L8004F5A0
    /* 3FF64 8004F764 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004F768:
    /* 3FF68 8004F768 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3FF6C 8004F76C B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3FF70 8004F770 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3FF74 8004F774 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3FF78 8004F778 2400B68F */  lw         $s6, 0x24($sp)
    /* 3FF7C 8004F77C 2000B58F */  lw         $s5, 0x20($sp)
    /* 3FF80 8004F780 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3FF84 8004F784 1800B38F */  lw         $s3, 0x18($sp)
    /* 3FF88 8004F788 1400B28F */  lw         $s2, 0x14($sp)
    /* 3FF8C 8004F78C 1000B18F */  lw         $s1, 0x10($sp)
    /* 3FF90 8004F790 0800E003 */  jr         $ra
    /* 3FF94 8004F794 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004F53C
