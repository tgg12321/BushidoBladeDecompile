glabel func_8003AFFC
    /* 2B7FC 8003AFFC D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 2B800 8003B000 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 2B804 8003B004 1980133C */  lui        $s3, (0x80190800 >> 16)
    /* 2B808 8003B008 00087336 */  ori        $s3, $s3, (0x80190800 & 0xFFFF)
    /* 2B80C 8003B00C 2400BFAF */  sw         $ra, 0x24($sp)
    /* 2B810 8003B010 2000B4AF */  sw         $s4, 0x20($sp)
    /* 2B814 8003B014 1800B2AF */  sw         $s2, 0x18($sp)
    /* 2B818 8003B018 1400B1AF */  sw         $s1, 0x14($sp)
    /* 2B81C 8003B01C 1A5A000C */  jal        gpu_EnableDisplay
    /* 2B820 8003B020 1000B0AF */   sw        $s0, 0x10($sp)
    /* 2B824 8003B024 4E83000C */  jal        func_80020D38
    /* 2B828 8003B028 21900000 */   addu      $s2, $zero, $zero
    /* 2B82C 8003B02C E724010C */  jal        func_8004939C
    /* 2B830 8003B030 21800000 */   addu      $s0, $zero, $zero
    /* 2B834 8003B034 0980143C */  lui        $s4, %hi(D_8008E5CC)
    /* 2B838 8003B038 CCE59426 */  addiu      $s4, $s4, %lo(D_8008E5CC)
    /* 2B83C 8003B03C 1080113C */  lui        $s1, %hi(D_80101EDC)
    /* 2B840 8003B040 DC1E3126 */  addiu      $s1, $s1, %lo(D_80101EDC)
  .L8003B044:
    /* 2B844 8003B044 1080013C */  lui        $at, %hi(D_80101EDA)
    /* 2B848 8003B048 21083000 */  addu       $at, $at, $s0
    /* 2B84C 8003B04C DA1E2484 */  lh         $a0, %lo(D_80101EDA)($at)
    /* 2B850 8003B050 F924010C */  jal        func_800493E4
    /* 2B854 8003B054 00000000 */   nop
    /* 2B858 8003B058 1080013C */  lui        $at, %hi(D_80101ED2)
    /* 2B85C 8003B05C 21083000 */  addu       $at, $at, $s0
    /* 2B860 8003B060 D21E2284 */  lh         $v0, %lo(D_80101ED2)($at)
    /* 2B864 8003B064 1080013C */  lui        $at, %hi(D_80101ED6)
    /* 2B868 8003B068 21083000 */  addu       $at, $at, $s0
    /* 2B86C 8003B06C D61E2384 */  lh         $v1, %lo(D_80101ED6)($at)
    /* 2B870 8003B070 C0100200 */  sll        $v0, $v0, 3
    /* 2B874 8003B074 21105400 */  addu       $v0, $v0, $s4
    /* 2B878 8003B078 21104300 */  addu       $v0, $v0, $v1
    /* 2B87C 8003B07C 00004590 */  lbu        $a1, 0x0($v0)
    /* 2B880 8003B080 3525010C */  jal        func_800494D4
    /* 2B884 8003B084 21204002 */   addu      $a0, $s2, $zero
    /* 2B888 8003B088 00002386 */  lh         $v1, 0x0($s1)
    /* 2B88C 8003B08C FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 2B890 8003B090 0E006210 */  beq        $v1, $v0, .L8003B0CC
    /* 2B894 8003B094 00000000 */   nop
    /* 2B898 8003B098 0980013C */  lui        $at, %hi(D_8008EB80)
    /* 2B89C 8003B09C 21082300 */  addu       $at, $at, $v1
    /* 2B8A0 8003B0A0 80EB2490 */  lbu        $a0, %lo(D_8008EB80)($at)
    /* 2B8A4 8003B0A4 F924010C */  jal        func_800493E4
    /* 2B8A8 8003B0A8 00000000 */   nop
    /* 2B8AC 8003B0AC 00002386 */  lh         $v1, 0x0($s1)
    /* 2B8B0 8003B0B0 0E000224 */  addiu      $v0, $zero, 0xE
    /* 2B8B4 8003B0B4 05006214 */  bne        $v1, $v0, .L8003B0CC
    /* 2B8B8 8003B0B8 00000000 */   nop
    /* 2B8BC 8003B0BC 0980043C */  lui        $a0, %hi(D_8008EB8E)
    /* 2B8C0 8003B0C0 8EEB8490 */  lbu        $a0, %lo(D_8008EB8E)($a0)
    /* 2B8C4 8003B0C4 F924010C */  jal        func_800493E4
    /* 2B8C8 8003B0C8 03008424 */   addiu     $a0, $a0, 0x3
  .L8003B0CC:
    /* 2B8CC 8003B0CC 4C043126 */  addiu      $s1, $s1, 0x44C
    /* 2B8D0 8003B0D0 01005226 */  addiu      $s2, $s2, 0x1
    /* 2B8D4 8003B0D4 0200422A */  slti       $v0, $s2, 0x2
    /* 2B8D8 8003B0D8 DAFF4014 */  bnez       $v0, .L8003B044
    /* 2B8DC 8003B0DC 4C041026 */   addiu     $s0, $s0, 0x44C
    /* 2B8E0 8003B0E0 6125010C */  jal        func_80049584
    /* 2B8E4 8003B0E4 21206002 */   addu      $a0, $s3, $zero
    /* 2B8E8 8003B0E8 2400BF8F */  lw         $ra, 0x24($sp)
    /* 2B8EC 8003B0EC 2000B48F */  lw         $s4, 0x20($sp)
    /* 2B8F0 8003B0F0 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 2B8F4 8003B0F4 1800B28F */  lw         $s2, 0x18($sp)
    /* 2B8F8 8003B0F8 1400B18F */  lw         $s1, 0x14($sp)
    /* 2B8FC 8003B0FC 1000B08F */  lw         $s0, 0x10($sp)
    /* 2B900 8003B100 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 2B904 8003B104 0800E003 */  jr         $ra
    /* 2B908 8003B108 00000000 */   nop
endlabel func_8003AFFC
