glabel func_8004D634
    /* 3DE34 8004D634 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3DE38 8004D638 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3DE3C 8004D63C 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3DE40 8004D640 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3DE44 8004D644 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3DE48 8004D648 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3DE4C 8004D64C 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3DE50 8004D650 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3DE54 8004D654 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3DE58 8004D658 6B008010 */  beqz       $a0, .L8004D808
    /* 3DE5C 8004D65C FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3DE60 8004D660 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3DE64 8004D664 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3DE68 8004D668 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3DE6C 8004D66C B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3DE70 8004D670 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3DE74 8004D674 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3DE78 8004D678 0009153C */  lui        $s5, (0x9000000 >> 16)
    /* 3DE7C 8004D67C FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3DE80 8004D680 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3DE84 8004D684 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3DE88 8004D688 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3DE8C 8004D68C 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3DE90 8004D690 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
    /* 3DE94 8004D694 000006CA */  lwc2       $6, 0x0($s0)
  .L8004D698:
    /* 3DE98 8004D698 0C00048E */  lw         $a0, 0xC($s0)
    /* 3DE9C 8004D69C 00000000 */  nop
    /* 3DEA0 8004D6A0 80280400 */  sll        $a1, $a0, 2
    /* 3DEA4 8004D6A4 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3DEA8 8004D6A8 21287101 */  addu       $a1, $t3, $s1
    /* 3DEAC 8004D6AC 0000A88C */  lw         $t0, 0x0($a1)
    /* 3DEB0 8004D6B0 82290400 */  srl        $a1, $a0, 6
    /* 3DEB4 8004D6B4 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3DEB8 8004D6B8 21289101 */  addu       $a1, $t4, $s1
    /* 3DEBC 8004D6BC 0000A98C */  lw         $t1, 0x0($a1)
    /* 3DEC0 8004D6C0 822B0400 */  srl        $a1, $a0, 14
    /* 3DEC4 8004D6C4 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3DEC8 8004D6C8 2128B101 */  addu       $a1, $t5, $s1
    /* 3DECC 8004D6CC 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3DED0 8004D6D0 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3DED4 8004D6D4 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3DED8 8004D6D8 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3DEDC 8004D6DC 24700901 */  and        $t6, $t0, $t1
    /* 3DEE0 8004D6E0 2470CA01 */  and        $t6, $t6, $t2
    /* 3DEE4 8004D6E4 0600404B */  nclip
    /* 3DEE8 8004D6E8 4400C005 */  bltz       $t6, .L8004D7FC
    /* 3DEEC 8004D6EC 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3DEF0 8004D6F0 0080CE31 */  andi       $t6, $t6, 0x8000
    /* 3DEF4 8004D6F4 41004104 */  bgez       $v0, .L8004D7FC
    /* 3DEF8 8004D6F8 00000000 */   nop
    /* 3DEFC 8004D6FC 3F00C015 */  bnez       $t6, .L8004D7FC
    /* 3DF00 8004D700 42100B00 */   srl       $v0, $t3, 1
    /* 3DF04 8004D704 21105200 */  addu       $v0, $v0, $s2
    /* 3DF08 8004D708 00004484 */  lh         $a0, 0x0($v0)
    /* 3DF0C 8004D70C 42100C00 */  srl        $v0, $t4, 1
    /* 3DF10 8004D710 21105200 */  addu       $v0, $v0, $s2
    /* 3DF14 8004D714 00004584 */  lh         $a1, 0x0($v0)
    /* 3DF18 8004D718 42100D00 */  srl        $v0, $t5, 1
    /* 3DF1C 8004D71C 21105200 */  addu       $v0, $v0, $s2
    /* 3DF20 8004D720 00004684 */  lh         $a2, 0x0($v0)
    /* 3DF24 8004D724 00888448 */  mtc2       $a0, $17 /* handwritten instruction */
    /* 3DF28 8004D728 00908548 */  mtc2       $a1, $18 /* handwritten instruction */
    /* 3DF2C 8004D72C 00988648 */  mtc2       $a2, $19 /* handwritten instruction */
    /* 3DF30 8004D730 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3DF34 8004D734 00000000 */  nop
    /* 3DF38 8004D738 2D00584B */  avsz3
    /* 3DF3C 8004D73C 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3DF40 8004D740 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 3DF44 8004D744 2000CAAE */  sw         $t2, 0x20($s6)
    /* 3DF48 8004D748 2C004018 */  blez       $v0, .L8004D7FC
    /* 3DF4C 8004D74C 80100200 */   sll       $v0, $v0, 2
    /* 3DF50 8004D750 0100013C */  lui        $at, (0x10000 >> 16)
    /* 3DF54 8004D754 2AC84100 */  slt        $t9, $v0, $at
    /* 3DF58 8004D758 28002013 */  beqz       $t9, .L8004D7FC
    /* 3DF5C 8004D75C 00000000 */   nop
    /* 3DF60 8004D760 100000CA */  lwc2       $0, 0x10($s0)
    /* 3DF64 8004D764 1400088E */  lw         $t0, 0x14($s0)
    /* 3DF68 8004D768 180002CA */  lwc2       $2, 0x18($s0)
    /* 3DF6C 8004D76C 00088848 */  mtc2       $t0, $1 /* handwritten instruction */
    /* 3DF70 8004D770 1C0004CA */  lwc2       $4, 0x1C($s0)
    /* 3DF74 8004D774 200005CA */  lwc2       $5, 0x20($s0)
    /* 3DF78 8004D778 02440800 */  srl        $t0, $t0, 16
    /* 3DF7C 8004D77C 00188848 */  mtc2       $t0, $3 /* handwritten instruction */
    /* 3DF80 8004D780 06106200 */  srlv       $v0, $v0, $v1
    /* 3DF84 8004D784 C2420200 */  srl        $t0, $v0, 11
    /* 3DF88 8004D788 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3DF8C 8004D78C 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3DF90 8004D790 07100201 */  srav       $v0, $v0, $t0
    /* 3DF94 8004D794 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3DF98 8004D798 2004D84A */  nct
    /* 3DF9C 8004D79C 04000C86 */  lh         $t4, 0x4($s0)
    /* 3DFA0 8004D7A0 00000D86 */  lh         $t5, 0x0($s0)
    /* 3DFA4 8004D7A4 00640C00 */  sll        $t4, $t4, 16
    /* 3DFA8 8004D7A8 006C0D00 */  sll        $t5, $t5, 16
    /* 3DFAC 8004D7AC 06000896 */  lhu        $t0, 0x6($s0)
    /* 3DFB0 8004D7B0 08000996 */  lhu        $t1, 0x8($s0)
    /* 3DFB4 8004D7B4 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3DFB8 8004D7B8 25400C01 */  or         $t0, $t0, $t4
    /* 3DFBC 8004D7BC 25482D01 */  or         $t1, $t1, $t5
    /* 3DFC0 8004D7C0 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3DFC4 8004D7C4 1800C9AE */  sw         $t1, 0x18($s6)
    /* 3DFC8 8004D7C8 2400CAA6 */  sh         $t2, 0x24($s6)
    /* 3DFCC 8004D7CC 80100200 */  sll        $v0, $v0, 2
    /* 3DFD0 8004D7D0 21105400 */  addu       $v0, $v0, $s4
    /* 3DFD4 8004D7D4 0000498C */  lw         $t1, 0x0($v0)
    /* 3DFD8 8004D7D8 2440DE02 */  and        $t0, $s6, $fp
    /* 3DFDC 8004D7DC 000048AC */  sw         $t0, 0x0($v0)
    /* 3DFE0 8004D7E0 25483501 */  or         $t1, $t1, $s5
    /* 3DFE4 8004D7E4 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3DFE8 8004D7E8 0400D4EA */  swc2       $20, 0x4($s6)
    /* 3DFEC 8004D7EC 1000D5EA */  swc2       $21, 0x10($s6)
    /* 3DFF0 8004D7F0 1C00D6EA */  swc2       $22, 0x1C($s6)
    /* 3DFF4 8004D7F4 00000000 */  nop
    /* 3DFF8 8004D7F8 2800D626 */  addiu      $s6, $s6, 0x28
  .L8004D7FC:
    /* 3DFFC 8004D7FC 24001026 */  addiu      $s0, $s0, 0x24
    /* 3E000 8004D800 A5FF6016 */  bnez       $s3, .L8004D698
    /* 3E004 8004D804 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004D808:
    /* 3E008 8004D808 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3E00C 8004D80C B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3E010 8004D810 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3E014 8004D814 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3E018 8004D818 2400B68F */  lw         $s6, 0x24($sp)
    /* 3E01C 8004D81C 2000B58F */  lw         $s5, 0x20($sp)
    /* 3E020 8004D820 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3E024 8004D824 1800B38F */  lw         $s3, 0x18($sp)
    /* 3E028 8004D828 1400B28F */  lw         $s2, 0x14($sp)
    /* 3E02C 8004D82C 1000B18F */  lw         $s1, 0x10($sp)
    /* 3E030 8004D830 0800E003 */  jr         $ra
    /* 3E034 8004D834 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004D634
