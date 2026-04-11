glabel func_8003D7B4
    /* 2DFB4 8003D7B4 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 2DFB8 8003D7B8 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 2DFBC 8003D7BC 21980000 */  addu       $s3, $zero, $zero
    /* 2DFC0 8003D7C0 2000B4AF */  sw         $s4, 0x20($sp)
    /* 2DFC4 8003D7C4 FFFF1434 */  ori        $s4, $zero, 0xFFFF
    /* 2DFC8 8003D7C8 40100400 */  sll        $v0, $a0, 1
    /* 2DFCC 8003D7CC 21104400 */  addu       $v0, $v0, $a0
    /* 2DFD0 8003D7D0 C0100200 */  sll        $v0, $v0, 3
    /* 2DFD4 8003D7D4 0A80033C */  lui        $v1, %hi(D_800A3D40)
    /* 2DFD8 8003D7D8 403D6324 */  addiu      $v1, $v1, %lo(D_800A3D40)
    /* 2DFDC 8003D7DC 1800B2AF */  sw         $s2, 0x18($sp)
    /* 2DFE0 8003D7E0 21904300 */  addu       $s2, $v0, $v1
    /* 2DFE4 8003D7E4 1400B1AF */  sw         $s1, 0x14($sp)
    /* 2DFE8 8003D7E8 21884002 */  addu       $s1, $s2, $zero
    /* 2DFEC 8003D7EC 2400BFAF */  sw         $ra, 0x24($sp)
    /* 2DFF0 8003D7F0 1000B0AF */  sw         $s0, 0x10($sp)
  .L8003D7F4:
    /* 2DFF4 8003D7F4 21204002 */  addu       $a0, $s2, $zero
    /* 2DFF8 8003D7F8 22F6000C */  jal        coli_calc_motion
    /* 2DFFC 8003D7FC 04000524 */   addiu     $a1, $zero, 0x4
    /* 2E000 8003D800 21804000 */  addu       $s0, $v0, $zero
    /* 2E004 8003D804 02000016 */  bnez       $s0, .L8003D810
    /* 2E008 8003D808 00000000 */   nop
    /* 2E00C 8003D80C 10001024 */  addiu      $s0, $zero, 0x10
  .L8003D810:
    /* 2E010 8003D810 21204002 */  addu       $a0, $s2, $zero
    /* 2E014 8003D814 22F6000C */  jal        coli_calc_motion
    /* 2E018 8003D818 21280002 */   addu      $a1, $s0, $zero
    /* 2E01C 8003D81C 21184000 */  addu       $v1, $v0, $zero
    /* 2E020 8003D820 00140300 */  sll        $v0, $v1, 16
    /* 2E024 8003D824 03140200 */  sra        $v0, $v0, 16
    /* 2E028 8003D828 FFFF0526 */  addiu      $a1, $s0, -0x1
    /* 2E02C 8003D82C 0710A200 */  srav       $v0, $v0, $a1
    /* 2E030 8003D830 01004230 */  andi       $v0, $v0, 0x1
    /* 2E034 8003D834 03004010 */  beqz       $v0, .L8003D844
    /* 2E038 8003D838 21206000 */   addu      $a0, $v1, $zero
    /* 2E03C 8003D83C 0410B400 */  sllv       $v0, $s4, $a1
    /* 2E040 8003D840 25206200 */  or         $a0, $v1, $v0
  .L8003D844:
    /* 2E044 8003D844 0C002296 */  lhu        $v0, 0xC($s1)
    /* 2E048 8003D848 01007326 */  addiu      $s3, $s3, 0x1
    /* 2E04C 8003D84C 21104400 */  addu       $v0, $v0, $a0
    /* 2E050 8003D850 0C0022A6 */  sh         $v0, 0xC($s1)
    /* 2E054 8003D854 0600622A */  slti       $v0, $s3, 0x6
    /* 2E058 8003D858 E6FF4014 */  bnez       $v0, .L8003D7F4
    /* 2E05C 8003D85C 02003126 */   addiu     $s1, $s1, 0x2
    /* 2E060 8003D860 0C004226 */  addiu      $v0, $s2, 0xC
    /* 2E064 8003D864 2400BF8F */  lw         $ra, 0x24($sp)
    /* 2E068 8003D868 2000B48F */  lw         $s4, 0x20($sp)
    /* 2E06C 8003D86C 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 2E070 8003D870 1800B28F */  lw         $s2, 0x18($sp)
    /* 2E074 8003D874 1400B18F */  lw         $s1, 0x14($sp)
    /* 2E078 8003D878 1000B08F */  lw         $s0, 0x10($sp)
    /* 2E07C 8003D87C 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 2E080 8003D880 0800E003 */  jr         $ra
    /* 2E084 8003D884 00000000 */   nop
endlabel func_8003D7B4
