glabel gpu_EmitOtCutoffFlatLitTexturedTrianglesWithOptionalSubdivision
    /* 3BEE4 8004B6E4 801F083C */  lui        $t0, (0x1F8001D4 >> 16)
    /* 3BEE8 8004B6E8 E8011FAD */  sw         $ra, (0x1F8001E8 & 0xFFFF)($t0)
    /* 3BEEC 8004B6EC EC011EAD */  sw         $fp, (0x1F8001EC & 0xFFFF)($t0)
    /* 3BEF0 8004B6F0 F00117AD */  sw         $s7, (0x1F8001F0 & 0xFFFF)($t0)
    /* 3BEF4 8004B6F4 F40116AD */  sw         $s6, (0x1F8001F4 & 0xFFFF)($t0)
    /* 3BEF8 8004B6F8 F80115AD */  sw         $s5, (0x1F8001F8 & 0xFFFF)($t0)
    /* 3BEFC 8004B6FC FC0114AD */  sw         $s4, (0x1F8001FC & 0xFFFF)($t0)
    /* 3BF00 8004B700 000213AD */  sw         $s3, (0x1F800200 & 0xFFFF)($t0)
    /* 3BF04 8004B704 040212AD */  sw         $s2, (0x1F800204 & 0xFFFF)($t0)
    /* 3BF08 8004B708 080211AD */  sw         $s1, (0x1F800208 & 0xFFFF)($t0)
    /* 3BF0C 8004B70C 0C0210AD */  sw         $s0, (0x1F80020C & 0xFFFF)($t0)
    /* 3BF10 8004B710 21B80001 */  addu       $s7, $t0, $zero
    /* 3BF14 8004B714 E001E7A6 */  sh         $a3, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3BF18 8004B718 D801E5AE */  sw         $a1, (0x1F8001D8 & 0xFFFF)($s7)
    /* 3BF1C 8004B71C 2180C000 */  addu       $s0, $a2, $zero
    /* 3BF20 8004B720 0C00F28E */  lw         $s2, (0x1F80000C & 0xFFFF)($s7)
    /* 3BF24 8004B724 0A80143C */  lui        $s4, %hi(g_prim_buf_cursor)
    /* 3BF28 8004B728 B438948E */  lw         $s4, %lo(g_prim_buf_cursor)($s4)
    /* 3BF2C 8004B72C 2000F522 */  addi       $s5, $s7, (0x1F800020 & 0xFFFF) /* handwritten instruction */
    /* 3BF30 8004B730 1001F622 */  addi       $s6, $s7, (0x1F800110 & 0xFFFF) /* handwritten instruction */
    /* 3BF34 8004B734 1002F322 */  addi       $s3, $s7, (0x1F800210 & 0xFFFF) /* handwritten instruction */
    /* 3BF38 8004B738 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3BF3C 8004B73C FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3BF40 8004B740 FFFF8420 */  addi       $a0, $a0, -0x1 /* handwritten instruction */
  .L8004B744:
    /* 3BF44 8004B744 E101E982 */  lb         $t1, (0x1F8001E1 & 0xFFFF)($s7)
    /* 3BF48 8004B748 0000088E */  lw         $t0, 0x0($s0)
    /* 3BF4C 8004B74C 01002931 */  andi       $t1, $t1, 0x1
    /* 3BF50 8004B750 404E0900 */  sll        $t1, $t1, 25
    /* 3BF54 8004B754 25400901 */  or         $t0, $t0, $t1
    /* 3BF58 8004B758 00308848 */  mtc2       $t0, $6 /* handwritten instruction */
    /* 3BF5C 8004B75C 0C00028E */  lw         $v0, 0xC($s0)
    /* 3BF60 8004B760 D401E4AE */  sw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3BF64 8004B764 80600200 */  sll        $t4, $v0, 2
    /* 3BF68 8004B768 FC038C31 */  andi       $t4, $t4, 0x3FC
    /* 3BF6C 8004B76C 21409501 */  addu       $t0, $t4, $s5
    /* 3BF70 8004B770 0000088D */  lw         $t0, 0x0($t0)
    /* 3BF74 8004B774 82690200 */  srl        $t5, $v0, 6
    /* 3BF78 8004B778 FC03AD31 */  andi       $t5, $t5, 0x3FC
    /* 3BF7C 8004B77C 2148B501 */  addu       $t1, $t5, $s5
    /* 3BF80 8004B780 0000298D */  lw         $t1, 0x0($t1)
    /* 3BF84 8004B784 82730200 */  srl        $t6, $v0, 14
    /* 3BF88 8004B788 FC03CE31 */  andi       $t6, $t6, 0x3FC
    /* 3BF8C 8004B78C 2150D501 */  addu       $t2, $t6, $s5
    /* 3BF90 8004B790 00004A8D */  lw         $t2, 0x0($t2)
    /* 3BF94 8004B794 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3BF98 8004B798 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3BF9C 8004B79C 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3BFA0 8004B7A0 24180901 */  and        $v1, $t0, $t1
    /* 3BFA4 8004B7A4 24186A00 */  and        $v1, $v1, $t2
    /* 3BFA8 8004B7A8 00000000 */  nop
    /* 3BFAC 8004B7AC 0600404B */  nclip
    /* 3BFB0 8004B7B0 D9006004 */  bltz       $v1, .L8004BB18
    /* 3BFB4 8004B7B4 00000000 */   nop
    /* 3BFB8 8004B7B8 00806330 */  andi       $v1, $v1, 0x8000
    /* 3BFBC 8004B7BC 00C00248 */  mfc2       $v0, $24 /* handwritten instruction */
    /* 3BFC0 8004B7C0 D5006014 */  bnez       $v1, .L8004BB18
    /* 3BFC4 8004B7C4 00000000 */   nop
    /* 3BFC8 8004B7C8 D3004104 */  bgez       $v0, .L8004BB18
    /* 3BFCC 8004B7CC 22180200 */   neg       $v1, $v0 /* handwritten instruction */
    /* 3BFD0 8004B7D0 03240800 */  sra        $a0, $t0, 16
    /* 3BFD4 8004B7D4 10FF8420 */  addi       $a0, $a0, -0xF0 /* handwritten instruction */
    /* 3BFD8 8004B7D8 032C0900 */  sra        $a1, $t1, 16
    /* 3BFDC 8004B7DC 10FFA520 */  addi       $a1, $a1, -0xF0 /* handwritten instruction */
    /* 3BFE0 8004B7E0 25208500 */  or         $a0, $a0, $a1
    /* 3BFE4 8004B7E4 032C0A00 */  sra        $a1, $t2, 16
    /* 3BFE8 8004B7E8 10FFA520 */  addi       $a1, $a1, -0xF0 /* handwritten instruction */
    /* 3BFEC 8004B7EC 25208500 */  or         $a0, $a0, $a1
    /* 3BFF0 8004B7F0 00808430 */  andi       $a0, $a0, 0x8000
    /* 3BFF4 8004B7F4 C8008010 */  beqz       $a0, .L8004BB18
    /* 3BFF8 8004B7F8 00000000 */   nop
    /* 3BFFC 8004B7FC 42600C00 */  srl        $t4, $t4, 1
    /* 3C000 8004B800 20209601 */  add        $a0, $t4, $s6 /* handwritten instruction */
    /* 3C004 8004B804 00008484 */  lh         $a0, 0x0($a0)
    /* 3C008 8004B808 42680D00 */  srl        $t5, $t5, 1
    /* 3C00C 8004B80C 2028B601 */  add        $a1, $t5, $s6 /* handwritten instruction */
    /* 3C010 8004B810 0000A584 */  lh         $a1, 0x0($a1)
    /* 3C014 8004B814 42700E00 */  srl        $t6, $t6, 1
    /* 3C018 8004B818 2030D601 */  add        $a2, $t6, $s6 /* handwritten instruction */
    /* 3C01C 8004B81C 0000C684 */  lh         $a2, 0x0($a2)
    /* 3C020 8004B820 2A108500 */  slt        $v0, $a0, $a1
    /* 3C024 8004B824 03004010 */  beqz       $v0, .L8004B834
    /* 3C028 8004B828 2A108600 */   slt       $v0, $a0, $a2
    /* 3C02C 8004B82C 2120A000 */  addu       $a0, $a1, $zero
    /* 3C030 8004B830 2A108600 */  slt        $v0, $a0, $a2
  .L8004B834:
    /* 3C034 8004B834 02004010 */  beqz       $v0, .L8004B840
    /* 3C038 8004B838 00000000 */   nop
    /* 3C03C 8004B83C 2120C000 */  addu       $a0, $a2, $zero
  .L8004B840:
    /* 3C040 8004B840 21108000 */  addu       $v0, $a0, $zero
    /* 3C044 8004B844 21384000 */  addu       $a3, $v0, $zero
    /* 3C048 8004B848 C601F886 */  lh         $t8, (0x1F8001C6 & 0xFFFF)($s7)
    /* 3C04C 8004B84C 1800E68E */  lw         $a2, (0x1F800018 & 0xFFFF)($s7)
    /* 3C050 8004B850 1000E58E */  lw         $a1, (0x1F800010 & 0xFFFF)($s7)
    /* 3C054 8004B854 06100203 */  srlv       $v0, $v0, $t8
    /* 3C058 8004B858 C2C20200 */  srl        $t8, $v0, 11
    /* 3C05C 8004B85C FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3C060 8004B860 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3C064 8004B864 07100203 */  srav       $v0, $v0, $t8
    /* 3C068 8004B868 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3C06C 8004B86C 2A08A200 */  slt        $at, $a1, $v0
    /* 3C070 8004B870 A9002014 */  bnez       $at, .L8004BB18
    /* 3C074 8004B874 00000000 */   nop
    /* 3C078 8004B878 2A084600 */  slt        $at, $v0, $a2
    /* 3C07C 8004B87C A6002014 */  bnez       $at, .L8004BB18
    /* 3C080 8004B880 00000000 */   nop
    /* 3C084 8004B884 100000CA */  lwc2       $0, 0x10($s0)
    /* 3C088 8004B888 140001CA */  lwc2       $1, 0x14($s0)
    /* 3C08C 8004B88C 04000486 */  lh         $a0, 0x4($s0)
    /* 3C090 8004B890 00000586 */  lh         $a1, 0x0($s0)
    /* 3C094 8004B894 CC01E4A6 */  sh         $a0, (0x1F8001CC & 0xFFFF)($s7)
    /* 3C098 8004B898 CE01E5A6 */  sh         $a1, (0x1F8001CE & 0xFFFF)($s7)
    /* 3C09C 8004B89C 1E04C84A */  ncs
    /* 3C0A0 8004B8A0 00026428 */  slti       $a0, $v1, 0x200
    /* 3C0A4 8004B8A4 00036328 */  slti       $v1, $v1, 0x300
    /* 3C0A8 8004B8A8 20188300 */  add        $v1, $a0, $v1 /* handwritten instruction */
    /* 3C0AC 8004B8AC C001E58E */  lw         $a1, (0x1F8001C0 & 0xFFFF)($s7)
    /* 3C0B0 8004B8B0 00000000 */  nop
    /* 3C0B4 8004B8B4 0300A530 */  andi       $a1, $a1, 0x3
    /* 3C0B8 8004B8B8 1100A010 */  beqz       $a1, .L8004B900
    /* 3C0BC 8004B8BC 21200000 */   addu      $a0, $zero, $zero
    /* 3C0C0 8004B8C0 FEFFA620 */  addi       $a2, $a1, -0x2 /* handwritten instruction */
    /* 3C0C4 8004B8C4 0F00C010 */  beqz       $a2, .L8004B904
    /* 3C0C8 8004B8C8 00000000 */   nop
    /* 3C0CC 8004B8CC E001E482 */  lb         $a0, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3C0D0 8004B8D0 00000000 */  nop
    /* 3C0D4 8004B8D4 FEFF8420 */  addi       $a0, $a0, -0x2 /* handwritten instruction */
    /* 3C0D8 8004B8D8 22200400 */  neg        $a0, $a0 /* handwritten instruction */
    /* 3C0DC 8004B8DC FFFFA620 */  addi       $a2, $a1, -0x1 /* handwritten instruction */
    /* 3C0E0 8004B8E0 0700C010 */  beqz       $a2, .L8004B900
    /* 3C0E4 8004B8E4 00000000 */   nop
    /* 3C0E8 8004B8E8 E001E482 */  lb         $a0, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3C0EC 8004B8EC 00000000 */  nop
    /* 3C0F0 8004B8F0 2A288300 */  slt        $a1, $a0, $v1
    /* 3C0F4 8004B8F4 22280500 */  neg        $a1, $a1 /* handwritten instruction */
    /* 3C0F8 8004B8F8 22206400 */  sub        $a0, $v1, $a0 /* handwritten instruction */
    /* 3C0FC 8004B8FC 24208500 */  and        $a0, $a0, $a1
  .L8004B900:
    /* 3C100 8004B900 21188000 */  addu       $v1, $a0, $zero
  .L8004B904:
    /* 3C104 8004B904 06000496 */  lhu        $a0, 0x6($s0)
    /* 3C108 8004B908 08000596 */  lhu        $a1, 0x8($s0)
    /* 3C10C 8004B90C 0A000696 */  lhu        $a2, 0xA($s0)
    /* 3C110 8004B910 0A006010 */  beqz       $v1, .L8004B93C
    /* 3C114 8004B914 FF001824 */   addiu     $t8, $zero, 0xFF
    /* 3C118 8004B918 06C07800 */  srlv       $t8, $t8, $v1
    /* 3C11C 8004B91C 00CA1800 */  sll        $t9, $t8, 8
    /* 3C120 8004B920 25C01903 */  or         $t8, $t8, $t9
    /* 3C124 8004B924 06206400 */  srlv       $a0, $a0, $v1
    /* 3C128 8004B928 24209800 */  and        $a0, $a0, $t8
    /* 3C12C 8004B92C 06286500 */  srlv       $a1, $a1, $v1
    /* 3C130 8004B930 2428B800 */  and        $a1, $a1, $t8
    /* 3C134 8004B934 06306600 */  srlv       $a2, $a2, $v1
    /* 3C138 8004B938 2430D800 */  and        $a2, $a2, $t8
  .L8004B93C:
    /* 3C13C 8004B93C E201E4A6 */  sh         $a0, (0x1F8001E2 & 0xFFFF)($s7)
    /* 3C140 8004B940 E401E5A6 */  sh         $a1, (0x1F8001E4 & 0xFFFF)($s7)
    /* 3C144 8004B944 E601E6A6 */  sh         $a2, (0x1F8001E6 & 0xFFFF)($s7)
    /* 3C148 8004B948 02001892 */  lbu        $t8, 0x2($s0)
    /* 3C14C 8004B94C E001E782 */  lb         $a3, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3C150 8004B950 42C01800 */  srl        $t8, $t8, 1
    /* 3C154 8004B954 03001833 */  andi       $t8, $t8, 0x3
    /* 3C158 8004B958 04000013 */  beqz       $t8, .L8004B96C
    /* 3C15C 8004B95C FFFF1823 */   addi      $t8, $t8, -0x1 /* handwritten instruction */
    /* 3C160 8004B960 2238F800 */  sub        $a3, $a3, $t8 /* handwritten instruction */
    /* 3C164 8004B964 2000E01C */  bgtz       $a3, .L8004B9E8
    /* 3C168 8004B968 00000000 */   nop
  .L8004B96C:
    /* 3C16C 8004B96C CC01EC86 */  lh         $t4, (0x1F8001CC & 0xFFFF)($s7)
    /* 3C170 8004B970 CE01ED86 */  lh         $t5, (0x1F8001CE & 0xFFFF)($s7)
    /* 3C174 8004B974 080088AE */  sw         $t0, 0x8($s4)
    /* 3C178 8004B978 100089AE */  sw         $t1, 0x10($s4)
    /* 3C17C 8004B97C 18008AAE */  sw         $t2, 0x18($s4)
    /* 3C180 8004B980 00640C00 */  sll        $t4, $t4, 16
    /* 3C184 8004B984 006C0D00 */  sll        $t5, $t5, 16
    /* 3C188 8004B988 E201E896 */  lhu        $t0, (0x1F8001E2 & 0xFFFF)($s7)
    /* 3C18C 8004B98C E401E996 */  lhu        $t1, (0x1F8001E4 & 0xFFFF)($s7)
    /* 3C190 8004B990 E601EA96 */  lhu        $t2, (0x1F8001E6 & 0xFFFF)($s7)
    /* 3C194 8004B994 25400C01 */  or         $t0, $t0, $t4
    /* 3C198 8004B998 25482D01 */  or         $t1, $t1, $t5
    /* 3C19C 8004B99C 0C0088AE */  sw         $t0, 0xC($s4)
    /* 3C1A0 8004B9A0 140089AE */  sw         $t1, 0x14($s4)
    /* 3C1A4 8004B9A4 1C008AA6 */  sh         $t2, 0x1C($s4)
    /* 3C1A8 8004B9A8 040096EA */  swc2       $22, 0x4($s4)
    /* 3C1AC 8004B9AC 80100200 */  sll        $v0, $v0, 2
    /* 3C1B0 8004B9B0 21105200 */  addu       $v0, $v0, $s2
    /* 3C1B4 8004B9B4 0000498C */  lw         $t1, 0x0($v0)
    /* 3C1B8 8004B9B8 24409E02 */  and        $t0, $s4, $fp
    /* 3C1BC 8004B9BC 000048AC */  sw         $t0, 0x0($v0)
    /* 3C1C0 8004B9C0 00070A3C */  lui        $t2, (0x7000000 >> 16)
    /* 3C1C4 8004B9C4 25482A01 */  or         $t1, $t1, $t2
    /* 3C1C8 8004B9C8 000089AE */  sw         $t1, 0x0($s4)
    /* 3C1CC 8004B9CC D401E48E */  lw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3C1D0 8004B9D0 18001022 */  addi       $s0, $s0, 0x18 /* handwritten instruction */
    /* 3C1D4 8004B9D4 20009422 */  addi       $s4, $s4, 0x20 /* handwritten instruction */
    /* 3C1D8 8004B9D8 5AFF8014 */  bnez       $a0, .L8004B744
    /* 3C1DC 8004B9DC FFFF8420 */   addi      $a0, $a0, -0x1 /* handwritten instruction */
    /* 3C1E0 8004B9E0 CA2E0108 */  j          .L8004BB28
    /* 3C1E4 8004B9E4 00000000 */   nop
  .L8004B9E8:
    /* 3C1E8 8004B9E8 C001E48E */  lw         $a0, (0x1F8001C0 & 0xFFFF)($s7)
    /* 3C1EC 8004B9EC 00000000 */  nop
    /* 3C1F0 8004B9F0 04008430 */  andi       $a0, $a0, 0x4
    /* 3C1F4 8004B9F4 14008010 */  beqz       $a0, .L8004BA48
    /* 3C1F8 8004B9F8 00000000 */   nop
    /* 3C1FC 8004B9FC 2120E000 */  addu       $a0, $a3, $zero
    /* 3C200 8004BA00 0000058E */  lw         $a1, 0x0($s0)
    /* 3C204 8004BA04 00000000 */  nop
    /* 3C208 8004BA08 FF00013C */  lui        $at, (0xFFFFFF >> 16)
    /* 3C20C 8004BA0C FFFF2134 */  ori        $at, $at, (0xFFFFFF & 0xFFFF)
    /* 3C210 8004BA10 2528A100 */  or         $a1, $a1, $at
    /* 3C214 8004BA14 09008010 */  beqz       $a0, .L8004BA3C
    /* 3C218 8004BA18 00000000 */   nop
    /* 3C21C 8004BA1C 00FF0124 */  addiu      $at, $zero, -0x100
    /* 3C220 8004BA20 2428A100 */  and        $a1, $a1, $at
    /* 3C224 8004BA24 FFFF8420 */  addi       $a0, $a0, -0x1 /* handwritten instruction */
    /* 3C228 8004BA28 04008010 */  beqz       $a0, .L8004BA3C
    /* 3C22C 8004BA2C 00000000 */   nop
    /* 3C230 8004BA30 FFFF013C */  lui        $at, (0xFFFF4000 >> 16)
    /* 3C234 8004BA34 00402134 */  ori        $at, $at, (0xFFFF4000 & 0xFFFF)
    /* 3C238 8004BA38 2428A100 */  and        $a1, $a1, $at
  .L8004BA3C:
    /* 3C23C 8004BA3C 00B08548 */  mtc2       $a1, $22 /* handwritten instruction */
    /* 3C240 8004BA40 00000000 */  nop
    /* 3C244 8004BA44 00000000 */  nop
  .L8004BA48:
    /* 3C248 8004BA48 80100200 */  sll        $v0, $v0, 2
    /* 3C24C 8004BA4C 21105200 */  addu       $v0, $v0, $s2
    /* 3C250 8004BA50 0000518C */  lw         $s1, 0x0($v0)
    /* 3C254 8004BA54 DC01E2AE */  sw         $v0, (0x1F8001DC & 0xFFFF)($s7)
    /* 3C258 8004BA58 0007083C */  lui        $t0, (0x7000000 >> 16)
    /* 3C25C 8004BA5C 25882802 */  or         $s1, $s1, $t0
    /* 3C260 8004BA60 D801E58E */  lw         $a1, (0x1F8001D8 & 0xFFFF)($s7)
    /* 3C264 8004BA64 D001F6EA */  swc2       $22, (0x1F8001D0 & 0xFFFF)($s7)
    /* 3C268 8004BA68 20408C01 */  add        $t0, $t4, $t4 /* handwritten instruction */
    /* 3C26C 8004BA6C 20608801 */  add        $t4, $t4, $t0 /* handwritten instruction */
    /* 3C270 8004BA70 20608501 */  add        $t4, $t4, $a1 /* handwritten instruction */
    /* 3C274 8004BA74 00008885 */  lh         $t0, 0x0($t4)
    /* 3C278 8004BA78 02008985 */  lh         $t1, 0x2($t4)
    /* 3C27C 8004BA7C 04008A85 */  lh         $t2, 0x4($t4)
    /* 3C280 8004BA80 E201EB86 */  lh         $t3, (0x1F8001E2 & 0xFFFF)($s7)
    /* 3C284 8004BA84 080068A6 */  sh         $t0, 0x8($s3)
    /* 3C288 8004BA88 0A0069A6 */  sh         $t1, 0xA($s3)
    /* 3C28C 8004BA8C 0C006AA6 */  sh         $t2, 0xC($s3)
    /* 3C290 8004BA90 0E006BA6 */  sh         $t3, 0xE($s3)
    /* 3C294 8004BA94 2040AD01 */  add        $t0, $t5, $t5 /* handwritten instruction */
    /* 3C298 8004BA98 2068A801 */  add        $t5, $t5, $t0 /* handwritten instruction */
    /* 3C29C 8004BA9C 2068A501 */  add        $t5, $t5, $a1 /* handwritten instruction */
    /* 3C2A0 8004BAA0 0000A885 */  lh         $t0, 0x0($t5)
    /* 3C2A4 8004BAA4 0200A985 */  lh         $t1, 0x2($t5)
    /* 3C2A8 8004BAA8 0400AA85 */  lh         $t2, 0x4($t5)
    /* 3C2AC 8004BAAC E401EB86 */  lh         $t3, (0x1F8001E4 & 0xFFFF)($s7)
    /* 3C2B0 8004BAB0 100068A6 */  sh         $t0, 0x10($s3)
    /* 3C2B4 8004BAB4 120069A6 */  sh         $t1, 0x12($s3)
    /* 3C2B8 8004BAB8 14006AA6 */  sh         $t2, 0x14($s3)
    /* 3C2BC 8004BABC 16006BA6 */  sh         $t3, 0x16($s3)
    /* 3C2C0 8004BAC0 2040CE01 */  add        $t0, $t6, $t6 /* handwritten instruction */
    /* 3C2C4 8004BAC4 2070C801 */  add        $t6, $t6, $t0 /* handwritten instruction */
    /* 3C2C8 8004BAC8 2070C501 */  add        $t6, $t6, $a1 /* handwritten instruction */
    /* 3C2CC 8004BACC 0000C885 */  lh         $t0, 0x0($t6)
    /* 3C2D0 8004BAD0 0200C985 */  lh         $t1, 0x2($t6)
    /* 3C2D4 8004BAD4 0400CA85 */  lh         $t2, 0x4($t6)
    /* 3C2D8 8004BAD8 E601EB86 */  lh         $t3, (0x1F8001E6 & 0xFFFF)($s7)
    /* 3C2DC 8004BADC 180068A6 */  sh         $t0, 0x18($s3)
    /* 3C2E0 8004BAE0 1A0069A6 */  sh         $t1, 0x1A($s3)
    /* 3C2E4 8004BAE4 1C006AA6 */  sh         $t2, 0x1C($s3)
    /* 3C2E8 8004BAE8 1E006BA6 */  sh         $t3, 0x1E($s3)
    /* 3C2EC 8004BAEC DA2E010C */  jal        gpu_SubdivideAndEmitTexturedTriangle
    /* 3C2F0 8004BAF0 2120E000 */   addu      $a0, $a3, $zero
    /* 3C2F4 8004BAF4 DC01E88E */  lw         $t0, (0x1F8001DC & 0xFFFF)($s7)
    /* 3C2F8 8004BAF8 24483E02 */  and        $t1, $s1, $fp
    /* 3C2FC 8004BAFC 000009AD */  sw         $t1, 0x0($t0)
    /* 3C300 8004BB00 D401E48E */  lw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3C304 8004BB04 18001022 */  addi       $s0, $s0, 0x18 /* handwritten instruction */
    /* 3C308 8004BB08 0EFF8014 */  bnez       $a0, .L8004B744
    /* 3C30C 8004BB0C FFFF8420 */   addi      $a0, $a0, -0x1 /* handwritten instruction */
    /* 3C310 8004BB10 CA2E0108 */  j          .L8004BB28
    /* 3C314 8004BB14 00000000 */   nop
  .L8004BB18:
    /* 3C318 8004BB18 D401E48E */  lw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3C31C 8004BB1C 18001022 */  addi       $s0, $s0, 0x18 /* handwritten instruction */
    /* 3C320 8004BB20 08FF8014 */  bnez       $a0, .L8004B744
    /* 3C324 8004BB24 FFFF8420 */   addi      $a0, $a0, -0x1 /* handwritten instruction */
  .L8004BB28:
    /* 3C328 8004BB28 0A80013C */  lui        $at, %hi(g_prim_buf_cursor)
    /* 3C32C 8004BB2C B43834AC */  sw         $s4, %lo(g_prim_buf_cursor)($at)
    /* 3C330 8004BB30 21100002 */  addu       $v0, $s0, $zero
    /* 3C334 8004BB34 2140E002 */  addu       $t0, $s7, $zero
    /* 3C338 8004BB38 E8011F8D */  lw         $ra, (0x1F8001E8 & 0xFFFF)($t0)
    /* 3C33C 8004BB3C EC011E8D */  lw         $fp, (0x1F8001EC & 0xFFFF)($t0)
    /* 3C340 8004BB40 F001178D */  lw         $s7, (0x1F8001F0 & 0xFFFF)($t0)
    /* 3C344 8004BB44 F401168D */  lw         $s6, (0x1F8001F4 & 0xFFFF)($t0)
    /* 3C348 8004BB48 F801158D */  lw         $s5, (0x1F8001F8 & 0xFFFF)($t0)
    /* 3C34C 8004BB4C FC01148D */  lw         $s4, (0x1F8001FC & 0xFFFF)($t0)
    /* 3C350 8004BB50 0002138D */  lw         $s3, (0x1F800200 & 0xFFFF)($t0)
    /* 3C354 8004BB54 0402128D */  lw         $s2, (0x1F800204 & 0xFFFF)($t0)
    /* 3C358 8004BB58 0802118D */  lw         $s1, (0x1F800208 & 0xFFFF)($t0)
    /* 3C35C 8004BB5C 0C02108D */  lw         $s0, (0x1F80020C & 0xFFFF)($t0)
    /* 3C360 8004BB60 0800E003 */  jr         $ra
    /* 3C364 8004BB64 00000000 */   nop
endlabel gpu_EmitOtCutoffFlatLitTexturedTrianglesWithOptionalSubdivision
