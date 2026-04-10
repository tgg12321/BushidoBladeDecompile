glabel gpu_SetDispMask
    /* 6BAA0 8007B2A0 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6BAA4 8007B2A4 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BAA8 8007B2A8 0A80113C */  lui        $s1, %hi(D_8009BE76)
    /* 6BAAC 8007B2AC 76BE3126 */  addiu      $s1, $s1, %lo(D_8009BE76)
    /* 6BAB0 8007B2B0 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6BAB4 8007B2B4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BAB8 8007B2B8 00002292 */  lbu        $v0, 0x0($s1)
    /* 6BABC 8007B2BC 00000000 */  nop
    /* 6BAC0 8007B2C0 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6BAC4 8007B2C4 08004014 */  bnez       $v0, .L8007B2E8
    /* 6BAC8 8007B2C8 21808000 */   addu      $s0, $a0, $zero
    /* 6BACC 8007B2CC 0180043C */  lui        $a0, %hi(D_80015F04)
    /* 6BAD0 8007B2D0 045F8424 */  addiu      $a0, $a0, %lo(D_80015F04)
    /* 6BAD4 8007B2D4 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6BAD8 8007B2D8 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6BADC 8007B2DC 00000000 */  nop
    /* 6BAE0 8007B2E0 09F84000 */  jalr       $v0
    /* 6BAE4 8007B2E4 21280002 */   addu      $a1, $s0, $zero
  .L8007B2E8:
    /* 6BAE8 8007B2E8 04000016 */  bnez       $s0, .L8007B2FC
    /* 6BAEC 8007B2EC 6A002426 */   addiu     $a0, $s1, 0x6A
    /* 6BAF0 8007B2F0 FFFF0524 */  addiu      $a1, $zero, -0x1
    /* 6BAF4 8007B2F4 B9F7010C */  jal        bb2_memset
    /* 6BAF8 8007B2F8 14000624 */   addiu     $a2, $zero, 0x14
  .L8007B2FC:
    /* 6BAFC 8007B2FC 0003043C */  lui        $a0, (0x3000001 >> 16)
    /* 6BB00 8007B300 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6BB04 8007B304 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6BB08 8007B308 02000012 */  beqz       $s0, .L8007B314
    /* 6BB0C 8007B30C 01008434 */   ori       $a0, $a0, (0x3000001 & 0xFFFF)
    /* 6BB10 8007B310 0003043C */  lui        $a0, (0x3000000 >> 16)
  .L8007B314:
    /* 6BB14 8007B314 1000428C */  lw         $v0, 0x10($v0)
    /* 6BB18 8007B318 00000000 */  nop
    /* 6BB1C 8007B31C 09F84000 */  jalr       $v0
    /* 6BB20 8007B320 00000000 */   nop
    /* 6BB24 8007B324 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6BB28 8007B328 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BB2C 8007B32C 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BB30 8007B330 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6BB34 8007B334 0800E003 */  jr         $ra
    /* 6BB38 8007B338 00000000 */   nop
endlabel gpu_SetDispMask
