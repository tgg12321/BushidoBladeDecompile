glabel func_8007B3A8
    /* 6BBA8 8007B3A8 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6BBAC 8007B3AC 21408000 */  addu       $t0, $a0, $zero
    /* 6BBB0 8007B3B0 1800B0AF */  sw         $s0, 0x18($sp)
    /* 6BBB4 8007B3B4 2180A000 */  addu       $s0, $a1, $zero
    /* 6BBB8 8007B3B8 0A80033C */  lui        $v1, %hi(D_8009BE76)
    /* 6BBBC 8007B3BC 76BE6390 */  lbu        $v1, %lo(D_8009BE76)($v1)
    /* 6BBC0 8007B3C0 01000224 */  addiu      $v0, $zero, 0x1
    /* 6BBC4 8007B3C4 06006210 */  beq        $v1, $v0, .L8007B3E0
    /* 6BBC8 8007B3C8 1C00BFAF */   sw        $ra, 0x1C($sp)
    /* 6BBCC 8007B3CC 02000224 */  addiu      $v0, $zero, 0x2
    /* 6BBD0 8007B3D0 29006210 */  beq        $v1, $v0, .L8007B478
    /* 6BBD4 8007B3D4 00000000 */   nop
    /* 6BBD8 8007B3D8 2FED0108 */  j          .L8007B4BC
    /* 6BBDC 8007B3DC 00000000 */   nop
  .L8007B3E0:
    /* 6BBE0 8007B3E0 04000586 */  lh         $a1, 0x4($s0)
    /* 6BBE4 8007B3E4 0A80033C */  lui        $v1, %hi(D_8009BE78)
    /* 6BBE8 8007B3E8 78BE6384 */  lh         $v1, %lo(D_8009BE78)($v1)
    /* 6BBEC 8007B3EC 00000000 */  nop
    /* 6BBF0 8007B3F0 2A106500 */  slt        $v0, $v1, $a1
    /* 6BBF4 8007B3F4 1C004014 */  bnez       $v0, .L8007B468
    /* 6BBF8 8007B3F8 00000000 */   nop
    /* 6BBFC 8007B3FC 00000786 */  lh         $a3, 0x0($s0)
    /* 6BC00 8007B400 00000000 */  nop
    /* 6BC04 8007B404 2110A700 */  addu       $v0, $a1, $a3
    /* 6BC08 8007B408 2A106200 */  slt        $v0, $v1, $v0
    /* 6BC0C 8007B40C 16004014 */  bnez       $v0, .L8007B468
    /* 6BC10 8007B410 00000000 */   nop
    /* 6BC14 8007B414 02000386 */  lh         $v1, 0x2($s0)
    /* 6BC18 8007B418 0A80043C */  lui        $a0, %hi(D_8009BE7A)
    /* 6BC1C 8007B41C 7ABE8484 */  lh         $a0, %lo(D_8009BE7A)($a0)
    /* 6BC20 8007B420 00000000 */  nop
    /* 6BC24 8007B424 2A108300 */  slt        $v0, $a0, $v1
    /* 6BC28 8007B428 0F004014 */  bnez       $v0, .L8007B468
    /* 6BC2C 8007B42C 00000000 */   nop
    /* 6BC30 8007B430 06000686 */  lh         $a2, 0x6($s0)
    /* 6BC34 8007B434 00000000 */  nop
    /* 6BC38 8007B438 21106600 */  addu       $v0, $v1, $a2
    /* 6BC3C 8007B43C 2A108200 */  slt        $v0, $a0, $v0
    /* 6BC40 8007B440 09004014 */  bnez       $v0, .L8007B468
    /* 6BC44 8007B444 00000000 */   nop
    /* 6BC48 8007B448 0700A018 */  blez       $a1, .L8007B468
    /* 6BC4C 8007B44C 00000000 */   nop
    /* 6BC50 8007B450 0500E004 */  bltz       $a3, .L8007B468
    /* 6BC54 8007B454 00000000 */   nop
    /* 6BC58 8007B458 03006004 */  bltz       $v1, .L8007B468
    /* 6BC5C 8007B45C 00000000 */   nop
    /* 6BC60 8007B460 1600C01C */  bgtz       $a2, .L8007B4BC
    /* 6BC64 8007B464 00000000 */   nop
  .L8007B468:
    /* 6BC68 8007B468 0180043C */  lui        $a0, %hi(D_80015F2C)
    /* 6BC6C 8007B46C 2C5F8424 */  addiu      $a0, $a0, %lo(D_80015F2C)
    /* 6BC70 8007B470 20ED0108 */  j          .L8007B480
    /* 6BC74 8007B474 00000000 */   nop
  .L8007B478:
    /* 6BC78 8007B478 0180043C */  lui        $a0, %hi(D_80015F4C)
    /* 6BC7C 8007B47C 4C5F8424 */  addiu      $a0, $a0, %lo(D_80015F4C)
  .L8007B480:
    /* 6BC80 8007B480 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6BC84 8007B484 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6BC88 8007B488 00000000 */  nop
    /* 6BC8C 8007B48C 09F84000 */  jalr       $v0
    /* 6BC90 8007B490 21280001 */   addu      $a1, $t0, $zero
    /* 6BC94 8007B494 00000586 */  lh         $a1, 0x0($s0)
    /* 6BC98 8007B498 02000686 */  lh         $a2, 0x2($s0)
    /* 6BC9C 8007B49C 04000786 */  lh         $a3, 0x4($s0)
    /* 6BCA0 8007B4A0 06000286 */  lh         $v0, 0x6($s0)
    /* 6BCA4 8007B4A4 0A80033C */  lui        $v1, %hi(D_8009BE70)
    /* 6BCA8 8007B4A8 70BE638C */  lw         $v1, %lo(D_8009BE70)($v1)
    /* 6BCAC 8007B4AC 0180043C */  lui        $a0, %hi(D_80015F38)
    /* 6BCB0 8007B4B0 385F8424 */  addiu      $a0, $a0, %lo(D_80015F38)
    /* 6BCB4 8007B4B4 09F86000 */  jalr       $v1
    /* 6BCB8 8007B4B8 1000A2AF */   sw        $v0, 0x10($sp)
  .L8007B4BC:
    /* 6BCBC 8007B4BC 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 6BCC0 8007B4C0 1800B08F */  lw         $s0, 0x18($sp)
    /* 6BCC4 8007B4C4 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6BCC8 8007B4C8 0800E003 */  jr         $ra
    /* 6BCCC 8007B4CC 00000000 */   nop
endlabel func_8007B3A8
