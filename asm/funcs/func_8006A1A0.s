glabel func_8006A1A0
    /* 5A9A0 8006A1A0 90FFBD27 */  addiu      $sp, $sp, -0x70
    /* 5A9A4 8006A1A4 5800B0AF */  sw         $s0, 0x58($sp)
    /* 5A9A8 8006A1A8 21808000 */  addu       $s0, $a0, $zero
    /* 5A9AC 8006A1AC 6000B2AF */  sw         $s2, 0x60($sp)
    /* 5A9B0 8006A1B0 2190A000 */  addu       $s2, $a1, $zero
    /* 5A9B4 8006A1B4 01004232 */  andi       $v0, $s2, 0x1
    /* 5A9B8 8006A1B8 6C00BFAF */  sw         $ra, 0x6C($sp)
    /* 5A9BC 8006A1BC 6800B4AF */  sw         $s4, 0x68($sp)
    /* 5A9C0 8006A1C0 6400B3AF */  sw         $s3, 0x64($sp)
    /* 5A9C4 8006A1C4 78004010 */  beqz       $v0, .L8006A3A8
    /* 5A9C8 8006A1C8 5C00B1AF */   sw        $s1, 0x5C($sp)
    /* 5A9CC 8006A1CC 0400028E */  lw         $v0, 0x4($s0)
    /* 5A9D0 8006A1D0 00000000 */  nop
    /* 5A9D4 8006A1D4 1C00538C */  lw         $s3, 0x1C($v0)
    /* 5A9D8 8006A1D8 00000000 */  nop
    /* 5A9DC 8006A1DC 0800628E */  lw         $v0, 0x8($s3)
    /* 5A9E0 8006A1E0 00000000 */  nop
    /* 5A9E4 8006A1E4 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5A9E8 8006A1E8 02004232 */  andi       $v0, $s2, 0x2
    /* 5A9EC 8006A1EC 05004010 */  beqz       $v0, .L8006A204
    /* 5A9F0 8006A1F0 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 5A9F4 8006A1F4 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5A9F8 8006A1F8 01000224 */  addiu      $v0, $zero, 0x1
    /* 5A9FC 8006A1FC 84A80108 */  j          .L8006A210
    /* 5AA00 8006A200 4000A2A3 */   sb        $v0, 0x40($sp)
  .L8006A204:
    /* 5AA04 8006A204 4E000224 */  addiu      $v0, $zero, 0x4E
    /* 5AA08 8006A208 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5AA0C 8006A20C 4000A0A3 */  sb         $zero, 0x40($sp)
  .L8006A210:
    /* 5AA10 8006A210 5804828F */  lw         $v0, %gp_rel(D_800A3524)($gp)
    /* 5AA14 8006A214 00000000 */  nop
    /* 5AA18 8006A218 2000428C */  lw         $v0, 0x20($v0)
    /* 5AA1C 8006A21C 3000B18F */  lw         $s1, 0x30($sp)
    /* 5AA20 8006A220 08004230 */  andi       $v0, $v0, 0x8
    /* 5AA24 8006A224 1A004014 */  bnez       $v0, .L8006A290
    /* 5AA28 8006A228 32002226 */   addiu     $v0, $s1, 0x32
    /* 5AA2C 8006A22C 02004232 */  andi       $v0, $s2, 0x2
    /* 5AA30 8006A230 13004010 */  beqz       $v0, .L8006A280
    /* 5AA34 8006A234 00010224 */   addiu     $v0, $zero, 0x100
    /* 5AA38 8006A238 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5AA3C 8006A23C 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5AA40 8006A240 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5AA44 8006A244 0C004284 */  lh         $v0, 0xC($v0)
    /* 5AA48 8006A248 C0210400 */  sll        $a0, $a0, 7
    /* 5AA4C 8006A24C 21102202 */  addu       $v0, $s1, $v0
    /* 5AA50 8006A250 C8F7010C */  jal        func_8007DF20
    /* 5AA54 8006A254 3000A2AF */   sw        $v0, 0x30($sp)
    /* 5AA58 8006A258 40180200 */  sll        $v1, $v0, 1
    /* 5AA5C 8006A25C 21186200 */  addu       $v1, $v1, $v0
    /* 5AA60 8006A260 00190300 */  sll        $v1, $v1, 4
    /* 5AA64 8006A264 23186200 */  subu       $v1, $v1, $v0
    /* 5AA68 8006A268 031B0300 */  sra        $v1, $v1, 12
    /* 5AA6C 8006A26C 80FF6324 */  addiu      $v1, $v1, -0x80
    /* 5AA70 8006A270 4300A3A3 */  sb         $v1, 0x43($sp)
    /* 5AA74 8006A274 4200A3A3 */  sb         $v1, 0x42($sp)
    /* 5AA78 8006A278 4100A3A3 */  sb         $v1, 0x41($sp)
    /* 5AA7C 8006A27C 00010224 */  addiu      $v0, $zero, 0x100
  .L8006A280:
    /* 5AA80 8006A280 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5AA84 8006A284 3C00A2AF */  sw         $v0, 0x3C($sp)
    /* 5AA88 8006A288 AEA80108 */  j          .L8006A2B8
    /* 5AA8C 8006A28C 3800A2AF */   sw        $v0, 0x38($sp)
  .L8006A290:
    /* 5AA90 8006A290 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5AA94 8006A294 70000224 */  addiu      $v0, $zero, 0x70
    /* 5AA98 8006A298 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5AA9C 8006A29C 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5AAA0 8006A2A0 4100A2A3 */  sb         $v0, 0x41($sp)
    /* 5AAA4 8006A2A4 80000224 */  addiu      $v0, $zero, 0x80
    /* 5AAA8 8006A2A8 3C00A2AF */  sw         $v0, 0x3C($sp)
    /* 5AAAC 8006A2AC 3800A2AF */  sw         $v0, 0x38($sp)
    /* 5AAB0 8006A2B0 0A000224 */  addiu      $v0, $zero, 0xA
    /* 5AAB4 8006A2B4 3400A2AF */  sw         $v0, 0x34($sp)
  .L8006A2B8:
    /* 5AAB8 8006A2B8 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5AABC 8006A2BC 1800A28F */  lw         $v0, 0x18($sp)
    /* 5AAC0 8006A2C0 02001424 */  addiu      $s4, $zero, 0x2
    /* 5AAC4 8006A2C4 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5AAC8 8006A2C8 2C00B4AF */  sw         $s4, 0x2C($sp)
    /* 5AACC 8006A2CC 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5AAD0 8006A2D0 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5AAD4 8006A2D4 0800028E */  lw         $v0, 0x8($s0)
    /* 5AAD8 8006A2D8 21280000 */  addu       $a1, $zero, $zero
    /* 5AADC 8006A2DC CACD010C */  jal        func_80073728
    /* 5AAE0 8006A2E0 2400A2AF */   sw        $v0, 0x24($sp)
    /* 5AAE4 8006A2E4 080002AE */  sw         $v0, 0x8($s0)
    /* 5AAE8 8006A2E8 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5AAEC 8006A2EC 3000B1AF */  sw         $s1, 0x30($sp)
    /* 5AAF0 8006A2F0 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5AAF4 8006A2F4 0C00628E */  lw         $v0, 0xC($s3)
    /* 5AAF8 8006A2F8 00000000 */  nop
    /* 5AAFC 8006A2FC 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5AB00 8006A300 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5AB04 8006A304 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5AB08 8006A308 1400028E */  lw         $v0, 0x14($s0)
    /* 5AB0C 8006A30C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5AB10 8006A310 4BCD010C */  jal        func_8007352C
    /* 5AB14 8006A314 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5AB18 8006A318 140002AE */  sw         $v0, 0x14($s0)
    /* 5AB1C 8006A31C 02004232 */  andi       $v0, $s2, 0x2
    /* 5AB20 8006A320 0F004010 */  beqz       $v0, .L8006A360
    /* 5AB24 8006A324 01000324 */   addiu     $v1, $zero, 0x1
    /* 5AB28 8006A328 2C00B4AF */  sw         $s4, 0x2C($sp)
    /* 5AB2C 8006A32C 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5AB30 8006A330 1800628E */  lw         $v0, 0x18($s3)
    /* 5AB34 8006A334 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5AB38 8006A338 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5AB3C 8006A33C 4000A3A3 */  sb         $v1, 0x40($sp)
    /* 5AB40 8006A340 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5AB44 8006A344 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5AB48 8006A348 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5AB4C 8006A34C 1400028E */  lw         $v0, 0x14($s0)
    /* 5AB50 8006A350 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5AB54 8006A354 4BCD010C */  jal        func_8007352C
    /* 5AB58 8006A358 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5AB5C 8006A35C 140002AE */  sw         $v0, 0x14($s0)
  .L8006A360:
    /* 5AB60 8006A360 1800A48F */  lw         $a0, 0x18($sp)
    /* 5AB64 8006A364 20B9010C */  jal        func_8006E480
    /* 5AB68 8006A368 21280000 */   addu      $a1, $zero, $zero
    /* 5AB6C 8006A36C 01000524 */  addiu      $a1, $zero, 0x1
    /* 5AB70 8006A370 21300000 */  addu       $a2, $zero, $zero
    /* 5AB74 8006A374 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5AB78 8006A378 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5AB7C 8006A37C 92F0010C */  jal        func_8007C248
    /* 5AB80 8006A380 21384000 */   addu      $a3, $v0, $zero
    /* 5AB84 8006A384 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5AB88 8006A388 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5AB8C 8006A38C 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5AB90 8006A390 2DEA010C */  jal        func_8007A8B4
    /* 5AB94 8006A394 08008424 */   addiu     $a0, $a0, 0x8
    /* 5AB98 8006A398 1C00028E */  lw         $v0, 0x1C($s0)
    /* 5AB9C 8006A39C 00000000 */  nop
    /* 5ABA0 8006A3A0 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5ABA4 8006A3A4 1C0002AE */  sw         $v0, 0x1C($s0)
  .L8006A3A8:
    /* 5ABA8 8006A3A8 6C00BF8F */  lw         $ra, 0x6C($sp)
    /* 5ABAC 8006A3AC 6800B48F */  lw         $s4, 0x68($sp)
    /* 5ABB0 8006A3B0 6400B38F */  lw         $s3, 0x64($sp)
    /* 5ABB4 8006A3B4 6000B28F */  lw         $s2, 0x60($sp)
    /* 5ABB8 8006A3B8 5C00B18F */  lw         $s1, 0x5C($sp)
    /* 5ABBC 8006A3BC 5800B08F */  lw         $s0, 0x58($sp)
    /* 5ABC0 8006A3C0 7000BD27 */  addiu      $sp, $sp, 0x70
    /* 5ABC4 8006A3C4 0800E003 */  jr         $ra
    /* 5ABC8 8006A3C8 00000000 */   nop
endlabel func_8006A1A0
