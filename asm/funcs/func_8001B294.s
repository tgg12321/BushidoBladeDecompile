glabel func_8001B294
    /* BA94 8001B294 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* BA98 8001B298 1400B1AF */  sw         $s1, 0x14($sp)
    /* BA9C 8001B29C 21888000 */  addu       $s1, $a0, $zero
    /* BAA0 8001B2A0 1000B0AF */  sw         $s0, 0x10($sp)
    /* BAA4 8001B2A4 2180A000 */  addu       $s0, $a1, $zero
    /* BAA8 8001B2A8 64000224 */  addiu      $v0, $zero, 0x64
    /* BAAC 8001B2AC 1800BFAF */  sw         $ra, 0x18($sp)
    /* BAB0 8001B2B0 0A80013C */  lui        $at, %hi(D_800A36FA)
    /* BAB4 8001B2B4 FA3620A0 */  sb         $zero, %lo(D_800A36FA)($at)
    /* BAB8 8001B2B8 0F80013C */  lui        $at, %hi(D_800F6638)
    /* BABC 8001B2BC 386622A4 */  sh         $v0, %lo(D_800F6638)($at)
    /* BAC0 8001B2C0 0F80013C */  lui        $at, %hi(D_800F663A)
    /* BAC4 8001B2C4 3A6620A4 */  sh         $zero, %lo(D_800F663A)($at)
    /* BAC8 8001B2C8 0F80013C */  lui        $at, %hi(D_800F663C)
    /* BACC 8001B2CC 3C6622A4 */  sh         $v0, %lo(D_800F663C)($at)
    /* BAD0 8001B2D0 0F80013C */  lui        $at, %hi(D_800F6640)
    /* BAD4 8001B2D4 406622A4 */  sh         $v0, %lo(D_800F6640)($at)
    /* BAD8 8001B2D8 0F80013C */  lui        $at, %hi(D_800F6642)
    /* BADC 8001B2DC 426620A4 */  sh         $zero, %lo(D_800F6642)($at)
    /* BAE0 8001B2E0 0F80013C */  lui        $at, %hi(D_800F6644)
    /* BAE4 8001B2E4 446622A4 */  sh         $v0, %lo(D_800F6644)($at)
    /* BAE8 8001B2E8 79FC000C */  jal        func_8003F1E4
    /* BAEC 8001B2EC 21200000 */   addu      $a0, $zero, $zero
    /* BAF0 8001B2F0 F400228E */  lw         $v0, 0xF4($s1)
    /* BAF4 8001B2F4 F400038E */  lw         $v1, 0xF4($s0)
    /* BAF8 8001B2F8 00000000 */  nop
    /* BAFC 8001B2FC 21104300 */  addu       $v0, $v0, $v1
    /* BB00 8001B300 C21F0200 */  srl        $v1, $v0, 31
    /* BB04 8001B304 21104300 */  addu       $v0, $v0, $v1
    /* BB08 8001B308 43100200 */  sra        $v0, $v0, 1
    /* BB0C 8001B30C 0F80013C */  lui        $at, %hi(D_800F6608)
    /* BB10 8001B310 086622AC */  sw         $v0, %lo(D_800F6608)($at)
    /* BB14 8001B314 F800228E */  lw         $v0, 0xF8($s1)
    /* BB18 8001B318 F800038E */  lw         $v1, 0xF8($s0)
    /* BB1C 8001B31C 00000000 */  nop
    /* BB20 8001B320 21104300 */  addu       $v0, $v0, $v1
    /* BB24 8001B324 C21F0200 */  srl        $v1, $v0, 31
    /* BB28 8001B328 21104300 */  addu       $v0, $v0, $v1
    /* BB2C 8001B32C 43100200 */  sra        $v0, $v0, 1
    /* BB30 8001B330 0F80013C */  lui        $at, %hi(D_800F660C)
    /* BB34 8001B334 0C6622AC */  sw         $v0, %lo(D_800F660C)($at)
    /* BB38 8001B338 FC00228E */  lw         $v0, 0xFC($s1)
    /* BB3C 8001B33C FC00038E */  lw         $v1, 0xFC($s0)
    /* BB40 8001B340 0F80013C */  lui        $at, %hi(D_800F6618)
    /* BB44 8001B344 186620A4 */  sh         $zero, %lo(D_800F6618)($at)
    /* BB48 8001B348 21104300 */  addu       $v0, $v0, $v1
    /* BB4C 8001B34C C21F0200 */  srl        $v1, $v0, 31
    /* BB50 8001B350 21104300 */  addu       $v0, $v0, $v1
    /* BB54 8001B354 43100200 */  sra        $v0, $v0, 1
    /* BB58 8001B358 0F80013C */  lui        $at, %hi(D_800F6610)
    /* BB5C 8001B35C 106622AC */  sw         $v0, %lo(D_800F6610)($at)
    /* BB60 8001B360 F400038E */  lw         $v1, 0xF4($s0)
    /* BB64 8001B364 F400248E */  lw         $a0, 0xF4($s1)
    /* BB68 8001B368 FC00028E */  lw         $v0, 0xFC($s0)
    /* BB6C 8001B36C FC00258E */  lw         $a1, 0xFC($s1)
    /* BB70 8001B370 23206400 */  subu       $a0, $v1, $a0
    /* BB74 8001B374 57FF010C */  jal        func_8007FD5C
    /* BB78 8001B378 23284500 */   subu      $a1, $v0, $a1
    /* BB7C 8001B37C 00040324 */  addiu      $v1, $zero, 0x400
    /* BB80 8001B380 23186200 */  subu       $v1, $v1, $v0
    /* BB84 8001B384 88130224 */  addiu      $v0, $zero, 0x1388
    /* BB88 8001B388 0F80013C */  lui        $at, %hi(D_800F661A)
    /* BB8C 8001B38C 1A6623A4 */  sh         $v1, %lo(D_800F661A)($at)
    /* BB90 8001B390 0F80013C */  lui        $at, %hi(D_800F661C)
    /* BB94 8001B394 1C6620A4 */  sh         $zero, %lo(D_800F661C)($at)
    /* BB98 8001B398 0F80013C */  lui        $at, %hi(D_800F6620)
    /* BB9C 8001B39C 206622AC */  sw         $v0, %lo(D_800F6620)($at)
    /* BBA0 8001B3A0 0F80013C */  lui        $at, %hi(D_800F6626)
    /* BBA4 8001B3A4 266620A0 */  sb         $zero, %lo(D_800F6626)($at)
    /* BBA8 8001B3A8 1800BF8F */  lw         $ra, 0x18($sp)
    /* BBAC 8001B3AC 1400B18F */  lw         $s1, 0x14($sp)
    /* BBB0 8001B3B0 1000B08F */  lw         $s0, 0x10($sp)
    /* BBB4 8001B3B4 2000BD27 */  addiu      $sp, $sp, 0x20
    /* BBB8 8001B3B8 0800E003 */  jr         $ra
    /* BBBC 8001B3BC 00000000 */   nop
endlabel func_8001B294
