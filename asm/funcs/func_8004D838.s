glabel func_8004D838
    /* 3E038 8004D838 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3E03C 8004D83C 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3E040 8004D840 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3E044 8004D844 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3E048 8004D848 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3E04C 8004D84C 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3E050 8004D850 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3E054 8004D854 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3E058 8004D858 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3E05C 8004D85C 79008010 */  beqz       $a0, .L8004DA44
    /* 3E060 8004D860 FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3E064 8004D864 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3E068 8004D868 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3E06C 8004D86C 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3E070 8004D870 B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3E074 8004D874 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3E078 8004D878 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3E07C 8004D87C 000C153C */  lui        $s5, (0xC000000 >> 16)
    /* 3E080 8004D880 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3E084 8004D884 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3E088 8004D888 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3E08C 8004D88C B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3E090 8004D890 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3E094 8004D894 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
    /* 3E098 8004D898 000006CA */  lwc2       $6, 0x0($s0)
  .L8004D89C:
    /* 3E09C 8004D89C 1000048E */  lw         $a0, 0x10($s0)
    /* 3E0A0 8004D8A0 00000000 */  nop
    /* 3E0A4 8004D8A4 80280400 */  sll        $a1, $a0, 2
    /* 3E0A8 8004D8A8 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3E0AC 8004D8AC 21287101 */  addu       $a1, $t3, $s1
    /* 3E0B0 8004D8B0 0000A88C */  lw         $t0, 0x0($a1)
    /* 3E0B4 8004D8B4 82290400 */  srl        $a1, $a0, 6
    /* 3E0B8 8004D8B8 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3E0BC 8004D8BC 21289101 */  addu       $a1, $t4, $s1
    /* 3E0C0 8004D8C0 0000A98C */  lw         $t1, 0x0($a1)
    /* 3E0C4 8004D8C4 822B0400 */  srl        $a1, $a0, 14
    /* 3E0C8 8004D8C8 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3E0CC 8004D8CC 2128B101 */  addu       $a1, $t5, $s1
    /* 3E0D0 8004D8D0 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3E0D4 8004D8D4 822D0400 */  srl        $a1, $a0, 22
    /* 3E0D8 8004D8D8 FC03AE30 */  andi       $t6, $a1, 0x3FC
    /* 3E0DC 8004D8DC 2128D101 */  addu       $a1, $t6, $s1
    /* 3E0E0 8004D8E0 0000AF8C */  lw         $t7, 0x0($a1)
    /* 3E0E4 8004D8E4 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3E0E8 8004D8E8 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3E0EC 8004D8EC 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3E0F0 8004D8F0 24C00901 */  and        $t8, $t0, $t1
    /* 3E0F4 8004D8F4 24C00A03 */  and        $t8, $t8, $t2
    /* 3E0F8 8004D8F8 24C00F03 */  and        $t8, $t8, $t7
    /* 3E0FC 8004D8FC 0600404B */  nclip
    /* 3E100 8004D900 4D000007 */  bltz       $t8, .L8004DA38
    /* 3E104 8004D904 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3E108 8004D908 00801833 */  andi       $t8, $t8, 0x8000
    /* 3E10C 8004D90C 4A004104 */  bgez       $v0, .L8004DA38
    /* 3E110 8004D910 00000000 */   nop
    /* 3E114 8004D914 48000017 */  bnez       $t8, .L8004DA38
    /* 3E118 8004D918 42100B00 */   srl       $v0, $t3, 1
    /* 3E11C 8004D91C 21105200 */  addu       $v0, $v0, $s2
    /* 3E120 8004D920 00004484 */  lh         $a0, 0x0($v0)
    /* 3E124 8004D924 42100C00 */  srl        $v0, $t4, 1
    /* 3E128 8004D928 21105200 */  addu       $v0, $v0, $s2
    /* 3E12C 8004D92C 00004584 */  lh         $a1, 0x0($v0)
    /* 3E130 8004D930 42100D00 */  srl        $v0, $t5, 1
    /* 3E134 8004D934 21105200 */  addu       $v0, $v0, $s2
    /* 3E138 8004D938 00004684 */  lh         $a2, 0x0($v0)
    /* 3E13C 8004D93C 42100E00 */  srl        $v0, $t6, 1
    /* 3E140 8004D940 21105200 */  addu       $v0, $v0, $s2
    /* 3E144 8004D944 00004784 */  lh         $a3, 0x0($v0)
    /* 3E148 8004D948 20208500 */  add        $a0, $a0, $a1 /* handwritten instruction */
    /* 3E14C 8004D94C 20208600 */  add        $a0, $a0, $a2 /* handwritten instruction */
    /* 3E150 8004D950 20208700 */  add        $a0, $a0, $a3 /* handwritten instruction */
    /* 3E154 8004D954 83100400 */  sra        $v0, $a0, 2
    /* 3E158 8004D958 37004018 */  blez       $v0, .L8004DA38
    /* 3E15C 8004D95C 0100013C */   lui       $at, (0x10000 >> 16)
    /* 3E160 8004D960 2AC84100 */  slt        $t9, $v0, $at
    /* 3E164 8004D964 34002013 */  beqz       $t9, .L8004DA38
    /* 3E168 8004D968 00000000 */   nop
    /* 3E16C 8004D96C 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3E170 8004D970 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3E174 8004D974 2000CAAE */  sw         $t2, 0x20($s6)
    /* 3E178 8004D978 2C00CFAE */  sw         $t7, 0x2C($s6)
    /* 3E17C 8004D97C 1800198E */  lw         $t9, 0x18($s0)
    /* 3E180 8004D980 140000CA */  lwc2       $0, 0x14($s0)
    /* 3E184 8004D984 00089948 */  mtc2       $t9, $1 /* handwritten instruction */
    /* 3E188 8004D988 06106200 */  srlv       $v0, $v0, $v1
    /* 3E18C 8004D98C C2420200 */  srl        $t0, $v0, 11
    /* 3E190 8004D990 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3E194 8004D994 1E04C84A */  ncs
    /* 3E198 8004D998 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3E19C 8004D99C 07100201 */  srav       $v0, $v0, $t0
    /* 3E1A0 8004D9A0 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3E1A4 8004D9A4 0400D6EA */  swc2       $22, 0x4($s6)
    /* 3E1A8 8004D9A8 1C0000CA */  lwc2       $0, 0x1C($s0)
    /* 3E1AC 8004D9AC 02CC1900 */  srl        $t9, $t9, 16
    /* 3E1B0 8004D9B0 00089948 */  mtc2       $t9, $1 /* handwritten instruction */
    /* 3E1B4 8004D9B4 200002CA */  lwc2       $2, 0x20($s0)
    /* 3E1B8 8004D9B8 2400198E */  lw         $t9, 0x24($s0)
    /* 3E1BC 8004D9BC 280004CA */  lwc2       $4, 0x28($s0)
    /* 3E1C0 8004D9C0 00189948 */  mtc2       $t9, $3 /* handwritten instruction */
    /* 3E1C4 8004D9C4 02C41900 */  srl        $t8, $t9, 16
    /* 3E1C8 8004D9C8 00289848 */  mtc2       $t8, $5 /* handwritten instruction */
    /* 3E1CC 8004D9CC 04000C86 */  lh         $t4, 0x4($s0)
    /* 3E1D0 8004D9D0 00000D86 */  lh         $t5, 0x0($s0)
    /* 3E1D4 8004D9D4 00640C00 */  sll        $t4, $t4, 16
    /* 3E1D8 8004D9D8 006C0D00 */  sll        $t5, $t5, 16
    /* 3E1DC 8004D9DC 2004D84A */  nct
    /* 3E1E0 8004D9E0 06000896 */  lhu        $t0, 0x6($s0)
    /* 3E1E4 8004D9E4 08000996 */  lhu        $t1, 0x8($s0)
    /* 3E1E8 8004D9E8 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3E1EC 8004D9EC 0C000B96 */  lhu        $t3, 0xC($s0)
    /* 3E1F0 8004D9F0 25400C01 */  or         $t0, $t0, $t4
    /* 3E1F4 8004D9F4 25482D01 */  or         $t1, $t1, $t5
    /* 3E1F8 8004D9F8 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3E1FC 8004D9FC 1800C9AE */  sw         $t1, 0x18($s6)
    /* 3E200 8004DA00 2400CAA6 */  sh         $t2, 0x24($s6)
    /* 3E204 8004DA04 3000CBA6 */  sh         $t3, 0x30($s6)
    /* 3E208 8004DA08 80100200 */  sll        $v0, $v0, 2
    /* 3E20C 8004DA0C 21105400 */  addu       $v0, $v0, $s4
    /* 3E210 8004DA10 0000498C */  lw         $t1, 0x0($v0)
    /* 3E214 8004DA14 2440DE02 */  and        $t0, $s6, $fp
    /* 3E218 8004DA18 000048AC */  sw         $t0, 0x0($v0)
    /* 3E21C 8004DA1C 25483501 */  or         $t1, $t1, $s5
    /* 3E220 8004DA20 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3E224 8004DA24 1000D4EA */  swc2       $20, 0x10($s6)
    /* 3E228 8004DA28 1C00D5EA */  swc2       $21, 0x1C($s6)
    /* 3E22C 8004DA2C 2800D6EA */  swc2       $22, 0x28($s6)
    /* 3E230 8004DA30 00000000 */  nop
    /* 3E234 8004DA34 3400D626 */  addiu      $s6, $s6, 0x34
  .L8004DA38:
    /* 3E238 8004DA38 2C001026 */  addiu      $s0, $s0, 0x2C
    /* 3E23C 8004DA3C 97FF6016 */  bnez       $s3, .L8004D89C
    /* 3E240 8004DA40 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004DA44:
    /* 3E244 8004DA44 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3E248 8004DA48 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3E24C 8004DA4C 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3E250 8004DA50 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3E254 8004DA54 2400B68F */  lw         $s6, 0x24($sp)
    /* 3E258 8004DA58 2000B58F */  lw         $s5, 0x20($sp)
    /* 3E25C 8004DA5C 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3E260 8004DA60 1800B38F */  lw         $s3, 0x18($sp)
    /* 3E264 8004DA64 1400B28F */  lw         $s2, 0x14($sp)
    /* 3E268 8004DA68 1000B18F */  lw         $s1, 0x10($sp)
    /* 3E26C 8004DA6C 0800E003 */  jr         $ra
    /* 3E270 8004DA70 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004D838
