glabel func_8004CDB0
    /* 3D5B0 8004CDB0 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3D5B4 8004CDB4 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3D5B8 8004CDB8 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3D5BC 8004CDBC 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3D5C0 8004CDC0 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3D5C4 8004CDC4 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3D5C8 8004CDC8 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3D5CC 8004CDCC 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3D5D0 8004CDD0 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3D5D4 8004CDD4 76008010 */  beqz       $a0, .L8004CFB0
    /* 3D5D8 8004CDD8 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3D5DC 8004CDDC 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3D5E0 8004CDE0 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3D5E4 8004CDE4 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3D5E8 8004CDE8 B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3D5EC 8004CDEC 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3D5F0 8004CDF0 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3D5F4 8004CDF4 0009153C */  lui        $s5, (0x9000000 >> 16)
    /* 3D5F8 8004CDF8 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3D5FC 8004CDFC FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3D600 8004CE00 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3D604 8004CE04 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3D608 8004CE08 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3D60C 8004CE0C 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
    /* 3D610 8004CE10 000006CA */  lwc2       $6, 0x0($s0)
  .L8004CE14:
    /* 3D614 8004CE14 0C00048E */  lw         $a0, 0xC($s0)
    /* 3D618 8004CE18 00000000 */  nop
    /* 3D61C 8004CE1C 80280400 */  sll        $a1, $a0, 2
    /* 3D620 8004CE20 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3D624 8004CE24 21287101 */  addu       $a1, $t3, $s1
    /* 3D628 8004CE28 0000A88C */  lw         $t0, 0x0($a1)
    /* 3D62C 8004CE2C 82290400 */  srl        $a1, $a0, 6
    /* 3D630 8004CE30 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3D634 8004CE34 21289101 */  addu       $a1, $t4, $s1
    /* 3D638 8004CE38 0000A98C */  lw         $t1, 0x0($a1)
    /* 3D63C 8004CE3C 822B0400 */  srl        $a1, $a0, 14
    /* 3D640 8004CE40 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3D644 8004CE44 2128B101 */  addu       $a1, $t5, $s1
    /* 3D648 8004CE48 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3D64C 8004CE4C 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3D650 8004CE50 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3D654 8004CE54 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3D658 8004CE58 24700901 */  and        $t6, $t0, $t1
    /* 3D65C 8004CE5C 2470CA01 */  and        $t6, $t6, $t2
    /* 3D660 8004CE60 0600404B */  nclip
    /* 3D664 8004CE64 4F00C005 */  bltz       $t6, .L8004CFA4
    /* 3D668 8004CE68 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3D66C 8004CE6C 0080CE31 */  andi       $t6, $t6, 0x8000
    /* 3D670 8004CE70 4C00C015 */  bnez       $t6, .L8004CFA4
    /* 3D674 8004CE74 2A384000 */   slt       $a3, $v0, $zero
    /* 3D678 8004CE78 FFFFE720 */  addi       $a3, $a3, -0x1 /* handwritten instruction */
    /* 3D67C 8004CE7C 42100B00 */  srl        $v0, $t3, 1
    /* 3D680 8004CE80 21105200 */  addu       $v0, $v0, $s2
    /* 3D684 8004CE84 00004484 */  lh         $a0, 0x0($v0)
    /* 3D688 8004CE88 42100C00 */  srl        $v0, $t4, 1
    /* 3D68C 8004CE8C 21105200 */  addu       $v0, $v0, $s2
    /* 3D690 8004CE90 00004584 */  lh         $a1, 0x0($v0)
    /* 3D694 8004CE94 42100D00 */  srl        $v0, $t5, 1
    /* 3D698 8004CE98 21105200 */  addu       $v0, $v0, $s2
    /* 3D69C 8004CE9C 00004684 */  lh         $a2, 0x0($v0)
    /* 3D6A0 8004CEA0 00888448 */  mtc2       $a0, $17 /* handwritten instruction */
    /* 3D6A4 8004CEA4 00908548 */  mtc2       $a1, $18 /* handwritten instruction */
    /* 3D6A8 8004CEA8 00988648 */  mtc2       $a2, $19 /* handwritten instruction */
    /* 3D6AC 8004CEAC 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3D6B0 8004CEB0 00000000 */  nop
    /* 3D6B4 8004CEB4 2D00584B */  avsz3
    /* 3D6B8 8004CEB8 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3D6BC 8004CEBC 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 3D6C0 8004CEC0 2000CAAE */  sw         $t2, 0x20($s6)
    /* 3D6C4 8004CEC4 37004018 */  blez       $v0, .L8004CFA4
    /* 3D6C8 8004CEC8 80100200 */   sll       $v0, $v0, 2
    /* 3D6CC 8004CECC 0100013C */  lui        $at, (0x10000 >> 16)
    /* 3D6D0 8004CED0 2AC84100 */  slt        $t9, $v0, $at
    /* 3D6D4 8004CED4 33002013 */  beqz       $t9, .L8004CFA4
    /* 3D6D8 8004CED8 00000000 */   nop
    /* 3D6DC 8004CEDC 1000088E */  lw         $t0, 0x10($s0)
    /* 3D6E0 8004CEE0 1400098E */  lw         $t1, 0x14($s0)
    /* 3D6E4 8004CEE4 26400701 */  xor        $t0, $t0, $a3
    /* 3D6E8 8004CEE8 26482701 */  xor        $t1, $t1, $a3
    /* 3D6EC 8004CEEC 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 3D6F0 8004CEF0 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 3D6F4 8004CEF4 18000A8E */  lw         $t2, 0x18($s0)
    /* 3D6F8 8004CEF8 024C0900 */  srl        $t1, $t1, 16
    /* 3D6FC 8004CEFC 26504701 */  xor        $t2, $t2, $a3
    /* 3D700 8004CF00 00108A48 */  mtc2       $t2, $2 /* handwritten instruction */
    /* 3D704 8004CF04 00188948 */  mtc2       $t1, $3 /* handwritten instruction */
    /* 3D708 8004CF08 1C00088E */  lw         $t0, 0x1C($s0)
    /* 3D70C 8004CF0C 2000098E */  lw         $t1, 0x20($s0)
    /* 3D710 8004CF10 26400701 */  xor        $t0, $t0, $a3
    /* 3D714 8004CF14 26482701 */  xor        $t1, $t1, $a3
    /* 3D718 8004CF18 00208848 */  mtc2       $t0, $4 /* handwritten instruction */
    /* 3D71C 8004CF1C 00288948 */  mtc2       $t1, $5 /* handwritten instruction */
    /* 3D720 8004CF20 B0FF0124 */  addiu      $at, $zero, -0x50
    /* 3D724 8004CF24 2640E100 */  xor        $t0, $a3, $at
    /* 3D728 8004CF28 20104800 */  add        $v0, $v0, $t0 /* handwritten instruction */
    /* 3D72C 8004CF2C 06106200 */  srlv       $v0, $v0, $v1
    /* 3D730 8004CF30 C2420200 */  srl        $t0, $v0, 11
    /* 3D734 8004CF34 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3D738 8004CF38 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3D73C 8004CF3C 07100201 */  srav       $v0, $v0, $t0
    /* 3D740 8004CF40 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3D744 8004CF44 2004D84A */  nct
    /* 3D748 8004CF48 04000C86 */  lh         $t4, 0x4($s0)
    /* 3D74C 8004CF4C 00000D86 */  lh         $t5, 0x0($s0)
    /* 3D750 8004CF50 00640C00 */  sll        $t4, $t4, 16
    /* 3D754 8004CF54 006C0D00 */  sll        $t5, $t5, 16
    /* 3D758 8004CF58 06000896 */  lhu        $t0, 0x6($s0)
    /* 3D75C 8004CF5C 08000996 */  lhu        $t1, 0x8($s0)
    /* 3D760 8004CF60 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3D764 8004CF64 25400C01 */  or         $t0, $t0, $t4
    /* 3D768 8004CF68 25482D01 */  or         $t1, $t1, $t5
    /* 3D76C 8004CF6C 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3D770 8004CF70 1800C9AE */  sw         $t1, 0x18($s6)
    /* 3D774 8004CF74 2400CAA6 */  sh         $t2, 0x24($s6)
    /* 3D778 8004CF78 80100200 */  sll        $v0, $v0, 2
    /* 3D77C 8004CF7C 21105400 */  addu       $v0, $v0, $s4
    /* 3D780 8004CF80 0000498C */  lw         $t1, 0x0($v0)
    /* 3D784 8004CF84 2440DE02 */  and        $t0, $s6, $fp
    /* 3D788 8004CF88 000048AC */  sw         $t0, 0x0($v0)
    /* 3D78C 8004CF8C 25483501 */  or         $t1, $t1, $s5
    /* 3D790 8004CF90 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3D794 8004CF94 0400D4EA */  swc2       $20, 0x4($s6)
    /* 3D798 8004CF98 1000D5EA */  swc2       $21, 0x10($s6)
    /* 3D79C 8004CF9C 1C00D6EA */  swc2       $22, 0x1C($s6)
    /* 3D7A0 8004CFA0 2800D626 */  addiu      $s6, $s6, 0x28
  .L8004CFA4:
    /* 3D7A4 8004CFA4 24001026 */  addiu      $s0, $s0, 0x24
    /* 3D7A8 8004CFA8 9AFF6016 */  bnez       $s3, .L8004CE14
    /* 3D7AC 8004CFAC FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004CFB0:
    /* 3D7B0 8004CFB0 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3D7B4 8004CFB4 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3D7B8 8004CFB8 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3D7BC 8004CFBC 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3D7C0 8004CFC0 2400B68F */  lw         $s6, 0x24($sp)
    /* 3D7C4 8004CFC4 2000B58F */  lw         $s5, 0x20($sp)
    /* 3D7C8 8004CFC8 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3D7CC 8004CFCC 1800B38F */  lw         $s3, 0x18($sp)
    /* 3D7D0 8004CFD0 1400B28F */  lw         $s2, 0x14($sp)
    /* 3D7D4 8004CFD4 1000B18F */  lw         $s1, 0x10($sp)
    /* 3D7D8 8004CFD8 0800E003 */  jr         $ra
    /* 3D7DC 8004CFDC 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004CDB0
