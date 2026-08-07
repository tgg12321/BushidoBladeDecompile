glabel obj_InitAll
    /* 4BF2C 8005B72C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 4BF30 8005B730 21200000 */  addu       $a0, $zero, $zero
    /* 4BF34 8005B734 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 4BF38 8005B738 1800B2AF */  sw         $s2, 0x18($sp)
    /* 4BF3C 8005B73C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 4BF40 8005B740 3416020C */  jal        func_800858D0
    /* 4BF44 8005B744 1000B0AF */   sw        $s0, 0x10($sp)
    /* 4BF48 8005B748 E617020C */  jal        func_80085F98
    /* 4BF4C 8005B74C 01001024 */   addiu     $s0, $zero, 0x1
    /* 4BF50 8005B750 B917020C */  jal        func_80085EE4
    /* 4BF54 8005B754 21200000 */   addu      $a0, $zero, $zero
    /* 4BF58 8005B758 21200000 */  addu       $a0, $zero, $zero
    /* 4BF5C 8005B75C 9317020C */  jal        func_80085E4C
    /* 4BF60 8005B760 21280000 */   addu      $a1, $zero, $zero
    /* 4BF64 8005B764 0F80123C */  lui        $s2, %hi(D_800EFB3C)
    /* 4BF68 8005B768 3CFB5226 */  addiu      $s2, $s2, %lo(D_800EFB3C)
    /* 4BF6C 8005B76C 0F80113C */  lui        $s1, %hi(D_800EFC3C)
    /* 4BF70 8005B770 3CFC3126 */  addiu      $s1, $s1, %lo(D_800EFC3C)
  .L8005B774:
    /* 4BF74 8005B774 00241000 */  sll        $a0, $s0, 16
    /* 4BF78 8005B778 D91F020C */  jal        func_80087F64
    /* 4BF7C 8005B77C 03240400 */   sra       $a0, $a0, 16
    /* 4BF80 8005B780 000020AE */  sw         $zero, 0x0($s1)
    /* 4BF84 8005B784 000040AE */  sw         $zero, 0x0($s2)
    /* 4BF88 8005B788 04005226 */  addiu      $s2, $s2, 0x4
    /* 4BF8C 8005B78C 01001026 */  addiu      $s0, $s0, 0x1
    /* 4BF90 8005B790 1000022A */  slti       $v0, $s0, 0x10
    /* 4BF94 8005B794 F7FF4014 */  bnez       $v0, .L8005B774
    /* 4BF98 8005B798 04003126 */   addiu     $s1, $s1, 0x4
    /* 4BF9C 8005B79C 3C0380AF */  sw         $zero, %gp_rel(D_800A3408)($gp)
    /* 4BFA0 8005B7A0 6B6D010C */  jal        obj_InitChars
    /* 4BFA4 8005B7A4 00000000 */   nop
    /* 4BFA8 8005B7A8 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 4BFAC 8005B7AC 1800B28F */  lw         $s2, 0x18($sp)
    /* 4BFB0 8005B7B0 1400B18F */  lw         $s1, 0x14($sp)
    /* 4BFB4 8005B7B4 1000B08F */  lw         $s0, 0x10($sp)
    /* 4BFB8 8005B7B8 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 4BFBC 8005B7BC 0800E003 */  jr         $ra
    /* 4BFC0 8005B7C0 00000000 */   nop
endlabel obj_InitAll
