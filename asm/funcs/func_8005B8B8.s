glabel func_8005B8B8
    /* 4C0B8 8005B8B8 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 4C0BC 8005B8BC 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 4C0C0 8005B8C0 21988000 */  addu       $s3, $a0, $zero
    /* 4C0C4 8005B8C4 2000BFAF */  sw         $ra, 0x20($sp)
    /* 4C0C8 8005B8C8 1800B2AF */  sw         $s2, 0x18($sp)
    /* 4C0CC 8005B8CC 1400B1AF */  sw         $s1, 0x14($sp)
    /* 4C0D0 8005B8D0 1A6E010C */  jal        func_8005B868
    /* 4C0D4 8005B8D4 1000B0AF */   sw        $s0, 0x10($sp)
    /* 4C0D8 8005B8D8 3416020C */  jal        func_800858D0
    /* 4C0DC 8005B8DC 21200000 */   addu      $a0, $zero, $zero
    /* 4C0E0 8005B8E0 02000424 */  addiu      $a0, $zero, 0x2
    /* 4C0E4 8005B8E4 AADB000C */  jal        func_80036EA8
    /* 4C0E8 8005B8E8 5D000524 */   addiu     $a1, $zero, 0x5D
    /* 4C0EC 8005B8EC D0DB000C */  jal        func_80036F40
    /* 4C0F0 8005B8F0 21804000 */   addu      $s0, $v0, $zero
    /* 4C0F4 8005B8F4 21200002 */  addu       $a0, $s0, $zero
    /* 4C0F8 8005B8F8 66DB000C */  jal        replay_camera_Init
    /* 4C0FC 8005B8FC 21286002 */   addu      $a1, $s3, $zero
    /* 4C100 8005B900 CADB000C */  jal        func_80036F28
    /* 4C104 8005B904 21200002 */   addu      $a0, $s0, $zero
    /* 4C108 8005B908 D0DB000C */  jal        func_80036F40
    /* 4C10C 8005B90C 21884000 */   addu      $s1, $v0, $zero
    /* 4C110 8005B910 21206002 */  addu       $a0, $s3, $zero
    /* 4C114 8005B914 08000524 */  addiu      $a1, $zero, 0x8
    /* 4C118 8005B918 AA70010C */  jal        func_8005C2A8
    /* 4C11C 8005B91C 21307102 */   addu      $a2, $s3, $s1
    /* 4C120 8005B920 02000424 */  addiu      $a0, $zero, 0x2
    /* 4C124 8005B924 5E000524 */  addiu      $a1, $zero, 0x5E
    /* 4C128 8005B928 AADB000C */  jal        func_80036EA8
    /* 4C12C 8005B92C 21804000 */   addu      $s0, $v0, $zero
    /* 4C130 8005B930 D0DB000C */  jal        func_80036F40
    /* 4C134 8005B934 21884000 */   addu      $s1, $v0, $zero
    /* 4C138 8005B938 21202002 */  addu       $a0, $s1, $zero
    /* 4C13C 8005B93C 21907002 */  addu       $s2, $s3, $s0
    /* 4C140 8005B940 66DB000C */  jal        replay_camera_Init
    /* 4C144 8005B944 21284002 */   addu      $a1, $s2, $zero
    /* 4C148 8005B948 CADB000C */  jal        func_80036F28
    /* 4C14C 8005B94C 21202002 */   addu      $a0, $s1, $zero
    /* 4C150 8005B950 D0DB000C */  jal        func_80036F40
    /* 4C154 8005B954 21885000 */   addu      $s1, $v0, $s0
    /* 4C158 8005B958 21204002 */  addu       $a0, $s2, $zero
    /* 4C15C 8005B95C 04000524 */  addiu      $a1, $zero, 0x4
    /* 4C160 8005B960 AA70010C */  jal        func_8005C2A8
    /* 4C164 8005B964 21307102 */   addu      $a2, $s3, $s1
    /* 4C168 8005B968 21105000 */  addu       $v0, $v0, $s0
    /* 4C16C 8005B96C 2000BF8F */  lw         $ra, 0x20($sp)
    /* 4C170 8005B970 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 4C174 8005B974 1800B28F */  lw         $s2, 0x18($sp)
    /* 4C178 8005B978 1400B18F */  lw         $s1, 0x14($sp)
    /* 4C17C 8005B97C 1000B08F */  lw         $s0, 0x10($sp)
    /* 4C180 8005B980 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 4C184 8005B984 0800E003 */  jr         $ra
    /* 4C188 8005B988 00000000 */   nop
endlabel func_8005B8B8
