glabel func_8006B120
    /* 5B920 8006B120 98FFBD27 */  addiu      $sp, $sp, -0x68
    /* 5B924 8006B124 5400B1AF */  sw         $s1, 0x54($sp)
    /* 5B928 8006B128 21888000 */  addu       $s1, $a0, $zero
    /* 5B92C 8006B12C 0A000224 */  addiu      $v0, $zero, 0xA
    /* 5B930 8006B130 6400BFAF */  sw         $ra, 0x64($sp)
    /* 5B934 8006B134 6000B4AF */  sw         $s4, 0x60($sp)
    /* 5B938 8006B138 5C00B3AF */  sw         $s3, 0x5C($sp)
    /* 5B93C 8006B13C 5800B2AF */  sw         $s2, 0x58($sp)
    /* 5B940 8006B140 5000B0AF */  sw         $s0, 0x50($sp)
    /* 5B944 8006B144 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5B948 8006B148 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5B94C 8006B14C 0400228E */  lw         $v0, 0x4($s1)
    /* 5B950 8006B150 00000000 */  nop
    /* 5B954 8006B154 2800538C */  lw         $s3, 0x28($v0)
    /* 5B958 8006B158 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5B95C 8006B15C 0000628E */  lw         $v0, 0x0($s3)
    /* 5B960 8006B160 21800000 */  addu       $s0, $zero, $zero
    /* 5B964 8006B164 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5B968 8006B168 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5B96C 8006B16C 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5B970 8006B170 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5B974 8006B174 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5B978 8006B178 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5B97C 8006B17C 1400228E */  lw         $v0, 0x14($s1)
    /* 5B980 8006B180 01001424 */  addiu      $s4, $zero, 0x1
    /* 5B984 8006B184 4BCD010C */  jal        func_8007352C
    /* 5B988 8006B188 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5B98C 8006B18C 140022AE */  sw         $v0, 0x14($s1)
    /* 5B990 8006B190 1800A48F */  lw         $a0, 0x18($sp)
    /* 5B994 8006B194 20B9010C */  jal        func_8006E480
    /* 5B998 8006B198 21280000 */   addu      $a1, $zero, $zero
    /* 5B99C 8006B19C 01000524 */  addiu      $a1, $zero, 0x1
    /* 5B9A0 8006B1A0 21300000 */  addu       $a2, $zero, $zero
    /* 5B9A4 8006B1A4 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5B9A8 8006B1A8 1C00248E */  lw         $a0, 0x1C($s1)
    /* 5B9AC 8006B1AC 92F0010C */  jal        initTexPage
    /* 5B9B0 8006B1B0 21384000 */   addu      $a3, $v0, $zero
    /* 5B9B4 8006B1B4 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5B9B8 8006B1B8 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5B9BC 8006B1BC 1C00258E */  lw         $a1, 0x1C($s1)
    /* 5B9C0 8006B1C0 2DEA010C */  jal        ot_Link
    /* 5B9C4 8006B1C4 28008424 */   addiu     $a0, $a0, 0x28
    /* 5B9C8 8006B1C8 1C00228E */  lw         $v0, 0x1C($s1)
    /* 5B9CC 8006B1CC 21906002 */  addu       $s2, $s3, $zero
    /* 5B9D0 8006B1D0 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B9D4 8006B1D4 1C0022AE */  sw         $v0, 0x1C($s1)
    /* 5B9D8 8006B1D8 3000A0AF */  sw         $zero, 0x30($sp)
  .L8006B1DC:
    /* 5B9DC 8006B1DC 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5B9E0 8006B1E0 0400438E */  lw         $v1, 0x4($s2)
    /* 5B9E4 8006B1E4 82120200 */  srl        $v0, $v0, 10
    /* 5B9E8 8006B1E8 07004230 */  andi       $v0, $v0, 0x7
    /* 5B9EC 8006B1EC 15005014 */  bne        $v0, $s0, .L8006B244
    /* 5B9F0 8006B1F0 1800A3AF */   sw        $v1, 0x18($sp)
    /* 5B9F4 8006B1F4 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5B9F8 8006B1F8 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5B9FC 8006B1FC 4000B4A3 */  sb         $s4, 0x40($sp)
    /* 5BA00 8006B200 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5BA04 8006B204 C0210400 */  sll        $a0, $a0, 7
    /* 5BA08 8006B208 0E004284 */  lh         $v0, 0xE($v0)
    /* 5BA0C 8006B20C FF018424 */  addiu      $a0, $a0, 0x1FF
    /* 5BA10 8006B210 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5BA14 8006B214 C8F7010C */  jal        math_Sin
    /* 5BA18 8006B218 3400A2AF */   sw        $v0, 0x34($sp)
    /* 5BA1C 8006B21C 40180200 */  sll        $v1, $v0, 1
    /* 5BA20 8006B220 21186200 */  addu       $v1, $v1, $v0
    /* 5BA24 8006B224 00190300 */  sll        $v1, $v1, 4
    /* 5BA28 8006B228 23186200 */  subu       $v1, $v1, $v0
    /* 5BA2C 8006B22C 031B0300 */  sra        $v1, $v1, 12
    /* 5BA30 8006B230 80FF6324 */  addiu      $v1, $v1, -0x80
    /* 5BA34 8006B234 4300A3A3 */  sb         $v1, 0x43($sp)
    /* 5BA38 8006B238 4200A3A3 */  sb         $v1, 0x42($sp)
    /* 5BA3C 8006B23C 94AC0108 */  j          .L8006B250
    /* 5BA40 8006B240 4100A3A3 */   sb        $v1, 0x41($sp)
  .L8006B244:
    /* 5BA44 8006B244 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5BA48 8006B248 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5BA4C 8006B24C 2800B4AF */  sw         $s4, 0x28($sp)
  .L8006B250:
    /* 5BA50 8006B250 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5BA54 8006B254 1800A28F */  lw         $v0, 0x18($sp)
    /* 5BA58 8006B258 04005226 */  addiu      $s2, $s2, 0x4
    /* 5BA5C 8006B25C 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5BA60 8006B260 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5BA64 8006B264 1400228E */  lw         $v0, 0x14($s1)
    /* 5BA68 8006B268 01001026 */  addiu      $s0, $s0, 0x1
    /* 5BA6C 8006B26C 4BCD010C */  jal        func_8007352C
    /* 5BA70 8006B270 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5BA74 8006B274 140022AE */  sw         $v0, 0x14($s1)
    /* 5BA78 8006B278 0600022A */  slti       $v0, $s0, 0x6
    /* 5BA7C 8006B27C D7FF4014 */  bnez       $v0, .L8006B1DC
    /* 5BA80 8006B280 00000000 */   nop
    /* 5BA84 8006B284 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5BA88 8006B288 21800000 */  addu       $s0, $zero, $zero
    /* 5BA8C 8006B28C 01001424 */  addiu      $s4, $zero, 0x1
    /* 5BA90 8006B290 21906002 */  addu       $s2, $s3, $zero
  .L8006B294:
    /* 5BA94 8006B294 1C00428E */  lw         $v0, 0x1C($s2)
    /* 5BA98 8006B298 5804838F */  lw         $v1, %gp_rel(D_800A3524)($gp)
    /* 5BA9C 8006B29C 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5BAA0 8006B2A0 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5BAA4 8006B2A4 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5BAA8 8006B2A8 2000628C */  lw         $v0, 0x20($v1)
    /* 5BAAC 8006B2AC 00000000 */  nop
    /* 5BAB0 8006B2B0 01004230 */  andi       $v0, $v0, 0x1
    /* 5BAB4 8006B2B4 16005014 */  bne        $v0, $s0, .L8006B310
    /* 5BAB8 8006B2B8 00000000 */   nop
    /* 5BABC 8006B2BC 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5BAC0 8006B2C0 00000000 */  nop
    /* 5BAC4 8006B2C4 001C4230 */  andi       $v0, $v0, 0x1C00
    /* 5BAC8 8006B2C8 12004014 */  bnez       $v0, .L8006B314
    /* 5BACC 8006B2CC 2800A0AF */   sw        $zero, 0x28($sp)
    /* 5BAD0 8006B2D0 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5BAD4 8006B2D4 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5BAD8 8006B2D8 4000B4A3 */  sb         $s4, 0x40($sp)
    /* 5BADC 8006B2DC 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5BAE0 8006B2E0 C0210400 */  sll        $a0, $a0, 7
    /* 5BAE4 8006B2E4 0C004284 */  lh         $v0, 0xC($v0)
    /* 5BAE8 8006B2E8 FF018424 */  addiu      $a0, $a0, 0x1FF
    /* 5BAEC 8006B2EC C8F7010C */  jal        math_Sin
    /* 5BAF0 8006B2F0 3000A2AF */   sw        $v0, 0x30($sp)
    /* 5BAF4 8006B2F4 40110200 */  sll        $v0, $v0, 5
    /* 5BAF8 8006B2F8 03130200 */  sra        $v0, $v0, 12
    /* 5BAFC 8006B2FC 80FF4224 */  addiu      $v0, $v0, -0x80
    /* 5BB00 8006B300 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5BB04 8006B304 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5BB08 8006B308 C5AC0108 */  j          .L8006B314
    /* 5BB0C 8006B30C 4100A2A3 */   sb        $v0, 0x41($sp)
  .L8006B310:
    /* 5BB10 8006B310 2800B4AF */  sw         $s4, 0x28($sp)
  .L8006B314:
    /* 5BB14 8006B314 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5BB18 8006B318 1800A28F */  lw         $v0, 0x18($sp)
    /* 5BB1C 8006B31C 04005226 */  addiu      $s2, $s2, 0x4
    /* 5BB20 8006B320 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5BB24 8006B324 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5BB28 8006B328 1400228E */  lw         $v0, 0x14($s1)
    /* 5BB2C 8006B32C 01001026 */  addiu      $s0, $s0, 0x1
    /* 5BB30 8006B330 4BCD010C */  jal        func_8007352C
    /* 5BB34 8006B334 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5BB38 8006B338 140022AE */  sw         $v0, 0x14($s1)
    /* 5BB3C 8006B33C 0200022A */  slti       $v0, $s0, 0x2
    /* 5BB40 8006B340 D4FF4014 */  bnez       $v0, .L8006B294
    /* 5BB44 8006B344 00000000 */   nop
    /* 5BB48 8006B348 21800000 */  addu       $s0, $zero, $zero
    /* 5BB4C 8006B34C 01001424 */  addiu      $s4, $zero, 0x1
    /* 5BB50 8006B350 21906002 */  addu       $s2, $s3, $zero
  .L8006B354:
    /* 5BB54 8006B354 2400428E */  lw         $v0, 0x24($s2)
    /* 5BB58 8006B358 5804838F */  lw         $v1, %gp_rel(D_800A3524)($gp)
    /* 5BB5C 8006B35C 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5BB60 8006B360 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5BB64 8006B364 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5BB68 8006B368 2000628C */  lw         $v0, 0x20($v1)
    /* 5BB6C 8006B36C 00000000 */  nop
    /* 5BB70 8006B370 42100200 */  srl        $v0, $v0, 1
    /* 5BB74 8006B374 01004230 */  andi       $v0, $v0, 0x1
    /* 5BB78 8006B378 16005014 */  bne        $v0, $s0, .L8006B3D4
    /* 5BB7C 8006B37C 00040224 */   addiu     $v0, $zero, 0x400
    /* 5BB80 8006B380 2C04838F */  lw         $v1, %gp_rel(D_800A34F8)($gp)
    /* 5BB84 8006B384 00000000 */  nop
    /* 5BB88 8006B388 001C6330 */  andi       $v1, $v1, 0x1C00
    /* 5BB8C 8006B38C 12006214 */  bne        $v1, $v0, .L8006B3D8
    /* 5BB90 8006B390 2800A0AF */   sw        $zero, 0x28($sp)
    /* 5BB94 8006B394 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5BB98 8006B398 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5BB9C 8006B39C 4000B4A3 */  sb         $s4, 0x40($sp)
    /* 5BBA0 8006B3A0 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5BBA4 8006B3A4 C0210400 */  sll        $a0, $a0, 7
    /* 5BBA8 8006B3A8 0C004284 */  lh         $v0, 0xC($v0)
    /* 5BBAC 8006B3AC FF018424 */  addiu      $a0, $a0, 0x1FF
    /* 5BBB0 8006B3B0 C8F7010C */  jal        math_Sin
    /* 5BBB4 8006B3B4 3000A2AF */   sw        $v0, 0x30($sp)
    /* 5BBB8 8006B3B8 40110200 */  sll        $v0, $v0, 5
    /* 5BBBC 8006B3BC 03130200 */  sra        $v0, $v0, 12
    /* 5BBC0 8006B3C0 80FF4224 */  addiu      $v0, $v0, -0x80
    /* 5BBC4 8006B3C4 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5BBC8 8006B3C8 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5BBCC 8006B3CC F6AC0108 */  j          .L8006B3D8
    /* 5BBD0 8006B3D0 4100A2A3 */   sb        $v0, 0x41($sp)
  .L8006B3D4:
    /* 5BBD4 8006B3D4 2800B4AF */  sw         $s4, 0x28($sp)
  .L8006B3D8:
    /* 5BBD8 8006B3D8 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5BBDC 8006B3DC 1800A28F */  lw         $v0, 0x18($sp)
    /* 5BBE0 8006B3E0 04005226 */  addiu      $s2, $s2, 0x4
    /* 5BBE4 8006B3E4 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5BBE8 8006B3E8 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5BBEC 8006B3EC 1400228E */  lw         $v0, 0x14($s1)
    /* 5BBF0 8006B3F0 01001026 */  addiu      $s0, $s0, 0x1
    /* 5BBF4 8006B3F4 4BCD010C */  jal        func_8007352C
    /* 5BBF8 8006B3F8 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5BBFC 8006B3FC 140022AE */  sw         $v0, 0x14($s1)
    /* 5BC00 8006B400 0200022A */  slti       $v0, $s0, 0x2
    /* 5BC04 8006B404 D3FF4014 */  bnez       $v0, .L8006B354
    /* 5BC08 8006B408 00000000 */   nop
    /* 5BC0C 8006B40C 21800000 */  addu       $s0, $zero, $zero
    /* 5BC10 8006B410 01001424 */  addiu      $s4, $zero, 0x1
    /* 5BC14 8006B414 21906002 */  addu       $s2, $s3, $zero
  .L8006B418:
    /* 5BC18 8006B418 2C00428E */  lw         $v0, 0x2C($s2)
    /* 5BC1C 8006B41C 5804838F */  lw         $v1, %gp_rel(D_800A3524)($gp)
    /* 5BC20 8006B420 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5BC24 8006B424 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5BC28 8006B428 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5BC2C 8006B42C 2000628C */  lw         $v0, 0x20($v1)
    /* 5BC30 8006B430 00000000 */  nop
    /* 5BC34 8006B434 82100200 */  srl        $v0, $v0, 2
    /* 5BC38 8006B438 01004230 */  andi       $v0, $v0, 0x1
    /* 5BC3C 8006B43C 16005014 */  bne        $v0, $s0, .L8006B498
    /* 5BC40 8006B440 00080224 */   addiu     $v0, $zero, 0x800
    /* 5BC44 8006B444 2C04838F */  lw         $v1, %gp_rel(D_800A34F8)($gp)
    /* 5BC48 8006B448 00000000 */  nop
    /* 5BC4C 8006B44C 001C6330 */  andi       $v1, $v1, 0x1C00
    /* 5BC50 8006B450 12006214 */  bne        $v1, $v0, .L8006B49C
    /* 5BC54 8006B454 2800A0AF */   sw        $zero, 0x28($sp)
    /* 5BC58 8006B458 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5BC5C 8006B45C 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5BC60 8006B460 4000B4A3 */  sb         $s4, 0x40($sp)
    /* 5BC64 8006B464 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5BC68 8006B468 C0210400 */  sll        $a0, $a0, 7
    /* 5BC6C 8006B46C 0C004284 */  lh         $v0, 0xC($v0)
    /* 5BC70 8006B470 FF018424 */  addiu      $a0, $a0, 0x1FF
    /* 5BC74 8006B474 C8F7010C */  jal        math_Sin
    /* 5BC78 8006B478 3000A2AF */   sw        $v0, 0x30($sp)
    /* 5BC7C 8006B47C 40110200 */  sll        $v0, $v0, 5
    /* 5BC80 8006B480 03130200 */  sra        $v0, $v0, 12
    /* 5BC84 8006B484 80FF4224 */  addiu      $v0, $v0, -0x80
    /* 5BC88 8006B488 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5BC8C 8006B48C 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5BC90 8006B490 27AD0108 */  j          .L8006B49C
    /* 5BC94 8006B494 4100A2A3 */   sb        $v0, 0x41($sp)
  .L8006B498:
    /* 5BC98 8006B498 2800B4AF */  sw         $s4, 0x28($sp)
  .L8006B49C:
    /* 5BC9C 8006B49C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5BCA0 8006B4A0 1800A28F */  lw         $v0, 0x18($sp)
    /* 5BCA4 8006B4A4 04005226 */  addiu      $s2, $s2, 0x4
    /* 5BCA8 8006B4A8 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5BCAC 8006B4AC 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5BCB0 8006B4B0 1400228E */  lw         $v0, 0x14($s1)
    /* 5BCB4 8006B4B4 01001026 */  addiu      $s0, $s0, 0x1
    /* 5BCB8 8006B4B8 4BCD010C */  jal        func_8007352C
    /* 5BCBC 8006B4BC 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5BCC0 8006B4C0 140022AE */  sw         $v0, 0x14($s1)
    /* 5BCC4 8006B4C4 0200022A */  slti       $v0, $s0, 0x2
    /* 5BCC8 8006B4C8 D3FF4014 */  bnez       $v0, .L8006B418
    /* 5BCCC 8006B4CC 21280000 */   addu      $a1, $zero, $zero
    /* 5BCD0 8006B4D0 0400228E */  lw         $v0, 0x4($s1)
    /* 5BCD4 8006B4D4 00000000 */  nop
    /* 5BCD8 8006B4D8 2800538C */  lw         $s3, 0x28($v0)
    /* 5BCDC 8006B4DC 00000000 */  nop
    /* 5BCE0 8006B4E0 0400648E */  lw         $a0, 0x4($s3)
    /* 5BCE4 8006B4E4 20B9010C */  jal        func_8006E480
    /* 5BCE8 8006B4E8 1800A4AF */   sw        $a0, 0x18($sp)
    /* 5BCEC 8006B4EC 01000524 */  addiu      $a1, $zero, 0x1
    /* 5BCF0 8006B4F0 21300000 */  addu       $a2, $zero, $zero
    /* 5BCF4 8006B4F4 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5BCF8 8006B4F8 1C00248E */  lw         $a0, 0x1C($s1)
    /* 5BCFC 8006B4FC 92F0010C */  jal        initTexPage
    /* 5BD00 8006B500 21384000 */   addu      $a3, $v0, $zero
    /* 5BD04 8006B504 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5BD08 8006B508 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5BD0C 8006B50C 1C00258E */  lw         $a1, 0x1C($s1)
    /* 5BD10 8006B510 2DEA010C */  jal        ot_Link
    /* 5BD14 8006B514 28008424 */   addiu     $a0, $a0, 0x28
    /* 5BD18 8006B518 21202002 */  addu       $a0, $s1, $zero
    /* 5BD1C 8006B51C 4800A527 */  addiu      $a1, $sp, 0x48
    /* 5BD20 8006B520 1C00828C */  lw         $v0, 0x1C($a0)
    /* 5BD24 8006B524 11000624 */  addiu      $a2, $zero, 0x11
    /* 5BD28 8006B528 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5BD2C 8006B52C 1C0082AC */  sw         $v0, 0x1C($a0)
    /* 5BD30 8006B530 AF000224 */  addiu      $v0, $zero, 0xAF
    /* 5BD34 8006B534 4C00A2A7 */  sh         $v0, 0x4C($sp)
    /* 5BD38 8006B538 E8000224 */  addiu      $v0, $zero, 0xE8
    /* 5BD3C 8006B53C 4800A2A7 */  sh         $v0, 0x48($sp)
    /* 5BD40 8006B540 25000224 */  addiu      $v0, $zero, 0x25
    /* 5BD44 8006B544 4A00A2A7 */  sh         $v0, 0x4A($sp)
    /* 5BD48 8006B548 01000224 */  addiu      $v0, $zero, 0x1
    /* 5BD4C 8006B54C 26A6010C */  jal        func_80069898
    /* 5BD50 8006B550 4E00A2A7 */   sh        $v0, 0x4E($sp)
    /* 5BD54 8006B554 6400BF8F */  lw         $ra, 0x64($sp)
    /* 5BD58 8006B558 6000B48F */  lw         $s4, 0x60($sp)
    /* 5BD5C 8006B55C 5C00B38F */  lw         $s3, 0x5C($sp)
    /* 5BD60 8006B560 5800B28F */  lw         $s2, 0x58($sp)
    /* 5BD64 8006B564 5400B18F */  lw         $s1, 0x54($sp)
    /* 5BD68 8006B568 5000B08F */  lw         $s0, 0x50($sp)
    /* 5BD6C 8006B56C 6800BD27 */  addiu      $sp, $sp, 0x68
    /* 5BD70 8006B570 0800E003 */  jr         $ra
    /* 5BD74 8006B574 00000000 */   nop
endlabel func_8006B120
