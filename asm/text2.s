.include "macro.inc"

.set noat
.set noreorder

.section .text, "ax"

nonmatching func_8007B244, 0x5C

glabel func_8007B244
    /* 6BA44 8007B244 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6BA48 8007B248 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6BA4C 8007B24C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6BA50 8007B250 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BA54 8007B254 21808000 */  addu       $s0, $a0, $zero
    /* 6BA58 8007B258 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6BA5C 8007B25C 07004014 */  bnez       $v0, .L8007B27C
    /* 6BA60 8007B260 1400BFAF */   sw        $ra, 0x14($sp)
    /* 6BA64 8007B264 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6BA68 8007B268 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6BA6C 8007B26C 0180043C */  lui        $a0, %hi(D_80015EE8)
    /* 6BA70 8007B270 E85E8424 */  addiu      $a0, $a0, %lo(D_80015EE8)
    /* 6BA74 8007B274 09F84000 */  jalr       $v0
    /* 6BA78 8007B278 21280002 */   addu      $a1, $s0, $zero
  .L8007B27C:
    /* 6BA7C 8007B27C 0A80023C */  lui        $v0, %hi(D_8009BE80)
    /* 6BA80 8007B280 80BE428C */  lw         $v0, %lo(D_8009BE80)($v0)
    /* 6BA84 8007B284 0A80013C */  lui        $at, %hi(D_8009BE80)
    /* 6BA88 8007B288 80BE30AC */  sw         $s0, %lo(D_8009BE80)($at)
    /* 6BA8C 8007B28C 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6BA90 8007B290 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BA94 8007B294 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6BA98 8007B298 0800E003 */  jr         $ra
    /* 6BA9C 8007B29C 00000000 */   nop
endlabel func_8007B244

nonmatching gpu_SetDispMask, 0x9C

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
    /* 6BAF4 8007B2F4 B9F7010C */  jal        func_8007DEE4
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

nonmatching gpu_DrawSync, 0x6C

glabel gpu_DrawSync
    /* 6BB3C 8007B33C 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6BB40 8007B340 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6BB44 8007B344 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6BB48 8007B348 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BB4C 8007B34C 21808000 */  addu       $s0, $a0, $zero
    /* 6BB50 8007B350 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6BB54 8007B354 08004014 */  bnez       $v0, .L8007B378
    /* 6BB58 8007B358 1400BFAF */   sw        $ra, 0x14($sp)
    /* 6BB5C 8007B35C 0180043C */  lui        $a0, %hi(D_80015F18)
    /* 6BB60 8007B360 185F8424 */  addiu      $a0, $a0, %lo(D_80015F18)
    /* 6BB64 8007B364 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6BB68 8007B368 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6BB6C 8007B36C 00000000 */  nop
    /* 6BB70 8007B370 09F84000 */  jalr       $v0
    /* 6BB74 8007B374 21280002 */   addu      $a1, $s0, $zero
  .L8007B378:
    /* 6BB78 8007B378 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6BB7C 8007B37C 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6BB80 8007B380 00000000 */  nop
    /* 6BB84 8007B384 3C00428C */  lw         $v0, 0x3C($v0)
    /* 6BB88 8007B388 00000000 */  nop
    /* 6BB8C 8007B38C 09F84000 */  jalr       $v0
    /* 6BB90 8007B390 21200002 */   addu      $a0, $s0, $zero
    /* 6BB94 8007B394 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6BB98 8007B398 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BB9C 8007B39C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6BBA0 8007B3A0 0800E003 */  jr         $ra
    /* 6BBA4 8007B3A4 00000000 */   nop
endlabel gpu_DrawSync

nonmatching func_8007B3A8, 0x128

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

nonmatching func_8007B4D0, 0x94

glabel func_8007B4D0
    /* 6BCD0 8007B4D0 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6BCD4 8007B4D4 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6BCD8 8007B4D8 21988000 */  addu       $s3, $a0, $zero
    /* 6BCDC 8007B4DC 0180043C */  lui        $a0, %hi(D_80015F50)
    /* 6BCE0 8007B4E0 505F8424 */  addiu      $a0, $a0, %lo(D_80015F50)
    /* 6BCE4 8007B4E4 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6BCE8 8007B4E8 2190A000 */  addu       $s2, $a1, $zero
    /* 6BCEC 8007B4EC 21286002 */  addu       $a1, $s3, $zero
    /* 6BCF0 8007B4F0 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BCF4 8007B4F4 2188C000 */  addu       $s1, $a2, $zero
    /* 6BCF8 8007B4F8 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BCFC 8007B4FC 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6BD00 8007B500 EAEC010C */  jal        func_8007B3A8
    /* 6BD04 8007B504 2180E000 */   addu      $s0, $a3, $zero
    /* 6BD08 8007B508 21286002 */  addu       $a1, $s3, $zero
    /* 6BD0C 8007B50C FF001032 */  andi       $s0, $s0, 0xFF
    /* 6BD10 8007B510 00841000 */  sll        $s0, $s0, 16
    /* 6BD14 8007B514 FF003132 */  andi       $s1, $s1, 0xFF
    /* 6BD18 8007B518 008A1100 */  sll        $s1, $s1, 8
    /* 6BD1C 8007B51C 25801102 */  or         $s0, $s0, $s1
    /* 6BD20 8007B520 FF005232 */  andi       $s2, $s2, 0xFF
    /* 6BD24 8007B524 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6BD28 8007B528 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6BD2C 8007B52C 08000624 */  addiu      $a2, $zero, 0x8
    /* 6BD30 8007B530 0C00448C */  lw         $a0, 0xC($v0)
    /* 6BD34 8007B534 0800428C */  lw         $v0, 0x8($v0)
    /* 6BD38 8007B538 00000000 */  nop
    /* 6BD3C 8007B53C 09F84000 */  jalr       $v0
    /* 6BD40 8007B540 25381202 */   or        $a3, $s0, $s2
    /* 6BD44 8007B544 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6BD48 8007B548 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6BD4C 8007B54C 1800B28F */  lw         $s2, 0x18($sp)
    /* 6BD50 8007B550 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BD54 8007B554 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BD58 8007B558 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6BD5C 8007B55C 0800E003 */  jr         $ra
    /* 6BD60 8007B560 00000000 */   nop
endlabel func_8007B4D0

nonmatching func_8007B564, 0x9C

glabel func_8007B564
    /* 6BD64 8007B564 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6BD68 8007B568 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6BD6C 8007B56C 21988000 */  addu       $s3, $a0, $zero
    /* 6BD70 8007B570 0180043C */  lui        $a0, %hi(D_80015F50)
    /* 6BD74 8007B574 505F8424 */  addiu      $a0, $a0, %lo(D_80015F50)
    /* 6BD78 8007B578 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6BD7C 8007B57C 2190A000 */  addu       $s2, $a1, $zero
    /* 6BD80 8007B580 21286002 */  addu       $a1, $s3, $zero
    /* 6BD84 8007B584 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BD88 8007B588 2180C000 */  addu       $s0, $a2, $zero
    /* 6BD8C 8007B58C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BD90 8007B590 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6BD94 8007B594 EAEC010C */  jal        func_8007B3A8
    /* 6BD98 8007B598 2188E000 */   addu      $s1, $a3, $zero
    /* 6BD9C 8007B59C 21286002 */  addu       $a1, $s3, $zero
    /* 6BDA0 8007B5A0 FF003132 */  andi       $s1, $s1, 0xFF
    /* 6BDA4 8007B5A4 008C1100 */  sll        $s1, $s1, 16
    /* 6BDA8 8007B5A8 FF001032 */  andi       $s0, $s0, 0xFF
    /* 6BDAC 8007B5AC 00821000 */  sll        $s0, $s0, 8
    /* 6BDB0 8007B5B0 0080023C */  lui        $v0, (0x80000000 >> 16)
    /* 6BDB4 8007B5B4 25800202 */  or         $s0, $s0, $v0
    /* 6BDB8 8007B5B8 25883002 */  or         $s1, $s1, $s0
    /* 6BDBC 8007B5BC FF005232 */  andi       $s2, $s2, 0xFF
    /* 6BDC0 8007B5C0 0A80033C */  lui        $v1, %hi(D_8009BE6C)
    /* 6BDC4 8007B5C4 6CBE638C */  lw         $v1, %lo(D_8009BE6C)($v1)
    /* 6BDC8 8007B5C8 08000624 */  addiu      $a2, $zero, 0x8
    /* 6BDCC 8007B5CC 0C00648C */  lw         $a0, 0xC($v1)
    /* 6BDD0 8007B5D0 0800628C */  lw         $v0, 0x8($v1)
    /* 6BDD4 8007B5D4 00000000 */  nop
    /* 6BDD8 8007B5D8 09F84000 */  jalr       $v0
    /* 6BDDC 8007B5DC 25383202 */   or        $a3, $s1, $s2
    /* 6BDE0 8007B5E0 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6BDE4 8007B5E4 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6BDE8 8007B5E8 1800B28F */  lw         $s2, 0x18($sp)
    /* 6BDEC 8007B5EC 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BDF0 8007B5F0 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BDF4 8007B5F4 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6BDF8 8007B5F8 0800E003 */  jr         $ra
    /* 6BDFC 8007B5FC 00000000 */   nop
endlabel func_8007B564

nonmatching gpu_LoadImage, 0x64

glabel gpu_LoadImage
    /* 6BE00 8007B600 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6BE04 8007B604 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BE08 8007B608 21808000 */  addu       $s0, $a0, $zero
    /* 6BE0C 8007B60C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BE10 8007B610 2188A000 */  addu       $s1, $a1, $zero
    /* 6BE14 8007B614 0180043C */  lui        $a0, %hi(D_80015F5C)
    /* 6BE18 8007B618 5C5F8424 */  addiu      $a0, $a0, %lo(D_80015F5C)
    /* 6BE1C 8007B61C 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6BE20 8007B620 EAEC010C */  jal        func_8007B3A8
    /* 6BE24 8007B624 21280002 */   addu      $a1, $s0, $zero
    /* 6BE28 8007B628 21280002 */  addu       $a1, $s0, $zero
    /* 6BE2C 8007B62C 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6BE30 8007B630 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6BE34 8007B634 08000624 */  addiu      $a2, $zero, 0x8
    /* 6BE38 8007B638 2000448C */  lw         $a0, 0x20($v0)
    /* 6BE3C 8007B63C 0800428C */  lw         $v0, 0x8($v0)
    /* 6BE40 8007B640 00000000 */  nop
    /* 6BE44 8007B644 09F84000 */  jalr       $v0
    /* 6BE48 8007B648 21382002 */   addu      $a3, $s1, $zero
    /* 6BE4C 8007B64C 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6BE50 8007B650 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BE54 8007B654 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BE58 8007B658 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6BE5C 8007B65C 0800E003 */  jr         $ra
    /* 6BE60 8007B660 00000000 */   nop
endlabel gpu_LoadImage

nonmatching gpu_StoreImage, 0x64

glabel gpu_StoreImage
    /* 6BE64 8007B664 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6BE68 8007B668 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BE6C 8007B66C 21808000 */  addu       $s0, $a0, $zero
    /* 6BE70 8007B670 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BE74 8007B674 2188A000 */  addu       $s1, $a1, $zero
    /* 6BE78 8007B678 0180043C */  lui        $a0, %hi(D_80015F68)
    /* 6BE7C 8007B67C 685F8424 */  addiu      $a0, $a0, %lo(D_80015F68)
    /* 6BE80 8007B680 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6BE84 8007B684 EAEC010C */  jal        func_8007B3A8
    /* 6BE88 8007B688 21280002 */   addu      $a1, $s0, $zero
    /* 6BE8C 8007B68C 21280002 */  addu       $a1, $s0, $zero
    /* 6BE90 8007B690 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6BE94 8007B694 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6BE98 8007B698 08000624 */  addiu      $a2, $zero, 0x8
    /* 6BE9C 8007B69C 1C00448C */  lw         $a0, 0x1C($v0)
    /* 6BEA0 8007B6A0 0800428C */  lw         $v0, 0x8($v0)
    /* 6BEA4 8007B6A4 00000000 */  nop
    /* 6BEA8 8007B6A8 09F84000 */  jalr       $v0
    /* 6BEAC 8007B6AC 21382002 */   addu      $a3, $s1, $zero
    /* 6BEB0 8007B6B0 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6BEB4 8007B6B4 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BEB8 8007B6B8 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BEBC 8007B6BC 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6BEC0 8007B6C0 0800E003 */  jr         $ra
    /* 6BEC4 8007B6C4 00000000 */   nop
endlabel gpu_StoreImage

nonmatching func_8007B6C8, 0xC4

glabel func_8007B6C8
    /* 6BEC8 8007B6C8 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6BECC 8007B6CC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BED0 8007B6D0 21808000 */  addu       $s0, $a0, $zero
    /* 6BED4 8007B6D4 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6BED8 8007B6D8 2190A000 */  addu       $s2, $a1, $zero
    /* 6BEDC 8007B6DC 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BEE0 8007B6E0 2188C000 */  addu       $s1, $a2, $zero
    /* 6BEE4 8007B6E4 0180043C */  lui        $a0, %hi(D_80015F74)
    /* 6BEE8 8007B6E8 745F8424 */  addiu      $a0, $a0, %lo(D_80015F74)
    /* 6BEEC 8007B6EC 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 6BEF0 8007B6F0 EAEC010C */  jal        func_8007B3A8
    /* 6BEF4 8007B6F4 21280002 */   addu      $a1, $s0, $zero
    /* 6BEF8 8007B6F8 04000286 */  lh         $v0, 0x4($s0)
    /* 6BEFC 8007B6FC 00000000 */  nop
    /* 6BF00 8007B700 1B004010 */  beqz       $v0, .L8007B770
    /* 6BF04 8007B704 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 6BF08 8007B708 06000286 */  lh         $v0, 0x6($s0)
    /* 6BF0C 8007B70C 00000000 */  nop
    /* 6BF10 8007B710 03004014 */  bnez       $v0, .L8007B720
    /* 6BF14 8007B714 00141100 */   sll       $v0, $s1, 16
    /* 6BF18 8007B718 DCED0108 */  j          .L8007B770
    /* 6BF1C 8007B71C FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8007B720:
    /* 6BF20 8007B720 FFFF4332 */  andi       $v1, $s2, 0xFFFF
    /* 6BF24 8007B724 25104300 */  or         $v0, $v0, $v1
    /* 6BF28 8007B728 0A80053C */  lui        $a1, %hi(D_8009BF24)
    /* 6BF2C 8007B72C 24BFA524 */  addiu      $a1, $a1, %lo(D_8009BF24)
    /* 6BF30 8007B730 0000048E */  lw         $a0, 0x0($s0)
    /* 6BF34 8007B734 0A80033C */  lui        $v1, %hi(D_8009BE6C)
    /* 6BF38 8007B738 6CBE638C */  lw         $v1, %lo(D_8009BE6C)($v1)
    /* 6BF3C 8007B73C 14000624 */  addiu      $a2, $zero, 0x14
    /* 6BF40 8007B740 0A80013C */  lui        $at, %hi(D_8009BF28)
    /* 6BF44 8007B744 28BF22AC */  sw         $v0, %lo(D_8009BF28)($at)
    /* 6BF48 8007B748 0000A4AC */  sw         $a0, 0x0($a1)
    /* 6BF4C 8007B74C 0400028E */  lw         $v0, 0x4($s0)
    /* 6BF50 8007B750 21380000 */  addu       $a3, $zero, $zero
    /* 6BF54 8007B754 0A80013C */  lui        $at, %hi(D_8009BF2C)
    /* 6BF58 8007B758 2CBF22AC */  sw         $v0, %lo(D_8009BF2C)($at)
    /* 6BF5C 8007B75C 1800648C */  lw         $a0, 0x18($v1)
    /* 6BF60 8007B760 0800628C */  lw         $v0, 0x8($v1)
    /* 6BF64 8007B764 00000000 */  nop
    /* 6BF68 8007B768 09F84000 */  jalr       $v0
    /* 6BF6C 8007B76C F8FFA524 */   addiu     $a1, $a1, -0x8
  .L8007B770:
    /* 6BF70 8007B770 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 6BF74 8007B774 1800B28F */  lw         $s2, 0x18($sp)
    /* 6BF78 8007B778 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BF7C 8007B77C 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BF80 8007B780 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6BF84 8007B784 0800E003 */  jr         $ra
    /* 6BF88 8007B788 00000000 */   nop
endlabel func_8007B6C8

nonmatching gpu_ClearOTag, 0xB8

glabel gpu_ClearOTag
    /* 6BF8C 8007B78C 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6BF90 8007B790 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6BF94 8007B794 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6BF98 8007B798 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BF9C 8007B79C 21808000 */  addu       $s0, $a0, $zero
    /* 6BFA0 8007B7A0 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BFA4 8007B7A4 2188A000 */  addu       $s1, $a1, $zero
    /* 6BFA8 8007B7A8 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6BFAC 8007B7AC 08004014 */  bnez       $v0, .L8007B7D0
    /* 6BFB0 8007B7B0 1800BFAF */   sw        $ra, 0x18($sp)
    /* 6BFB4 8007B7B4 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6BFB8 8007B7B8 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6BFBC 8007B7BC 0180043C */  lui        $a0, %hi(D_80015F80)
    /* 6BFC0 8007B7C0 805F8424 */  addiu      $a0, $a0, %lo(D_80015F80)
    /* 6BFC4 8007B7C4 21280002 */  addu       $a1, $s0, $zero
    /* 6BFC8 8007B7C8 09F84000 */  jalr       $v0
    /* 6BFCC 8007B7CC 21302002 */   addu      $a2, $s1, $zero
  .L8007B7D0:
    /* 6BFD0 8007B7D0 FFFF3126 */  addiu      $s1, $s1, -0x1
    /* 6BFD4 8007B7D4 0F002012 */  beqz       $s1, .L8007B814
    /* 6BFD8 8007B7D8 FF00033C */   lui       $v1, (0xFFFFFF >> 16)
    /* 6BFDC 8007B7DC FF00053C */  lui        $a1, (0xFFFFFF >> 16)
    /* 6BFE0 8007B7E0 FFFFA534 */  ori        $a1, $a1, (0xFFFFFF & 0xFFFF)
    /* 6BFE4 8007B7E4 00FF063C */  lui        $a2, (0xFF000000 >> 16)
  .L8007B7E8:
    /* 6BFE8 8007B7E8 FFFF3126 */  addiu      $s1, $s1, -0x1
    /* 6BFEC 8007B7EC 04000426 */  addiu      $a0, $s0, 0x4
    /* 6BFF0 8007B7F0 030000A2 */  sb         $zero, 0x3($s0)
    /* 6BFF4 8007B7F4 0000028E */  lw         $v0, 0x0($s0)
    /* 6BFF8 8007B7F8 24188500 */  and        $v1, $a0, $a1
    /* 6BFFC 8007B7FC 24104600 */  and        $v0, $v0, $a2
    /* 6C000 8007B800 25104300 */  or         $v0, $v0, $v1
    /* 6C004 8007B804 000002AE */  sw         $v0, 0x0($s0)
    /* 6C008 8007B808 F7FF2016 */  bnez       $s1, .L8007B7E8
    /* 6C00C 8007B80C 21808000 */   addu      $s0, $a0, $zero
    /* 6C010 8007B810 FF00033C */  lui        $v1, (0xFFFFFF >> 16)
  .L8007B814:
    /* 6C014 8007B814 FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 6C018 8007B818 0A80023C */  lui        $v0, %hi(D_8009BF30)
    /* 6C01C 8007B81C 30BF4224 */  addiu      $v0, $v0, %lo(D_8009BF30)
    /* 6C020 8007B820 24104300 */  and        $v0, $v0, $v1
    /* 6C024 8007B824 000002AE */  sw         $v0, 0x0($s0)
    /* 6C028 8007B828 21100002 */  addu       $v0, $s0, $zero
    /* 6C02C 8007B82C 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6C030 8007B830 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C034 8007B834 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C038 8007B838 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6C03C 8007B83C 0800E003 */  jr         $ra
    /* 6C040 8007B840 00000000 */   nop
endlabel gpu_ClearOTag

nonmatching func_8007B844, 0x98

glabel func_8007B844
    /* 6C044 8007B844 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6C048 8007B848 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6C04C 8007B84C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6C050 8007B850 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C054 8007B854 21808000 */  addu       $s0, $a0, $zero
    /* 6C058 8007B858 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6C05C 8007B85C 2188A000 */  addu       $s1, $a1, $zero
    /* 6C060 8007B860 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C064 8007B864 09004014 */  bnez       $v0, .L8007B88C
    /* 6C068 8007B868 1800BFAF */   sw        $ra, 0x18($sp)
    /* 6C06C 8007B86C 0180043C */  lui        $a0, %hi(D_80015F98)
    /* 6C070 8007B870 985F8424 */  addiu      $a0, $a0, %lo(D_80015F98)
    /* 6C074 8007B874 21280002 */  addu       $a1, $s0, $zero
    /* 6C078 8007B878 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6C07C 8007B87C 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6C080 8007B880 00000000 */  nop
    /* 6C084 8007B884 09F84000 */  jalr       $v0
    /* 6C088 8007B888 21302002 */   addu      $a2, $s1, $zero
  .L8007B88C:
    /* 6C08C 8007B88C 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C090 8007B890 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C094 8007B894 21200002 */  addu       $a0, $s0, $zero
    /* 6C098 8007B898 2C00428C */  lw         $v0, 0x2C($v0)
    /* 6C09C 8007B89C 00000000 */  nop
    /* 6C0A0 8007B8A0 09F84000 */  jalr       $v0
    /* 6C0A4 8007B8A4 21282002 */   addu      $a1, $s1, $zero
    /* 6C0A8 8007B8A8 FF00043C */  lui        $a0, (0xFFFFFF >> 16)
    /* 6C0AC 8007B8AC FFFF8434 */  ori        $a0, $a0, (0xFFFFFF & 0xFFFF)
    /* 6C0B0 8007B8B0 21100002 */  addu       $v0, $s0, $zero
    /* 6C0B4 8007B8B4 0A80033C */  lui        $v1, %hi(D_8009BF30)
    /* 6C0B8 8007B8B8 30BF6324 */  addiu      $v1, $v1, %lo(D_8009BF30)
    /* 6C0BC 8007B8BC 24186400 */  and        $v1, $v1, $a0
    /* 6C0C0 8007B8C0 000043AC */  sw         $v1, 0x0($v0)
    /* 6C0C4 8007B8C4 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6C0C8 8007B8C8 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C0CC 8007B8CC 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C0D0 8007B8D0 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6C0D4 8007B8D4 0800E003 */  jr         $ra
    /* 6C0D8 8007B8D8 00000000 */   nop
endlabel func_8007B844

nonmatching gpu_SendPacket, 0x60

glabel gpu_SendPacket
    /* 6C0DC 8007B8DC E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6C0E0 8007B8E0 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C0E4 8007B8E4 21808000 */  addu       $s0, $a0, $zero
    /* 6C0E8 8007B8E8 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C0EC 8007B8EC 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C0F0 8007B8F0 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6C0F4 8007B8F4 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6C0F8 8007B8F8 3C00428C */  lw         $v0, 0x3C($v0)
    /* 6C0FC 8007B8FC 03001192 */  lbu        $s1, 0x3($s0)
    /* 6C100 8007B900 09F84000 */  jalr       $v0
    /* 6C104 8007B904 21200000 */   addu      $a0, $zero, $zero
    /* 6C108 8007B908 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C10C 8007B90C 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C110 8007B910 04000426 */  addiu      $a0, $s0, 0x4
    /* 6C114 8007B914 1400428C */  lw         $v0, 0x14($v0)
    /* 6C118 8007B918 00000000 */  nop
    /* 6C11C 8007B91C 09F84000 */  jalr       $v0
    /* 6C120 8007B920 21282002 */   addu      $a1, $s1, $zero
    /* 6C124 8007B924 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6C128 8007B928 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C12C 8007B92C 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C130 8007B930 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6C134 8007B934 0800E003 */  jr         $ra
    /* 6C138 8007B938 00000000 */   nop
endlabel gpu_SendPacket

nonmatching gpu_DrawOTag, 0x74

glabel gpu_DrawOTag
    /* 6C13C 8007B93C 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6C140 8007B940 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6C144 8007B944 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C148 8007B948 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C14C 8007B94C 21808000 */  addu       $s0, $a0, $zero
    /* 6C150 8007B950 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C154 8007B954 08004014 */  bnez       $v0, .L8007B978
    /* 6C158 8007B958 1400BFAF */   sw        $ra, 0x14($sp)
    /* 6C15C 8007B95C 0180043C */  lui        $a0, %hi(D_80015FB0)
    /* 6C160 8007B960 B05F8424 */  addiu      $a0, $a0, %lo(D_80015FB0)
    /* 6C164 8007B964 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6C168 8007B968 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6C16C 8007B96C 00000000 */  nop
    /* 6C170 8007B970 09F84000 */  jalr       $v0
    /* 6C174 8007B974 21280002 */   addu      $a1, $s0, $zero
  .L8007B978:
    /* 6C178 8007B978 21280002 */  addu       $a1, $s0, $zero
    /* 6C17C 8007B97C 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C180 8007B980 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C184 8007B984 21300000 */  addu       $a2, $zero, $zero
    /* 6C188 8007B988 1800448C */  lw         $a0, 0x18($v0)
    /* 6C18C 8007B98C 0800428C */  lw         $v0, 0x8($v0)
    /* 6C190 8007B990 00000000 */  nop
    /* 6C194 8007B994 09F84000 */  jalr       $v0
    /* 6C198 8007B998 21380000 */   addu      $a3, $zero, $zero
    /* 6C19C 8007B99C 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6C1A0 8007B9A0 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C1A4 8007B9A4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6C1A8 8007B9A8 0800E003 */  jr         $ra
    /* 6C1AC 8007B9AC 00000000 */   nop
endlabel gpu_DrawOTag

nonmatching func_8007B9B0, 0x104

glabel func_8007B9B0
    /* 6C1B0 8007B9B0 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6C1B4 8007B9B4 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6C1B8 8007B9B8 0A80123C */  lui        $s2, %hi(D_8009BE76)
    /* 6C1BC 8007B9BC 76BE5226 */  addiu      $s2, $s2, %lo(D_8009BE76)
    /* 6C1C0 8007B9C0 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 6C1C4 8007B9C4 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6C1C8 8007B9C8 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C1CC 8007B9CC 00004292 */  lbu        $v0, 0x0($s2)
    /* 6C1D0 8007B9D0 00000000 */  nop
    /* 6C1D4 8007B9D4 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C1D8 8007B9D8 08004014 */  bnez       $v0, .L8007B9FC
    /* 6C1DC 8007B9DC 21888000 */   addu      $s1, $a0, $zero
    /* 6C1E0 8007B9E0 0180043C */  lui        $a0, %hi(D_80015FC4)
    /* 6C1E4 8007B9E4 C45F8424 */  addiu      $a0, $a0, %lo(D_80015FC4)
    /* 6C1E8 8007B9E8 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6C1EC 8007B9EC 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6C1F0 8007B9F0 00000000 */  nop
    /* 6C1F4 8007B9F4 09F84000 */  jalr       $v0
    /* 6C1F8 8007B9F8 21282002 */   addu      $a1, $s1, $zero
  .L8007B9FC:
    /* 6C1FC 8007B9FC 1C003026 */  addiu      $s0, $s1, 0x1C
    /* 6C200 8007BA00 21200002 */  addu       $a0, $s0, $zero
    /* 6C204 8007BA04 2EF1010C */  jal        func_8007C4B8
    /* 6C208 8007BA08 21282002 */   addu      $a1, $s1, $zero
    /* 6C20C 8007BA0C FF00043C */  lui        $a0, (0xFFFFFF >> 16)
    /* 6C210 8007BA10 FFFF8434 */  ori        $a0, $a0, (0xFFFFFF & 0xFFFF)
    /* 6C214 8007BA14 21280002 */  addu       $a1, $s0, $zero
    /* 6C218 8007BA18 40000624 */  addiu      $a2, $zero, 0x40
    /* 6C21C 8007BA1C 1C00228E */  lw         $v0, 0x1C($s1)
    /* 6C220 8007BA20 0A80033C */  lui        $v1, %hi(D_8009BE6C)
    /* 6C224 8007BA24 6CBE638C */  lw         $v1, %lo(D_8009BE6C)($v1)
    /* 6C228 8007BA28 25104400 */  or         $v0, $v0, $a0
    /* 6C22C 8007BA2C 1C0022AE */  sw         $v0, 0x1C($s1)
    /* 6C230 8007BA30 1800648C */  lw         $a0, 0x18($v1)
    /* 6C234 8007BA34 0800628C */  lw         $v0, 0x8($v1)
    /* 6C238 8007BA38 00000000 */  nop
    /* 6C23C 8007BA3C 09F84000 */  jalr       $v0
    /* 6C240 8007BA40 21380000 */   addu      $a3, $zero, $zero
    /* 6C244 8007BA44 0E004726 */  addiu      $a3, $s2, 0xE
    /* 6C248 8007BA48 21302002 */  addu       $a2, $s1, $zero
    /* 6C24C 8007BA4C 50002826 */  addiu      $t0, $s1, 0x50
  .L8007BA50:
    /* 6C250 8007BA50 0000C28C */  lw         $v0, 0x0($a2)
    /* 6C254 8007BA54 0400C38C */  lw         $v1, 0x4($a2)
    /* 6C258 8007BA58 0800C48C */  lw         $a0, 0x8($a2)
    /* 6C25C 8007BA5C 0C00C58C */  lw         $a1, 0xC($a2)
    /* 6C260 8007BA60 0000E2AC */  sw         $v0, 0x0($a3)
    /* 6C264 8007BA64 0400E3AC */  sw         $v1, 0x4($a3)
    /* 6C268 8007BA68 0800E4AC */  sw         $a0, 0x8($a3)
    /* 6C26C 8007BA6C 0C00E5AC */  sw         $a1, 0xC($a3)
    /* 6C270 8007BA70 1000C624 */  addiu      $a2, $a2, 0x10
    /* 6C274 8007BA74 F6FFC814 */  bne        $a2, $t0, .L8007BA50
    /* 6C278 8007BA78 1000E724 */   addiu     $a3, $a3, 0x10
    /* 6C27C 8007BA7C 0000C28C */  lw         $v0, 0x0($a2)
    /* 6C280 8007BA80 0400C38C */  lw         $v1, 0x4($a2)
    /* 6C284 8007BA84 0800C48C */  lw         $a0, 0x8($a2)
    /* 6C288 8007BA88 0000E2AC */  sw         $v0, 0x0($a3)
    /* 6C28C 8007BA8C 0400E3AC */  sw         $v1, 0x4($a3)
    /* 6C290 8007BA90 0800E4AC */  sw         $a0, 0x8($a3)
    /* 6C294 8007BA94 21102002 */  addu       $v0, $s1, $zero
    /* 6C298 8007BA98 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 6C29C 8007BA9C 1800B28F */  lw         $s2, 0x18($sp)
    /* 6C2A0 8007BAA0 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C2A4 8007BAA4 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C2A8 8007BAA8 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6C2AC 8007BAAC 0800E003 */  jr         $ra
    /* 6C2B0 8007BAB0 00000000 */   nop
endlabel func_8007B9B0

nonmatching func_8007BAB4, 0x11C

glabel func_8007BAB4
    /* 6C2B4 8007BAB4 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6C2B8 8007BAB8 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6C2BC 8007BABC 21908000 */  addu       $s2, $a0, $zero
    /* 6C2C0 8007BAC0 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6C2C4 8007BAC4 0A80133C */  lui        $s3, %hi(D_8009BE76)
    /* 6C2C8 8007BAC8 76BE7326 */  addiu      $s3, $s3, %lo(D_8009BE76)
    /* 6C2CC 8007BACC 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6C2D0 8007BAD0 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6C2D4 8007BAD4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C2D8 8007BAD8 00006292 */  lbu        $v0, 0x0($s3)
    /* 6C2DC 8007BADC 00000000 */  nop
    /* 6C2E0 8007BAE0 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C2E4 8007BAE4 09004014 */  bnez       $v0, .L8007BB0C
    /* 6C2E8 8007BAE8 2188A000 */   addu      $s1, $a1, $zero
    /* 6C2EC 8007BAEC 0180043C */  lui        $a0, %hi(D_80015FDC)
    /* 6C2F0 8007BAF0 DC5F8424 */  addiu      $a0, $a0, %lo(D_80015FDC)
    /* 6C2F4 8007BAF4 21284002 */  addu       $a1, $s2, $zero
    /* 6C2F8 8007BAF8 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6C2FC 8007BAFC 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6C300 8007BB00 00000000 */  nop
    /* 6C304 8007BB04 09F84000 */  jalr       $v0
    /* 6C308 8007BB08 21302002 */   addu      $a2, $s1, $zero
  .L8007BB0C:
    /* 6C30C 8007BB0C 1C003026 */  addiu      $s0, $s1, 0x1C
    /* 6C310 8007BB10 21200002 */  addu       $a0, $s0, $zero
    /* 6C314 8007BB14 2EF1010C */  jal        func_8007C4B8
    /* 6C318 8007BB18 21282002 */   addu      $a1, $s1, $zero
    /* 6C31C 8007BB1C FF00043C */  lui        $a0, (0xFFFFFF >> 16)
    /* 6C320 8007BB20 FFFF8434 */  ori        $a0, $a0, (0xFFFFFF & 0xFFFF)
    /* 6C324 8007BB24 21280002 */  addu       $a1, $s0, $zero
    /* 6C328 8007BB28 40000624 */  addiu      $a2, $zero, 0x40
    /* 6C32C 8007BB2C 00FF033C */  lui        $v1, (0xFF000000 >> 16)
    /* 6C330 8007BB30 1C00228E */  lw         $v0, 0x1C($s1)
    /* 6C334 8007BB34 24204402 */  and        $a0, $s2, $a0
    /* 6C338 8007BB38 24104300 */  and        $v0, $v0, $v1
    /* 6C33C 8007BB3C 0A80033C */  lui        $v1, %hi(D_8009BE6C)
    /* 6C340 8007BB40 6CBE638C */  lw         $v1, %lo(D_8009BE6C)($v1)
    /* 6C344 8007BB44 25104400 */  or         $v0, $v0, $a0
    /* 6C348 8007BB48 1C0022AE */  sw         $v0, 0x1C($s1)
    /* 6C34C 8007BB4C 1800648C */  lw         $a0, 0x18($v1)
    /* 6C350 8007BB50 0800628C */  lw         $v0, 0x8($v1)
    /* 6C354 8007BB54 00000000 */  nop
    /* 6C358 8007BB58 09F84000 */  jalr       $v0
    /* 6C35C 8007BB5C 21380000 */   addu      $a3, $zero, $zero
    /* 6C360 8007BB60 0E006726 */  addiu      $a3, $s3, 0xE
    /* 6C364 8007BB64 21302002 */  addu       $a2, $s1, $zero
    /* 6C368 8007BB68 5000C824 */  addiu      $t0, $a2, 0x50
  .L8007BB6C:
    /* 6C36C 8007BB6C 0000C28C */  lw         $v0, 0x0($a2)
    /* 6C370 8007BB70 0400C38C */  lw         $v1, 0x4($a2)
    /* 6C374 8007BB74 0800C48C */  lw         $a0, 0x8($a2)
    /* 6C378 8007BB78 0C00C58C */  lw         $a1, 0xC($a2)
    /* 6C37C 8007BB7C 0000E2AC */  sw         $v0, 0x0($a3)
    /* 6C380 8007BB80 0400E3AC */  sw         $v1, 0x4($a3)
    /* 6C384 8007BB84 0800E4AC */  sw         $a0, 0x8($a3)
    /* 6C388 8007BB88 0C00E5AC */  sw         $a1, 0xC($a3)
    /* 6C38C 8007BB8C 1000C624 */  addiu      $a2, $a2, 0x10
    /* 6C390 8007BB90 F6FFC814 */  bne        $a2, $t0, .L8007BB6C
    /* 6C394 8007BB94 1000E724 */   addiu     $a3, $a3, 0x10
    /* 6C398 8007BB98 0000C28C */  lw         $v0, 0x0($a2)
    /* 6C39C 8007BB9C 0400C38C */  lw         $v1, 0x4($a2)
    /* 6C3A0 8007BBA0 0800C48C */  lw         $a0, 0x8($a2)
    /* 6C3A4 8007BBA4 0000E2AC */  sw         $v0, 0x0($a3)
    /* 6C3A8 8007BBA8 0400E3AC */  sw         $v1, 0x4($a3)
    /* 6C3AC 8007BBAC 0800E4AC */  sw         $a0, 0x8($a3)
    /* 6C3B0 8007BBB0 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6C3B4 8007BBB4 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6C3B8 8007BBB8 1800B28F */  lw         $s2, 0x18($sp)
    /* 6C3BC 8007BBBC 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C3C0 8007BBC0 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C3C4 8007BBC4 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6C3C8 8007BBC8 0800E003 */  jr         $ra
    /* 6C3CC 8007BBCC 00000000 */   nop
endlabel func_8007BAB4

nonmatching gpu_GetDrawEnv, 0x38

glabel gpu_GetDrawEnv
    /* 6C3D0 8007BBD0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C3D4 8007BBD4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C3D8 8007BBD8 21808000 */  addu       $s0, $a0, $zero
    /* 6C3DC 8007BBDC 0A80053C */  lui        $a1, %hi(D_8009BE84)
    /* 6C3E0 8007BBE0 84BEA524 */  addiu      $a1, $a1, %lo(D_8009BE84)
    /* 6C3E4 8007BBE4 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6C3E8 8007BBE8 48E4010C */  jal        bb2_memcpy
    /* 6C3EC 8007BBEC 5C000624 */   addiu     $a2, $zero, 0x5C
    /* 6C3F0 8007BBF0 21100002 */  addu       $v0, $s0, $zero
    /* 6C3F4 8007BBF4 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6C3F8 8007BBF8 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C3FC 8007BBFC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6C400 8007BC00 0800E003 */  jr         $ra
    /* 6C404 8007BC04 00000000 */   nop
endlabel gpu_GetDrawEnv

nonmatching func_8007BC08, 0x4A8

glabel func_8007BC08
    /* 6C408 8007BC08 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6C40C 8007BC0C 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6C410 8007BC10 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6C414 8007BC14 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C418 8007BC18 21808000 */  addu       $s0, $a0, $zero
    /* 6C41C 8007BC1C 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6C420 8007BC20 0008133C */  lui        $s3, (0x8000008 >> 16)
    /* 6C424 8007BC24 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6C428 8007BC28 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6C42C 8007BC2C 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C430 8007BC30 08004014 */  bnez       $v0, .L8007BC54
    /* 6C434 8007BC34 1400B1AF */   sw        $s1, 0x14($sp)
    /* 6C438 8007BC38 0180043C */  lui        $a0, %hi(D_80015FF8)
    /* 6C43C 8007BC3C F85F8424 */  addiu      $a0, $a0, %lo(D_80015FF8)
    /* 6C440 8007BC40 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6C444 8007BC44 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6C448 8007BC48 00000000 */  nop
    /* 6C44C 8007BC4C 09F84000 */  jalr       $v0
    /* 6C450 8007BC50 21280002 */   addu      $a1, $s0, $zero
  .L8007BC54:
    /* 6C454 8007BC54 0A80023C */  lui        $v0, %hi(D_8009BE74)
    /* 6C458 8007BC58 74BE4290 */  lbu        $v0, %lo(D_8009BE74)($v0)
    /* 6C45C 8007BC5C 00000000 */  nop
    /* 6C460 8007BC60 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6C464 8007BC64 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C468 8007BC68 0A004010 */  beqz       $v0, .L8007BC94
    /* 6C46C 8007BC6C 00000000 */   nop
    /* 6C470 8007BC70 80F2010C */  jal        func_8007CA00
    /* 6C474 8007BC74 21200002 */   addu      $a0, $s0, $zero
    /* 6C478 8007BC78 02000396 */  lhu        $v1, 0x2($s0)
    /* 6C47C 8007BC7C FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 6C480 8007BC80 FF0F6330 */  andi       $v1, $v1, 0xFFF
    /* 6C484 8007BC84 001B0300 */  sll        $v1, $v1, 12
    /* 6C488 8007BC88 25186200 */  or         $v1, $v1, $v0
    /* 6C48C 8007BC8C 2CEF0108 */  j          .L8007BCB0
    /* 6C490 8007BC90 0005023C */   lui       $v0, (0x5000000 >> 16)
  .L8007BC94:
    /* 6C494 8007BC94 02000296 */  lhu        $v0, 0x2($s0)
    /* 6C498 8007BC98 00000396 */  lhu        $v1, 0x0($s0)
    /* 6C49C 8007BC9C FF034230 */  andi       $v0, $v0, 0x3FF
    /* 6C4A0 8007BCA0 80120200 */  sll        $v0, $v0, 10
    /* 6C4A4 8007BCA4 FF036330 */  andi       $v1, $v1, 0x3FF
    /* 6C4A8 8007BCA8 25104300 */  or         $v0, $v0, $v1
    /* 6C4AC 8007BCAC 0005033C */  lui        $v1, (0x5000000 >> 16)
  .L8007BCB0:
    /* 6C4B0 8007BCB0 25204300 */  or         $a0, $v0, $v1
    /* 6C4B4 8007BCB4 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C4B8 8007BCB8 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C4BC 8007BCBC 00000000 */  nop
    /* 6C4C0 8007BCC0 1000428C */  lw         $v0, 0x10($v0)
    /* 6C4C4 8007BCC4 00000000 */  nop
    /* 6C4C8 8007BCC8 09F84000 */  jalr       $v0
    /* 6C4CC 8007BCCC 00000000 */   nop
    /* 6C4D0 8007BCD0 0A80023C */  lui        $v0, %hi(D_8009BEE8)
    /* 6C4D4 8007BCD4 E8BE4224 */  addiu      $v0, $v0, %lo(D_8009BEE8)
    /* 6C4D8 8007BCD8 00004294 */  lhu        $v0, 0x0($v0)
    /* 6C4DC 8007BCDC 08000386 */  lh         $v1, 0x8($s0)
    /* 6C4E0 8007BCE0 00140200 */  sll        $v0, $v0, 16
    /* 6C4E4 8007BCE4 03140200 */  sra        $v0, $v0, 16
    /* 6C4E8 8007BCE8 16004314 */  bne        $v0, $v1, .L8007BD44
    /* 6C4EC 8007BCEC 00000000 */   nop
    /* 6C4F0 8007BCF0 0A80023C */  lui        $v0, %hi(D_8009BEEA)
    /* 6C4F4 8007BCF4 EABE4294 */  lhu        $v0, %lo(D_8009BEEA)($v0)
    /* 6C4F8 8007BCF8 0A000386 */  lh         $v1, 0xA($s0)
    /* 6C4FC 8007BCFC 00140200 */  sll        $v0, $v0, 16
    /* 6C500 8007BD00 03140200 */  sra        $v0, $v0, 16
    /* 6C504 8007BD04 0F004314 */  bne        $v0, $v1, .L8007BD44
    /* 6C508 8007BD08 00000000 */   nop
    /* 6C50C 8007BD0C 0A80023C */  lui        $v0, %hi(D_8009BEEC)
    /* 6C510 8007BD10 ECBE4294 */  lhu        $v0, %lo(D_8009BEEC)($v0)
    /* 6C514 8007BD14 0C000386 */  lh         $v1, 0xC($s0)
    /* 6C518 8007BD18 00140200 */  sll        $v0, $v0, 16
    /* 6C51C 8007BD1C 03140200 */  sra        $v0, $v0, 16
    /* 6C520 8007BD20 08004314 */  bne        $v0, $v1, .L8007BD44
    /* 6C524 8007BD24 00000000 */   nop
    /* 6C528 8007BD28 0A80023C */  lui        $v0, %hi(D_8009BEEE)
    /* 6C52C 8007BD2C EEBE4294 */  lhu        $v0, %lo(D_8009BEEE)($v0)
    /* 6C530 8007BD30 0E000386 */  lh         $v1, 0xE($s0)
    /* 6C534 8007BD34 00140200 */  sll        $v0, $v0, 16
    /* 6C538 8007BD38 03140200 */  sra        $v0, $v0, 16
    /* 6C53C 8007BD3C 73004310 */  beq        $v0, $v1, .L8007BF0C
    /* 6C540 8007BD40 00000000 */   nop
  .L8007BD44:
    /* 6C544 8007BD44 A20D020C */  jal        sys_GetVideoMode
    /* 6C548 8007BD48 00000000 */   nop
    /* 6C54C 8007BD4C 08000486 */  lh         $a0, 0x8($s0)
    /* 6C550 8007BD50 120002A2 */  sb         $v0, 0x12($s0)
    /* 6C554 8007BD54 FF004230 */  andi       $v0, $v0, 0xFF
    /* 6C558 8007BD58 80180400 */  sll        $v1, $a0, 2
    /* 6C55C 8007BD5C 21186400 */  addu       $v1, $v1, $a0
    /* 6C560 8007BD60 40180300 */  sll        $v1, $v1, 1
    /* 6C564 8007BD64 0A000486 */  lh         $a0, 0xA($s0)
    /* 6C568 8007BD68 03004010 */  beqz       $v0, .L8007BD78
    /* 6C56C 8007BD6C 60026324 */   addiu     $v1, $v1, 0x260
    /* 6C570 8007BD70 5FEF0108 */  j          .L8007BD7C
    /* 6C574 8007BD74 13009124 */   addiu     $s1, $a0, 0x13
  .L8007BD78:
    /* 6C578 8007BD78 10009124 */  addiu      $s1, $a0, 0x10
  .L8007BD7C:
    /* 6C57C 8007BD7C 0C000586 */  lh         $a1, 0xC($s0)
    /* 6C580 8007BD80 00000000 */  nop
    /* 6C584 8007BD84 0500A010 */  beqz       $a1, .L8007BD9C
    /* 6C588 8007BD88 80100500 */   sll       $v0, $a1, 2
    /* 6C58C 8007BD8C 21104500 */  addu       $v0, $v0, $a1
    /* 6C590 8007BD90 40100200 */  sll        $v0, $v0, 1
    /* 6C594 8007BD94 68EF0108 */  j          .L8007BDA0
    /* 6C598 8007BD98 21306200 */   addu      $a2, $v1, $v0
  .L8007BD9C:
    /* 6C59C 8007BD9C 000A6624 */  addiu      $a2, $v1, 0xA00
  .L8007BDA0:
    /* 6C5A0 8007BDA0 0E000286 */  lh         $v0, 0xE($s0)
    /* 6C5A4 8007BDA4 00000000 */  nop
    /* 6C5A8 8007BDA8 02004014 */  bnez       $v0, .L8007BDB4
    /* 6C5AC 8007BDAC 21902202 */   addu      $s2, $s1, $v0
    /* 6C5B0 8007BDB0 F0003226 */  addiu      $s2, $s1, 0xF0
  .L8007BDB4:
    /* 6C5B4 8007BDB4 F4016228 */  slti       $v0, $v1, 0x1F4
    /* 6C5B8 8007BDB8 05004014 */  bnez       $v0, .L8007BDD0
    /* 6C5BC 8007BDBC DB0C6228 */   slti      $v0, $v1, 0xCDB
    /* 6C5C0 8007BDC0 04004010 */  beqz       $v0, .L8007BDD4
    /* 6C5C4 8007BDC4 DA0C0524 */   addiu     $a1, $zero, 0xCDA
    /* 6C5C8 8007BDC8 75EF0108 */  j          .L8007BDD4
    /* 6C5CC 8007BDCC 21286000 */   addu      $a1, $v1, $zero
  .L8007BDD0:
    /* 6C5D0 8007BDD0 F4010524 */  addiu      $a1, $zero, 0x1F4
  .L8007BDD4:
    /* 6C5D4 8007BDD4 2118A000 */  addu       $v1, $a1, $zero
    /* 6C5D8 8007BDD8 50006524 */  addiu      $a1, $v1, 0x50
    /* 6C5DC 8007BDDC 2A10C500 */  slt        $v0, $a2, $a1
    /* 6C5E0 8007BDE0 06004014 */  bnez       $v0, .L8007BDFC
    /* 6C5E4 8007BDE4 1000222A */   slti      $v0, $s1, 0x10
    /* 6C5E8 8007BDE8 DB0CC228 */  slti       $v0, $a2, 0xCDB
    /* 6C5EC 8007BDEC 02004010 */  beqz       $v0, .L8007BDF8
    /* 6C5F0 8007BDF0 DA0C0524 */   addiu     $a1, $zero, 0xCDA
    /* 6C5F4 8007BDF4 2128C000 */  addu       $a1, $a2, $zero
  .L8007BDF8:
    /* 6C5F8 8007BDF8 1000222A */  slti       $v0, $s1, 0x10
  .L8007BDFC:
    /* 6C5FC 8007BDFC 14004014 */  bnez       $v0, .L8007BE50
    /* 6C600 8007BE00 2130A000 */   addu      $a2, $a1, $zero
    /* 6C604 8007BE04 12000292 */  lbu        $v0, 0x12($s0)
    /* 6C608 8007BE08 00000000 */  nop
    /* 6C60C 8007BE0C 05004010 */  beqz       $v0, .L8007BE24
    /* 6C610 8007BE10 3701222A */   slti      $v0, $s1, 0x137
    /* 6C614 8007BE14 06004010 */  beqz       $v0, .L8007BE30
    /* 6C618 8007BE18 00000000 */   nop
    /* 6C61C 8007BE1C 95EF0108 */  j          .L8007BE54
    /* 6C620 8007BE20 21202002 */   addu      $a0, $s1, $zero
  .L8007BE24:
    /* 6C624 8007BE24 0101222A */  slti       $v0, $s1, 0x101
    /* 6C628 8007BE28 07004014 */  bnez       $v0, .L8007BE48
    /* 6C62C 8007BE2C 00000000 */   nop
  .L8007BE30:
    /* 6C630 8007BE30 12000292 */  lbu        $v0, 0x12($s0)
    /* 6C634 8007BE34 00000000 */  nop
    /* 6C638 8007BE38 06004010 */  beqz       $v0, .L8007BE54
    /* 6C63C 8007BE3C 00010424 */   addiu     $a0, $zero, 0x100
    /* 6C640 8007BE40 95EF0108 */  j          .L8007BE54
    /* 6C644 8007BE44 36010424 */   addiu     $a0, $zero, 0x136
  .L8007BE48:
    /* 6C648 8007BE48 95EF0108 */  j          .L8007BE54
    /* 6C64C 8007BE4C 21202002 */   addu      $a0, $s1, $zero
  .L8007BE50:
    /* 6C650 8007BE50 10000424 */  addiu      $a0, $zero, 0x10
  .L8007BE54:
    /* 6C654 8007BE54 21888000 */  addu       $s1, $a0, $zero
    /* 6C658 8007BE58 02002526 */  addiu      $a1, $s1, 0x2
    /* 6C65C 8007BE5C 2A104502 */  slt        $v0, $s2, $a1
    /* 6C660 8007BE60 13004014 */  bnez       $v0, .L8007BEB0
    /* 6C664 8007BE64 00000000 */   nop
    /* 6C668 8007BE68 12000292 */  lbu        $v0, 0x12($s0)
    /* 6C66C 8007BE6C 00000000 */  nop
    /* 6C670 8007BE70 05004010 */  beqz       $v0, .L8007BE88
    /* 6C674 8007BE74 3901422A */   slti      $v0, $s2, 0x139
    /* 6C678 8007BE78 06004010 */  beqz       $v0, .L8007BE94
    /* 6C67C 8007BE7C 00000000 */   nop
    /* 6C680 8007BE80 ACEF0108 */  j          .L8007BEB0
    /* 6C684 8007BE84 21284002 */   addu      $a1, $s2, $zero
  .L8007BE88:
    /* 6C688 8007BE88 0301422A */  slti       $v0, $s2, 0x103
    /* 6C68C 8007BE8C 07004014 */  bnez       $v0, .L8007BEAC
    /* 6C690 8007BE90 00000000 */   nop
  .L8007BE94:
    /* 6C694 8007BE94 12000292 */  lbu        $v0, 0x12($s0)
    /* 6C698 8007BE98 00000000 */  nop
    /* 6C69C 8007BE9C 04004010 */  beqz       $v0, .L8007BEB0
    /* 6C6A0 8007BEA0 02010524 */   addiu     $a1, $zero, 0x102
    /* 6C6A4 8007BEA4 ACEF0108 */  j          .L8007BEB0
    /* 6C6A8 8007BEA8 38010524 */   addiu     $a1, $zero, 0x138
  .L8007BEAC:
    /* 6C6AC 8007BEAC 21284002 */  addu       $a1, $s2, $zero
  .L8007BEB0:
    /* 6C6B0 8007BEB0 2190A000 */  addu       $s2, $a1, $zero
    /* 6C6B4 8007BEB4 FF0FC230 */  andi       $v0, $a2, 0xFFF
    /* 6C6B8 8007BEB8 00130200 */  sll        $v0, $v0, 12
    /* 6C6BC 8007BEBC FF0F6430 */  andi       $a0, $v1, 0xFFF
    /* 6C6C0 8007BEC0 0006033C */  lui        $v1, (0x6000000 >> 16)
    /* 6C6C4 8007BEC4 0A80053C */  lui        $a1, %hi(D_8009BE6C)
    /* 6C6C8 8007BEC8 6CBEA58C */  lw         $a1, %lo(D_8009BE6C)($a1)
    /* 6C6CC 8007BECC 25208300 */  or         $a0, $a0, $v1
    /* 6C6D0 8007BED0 1000A38C */  lw         $v1, 0x10($a1)
    /* 6C6D4 8007BED4 00000000 */  nop
    /* 6C6D8 8007BED8 09F86000 */  jalr       $v1
    /* 6C6DC 8007BEDC 25204400 */   or        $a0, $v0, $a0
    /* 6C6E0 8007BEE0 FF034232 */  andi       $v0, $s2, 0x3FF
    /* 6C6E4 8007BEE4 80120200 */  sll        $v0, $v0, 10
    /* 6C6E8 8007BEE8 FF032432 */  andi       $a0, $s1, 0x3FF
    /* 6C6EC 8007BEEC 0007033C */  lui        $v1, (0x7000000 >> 16)
    /* 6C6F0 8007BEF0 0A80053C */  lui        $a1, %hi(D_8009BE6C)
    /* 6C6F4 8007BEF4 6CBEA58C */  lw         $a1, %lo(D_8009BE6C)($a1)
    /* 6C6F8 8007BEF8 25208300 */  or         $a0, $a0, $v1
    /* 6C6FC 8007BEFC 1000A38C */  lw         $v1, 0x10($a1)
    /* 6C700 8007BF00 00000000 */  nop
    /* 6C704 8007BF04 09F86000 */  jalr       $v1
    /* 6C708 8007BF08 25204400 */   or        $a0, $v0, $a0
  .L8007BF0C:
    /* 6C70C 8007BF0C 0A80033C */  lui        $v1, %hi(D_8009BEF0)
    /* 6C710 8007BF10 F0BE638C */  lw         $v1, %lo(D_8009BEF0)($v1)
    /* 6C714 8007BF14 1000028E */  lw         $v0, 0x10($s0)
    /* 6C718 8007BF18 00000000 */  nop
    /* 6C71C 8007BF1C 1D006214 */  bne        $v1, $v0, .L8007BF94
    /* 6C720 8007BF20 00000000 */   nop
    /* 6C724 8007BF24 0A80023C */  lui        $v0, %hi(D_8009BEE0)
    /* 6C728 8007BF28 E0BE4294 */  lhu        $v0, %lo(D_8009BEE0)($v0)
    /* 6C72C 8007BF2C 00000386 */  lh         $v1, 0x0($s0)
    /* 6C730 8007BF30 00140200 */  sll        $v0, $v0, 16
    /* 6C734 8007BF34 03140200 */  sra        $v0, $v0, 16
    /* 6C738 8007BF38 16004314 */  bne        $v0, $v1, .L8007BF94
    /* 6C73C 8007BF3C 00000000 */   nop
    /* 6C740 8007BF40 0A80023C */  lui        $v0, %hi(D_8009BEE2)
    /* 6C744 8007BF44 E2BE4294 */  lhu        $v0, %lo(D_8009BEE2)($v0)
    /* 6C748 8007BF48 02000386 */  lh         $v1, 0x2($s0)
    /* 6C74C 8007BF4C 00140200 */  sll        $v0, $v0, 16
    /* 6C750 8007BF50 03140200 */  sra        $v0, $v0, 16
    /* 6C754 8007BF54 0F004314 */  bne        $v0, $v1, .L8007BF94
    /* 6C758 8007BF58 00000000 */   nop
    /* 6C75C 8007BF5C 0A80023C */  lui        $v0, %hi(D_8009BEE4)
    /* 6C760 8007BF60 E4BE4294 */  lhu        $v0, %lo(D_8009BEE4)($v0)
    /* 6C764 8007BF64 04000386 */  lh         $v1, 0x4($s0)
    /* 6C768 8007BF68 00140200 */  sll        $v0, $v0, 16
    /* 6C76C 8007BF6C 03140200 */  sra        $v0, $v0, 16
    /* 6C770 8007BF70 08004314 */  bne        $v0, $v1, .L8007BF94
    /* 6C774 8007BF74 00000000 */   nop
    /* 6C778 8007BF78 0A80023C */  lui        $v0, %hi(D_8009BEE6)
    /* 6C77C 8007BF7C E6BE4294 */  lhu        $v0, %lo(D_8009BEE6)($v0)
    /* 6C780 8007BF80 06000386 */  lh         $v1, 0x6($s0)
    /* 6C784 8007BF84 00140200 */  sll        $v0, $v0, 16
    /* 6C788 8007BF88 03140200 */  sra        $v0, $v0, 16
    /* 6C78C 8007BF8C 3A004310 */  beq        $v0, $v1, .L8007C078
    /* 6C790 8007BF90 00000000 */   nop
  .L8007BF94:
    /* 6C794 8007BF94 A20D020C */  jal        sys_GetVideoMode
    /* 6C798 8007BF98 00000000 */   nop
    /* 6C79C 8007BF9C 120002A2 */  sb         $v0, 0x12($s0)
    /* 6C7A0 8007BFA0 FF004230 */  andi       $v0, $v0, 0xFF
    /* 6C7A4 8007BFA4 01000324 */  addiu      $v1, $zero, 0x1
    /* 6C7A8 8007BFA8 02004314 */  bne        $v0, $v1, .L8007BFB4
    /* 6C7AC 8007BFAC 00000000 */   nop
    /* 6C7B0 8007BFB0 08007336 */  ori        $s3, $s3, (0x8000008 & 0xFFFF)
  .L8007BFB4:
    /* 6C7B4 8007BFB4 11000292 */  lbu        $v0, 0x11($s0)
    /* 6C7B8 8007BFB8 00000000 */  nop
    /* 6C7BC 8007BFBC 02004010 */  beqz       $v0, .L8007BFC8
    /* 6C7C0 8007BFC0 00000000 */   nop
    /* 6C7C4 8007BFC4 10007336 */  ori        $s3, $s3, (0x8000010 & 0xFFFF)
  .L8007BFC8:
    /* 6C7C8 8007BFC8 10000292 */  lbu        $v0, 0x10($s0)
    /* 6C7CC 8007BFCC 00000000 */  nop
    /* 6C7D0 8007BFD0 02004010 */  beqz       $v0, .L8007BFDC
    /* 6C7D4 8007BFD4 00000000 */   nop
    /* 6C7D8 8007BFD8 20007336 */  ori        $s3, $s3, (0x8000020 & 0xFFFF)
  .L8007BFDC:
    /* 6C7DC 8007BFDC 0A80023C */  lui        $v0, %hi(D_8009BE77)
    /* 6C7E0 8007BFE0 77BE4290 */  lbu        $v0, %lo(D_8009BE77)($v0)
    /* 6C7E4 8007BFE4 00000000 */  nop
    /* 6C7E8 8007BFE8 02004010 */  beqz       $v0, .L8007BFF4
    /* 6C7EC 8007BFEC 00000000 */   nop
    /* 6C7F0 8007BFF0 80007336 */  ori        $s3, $s3, (0x8000080 & 0xFFFF)
  .L8007BFF4:
    /* 6C7F4 8007BFF4 04000386 */  lh         $v1, 0x4($s0)
    /* 6C7F8 8007BFF8 00000000 */  nop
    /* 6C7FC 8007BFFC 19016228 */  slti       $v0, $v1, 0x119
    /* 6C800 8007C000 0E004014 */  bnez       $v0, .L8007C03C
    /* 6C804 8007C004 61016228 */   slti      $v0, $v1, 0x161
    /* 6C808 8007C008 03004010 */  beqz       $v0, .L8007C018
    /* 6C80C 8007C00C 91016228 */   slti      $v0, $v1, 0x191
    /* 6C810 8007C010 0FF00108 */  j          .L8007C03C
    /* 6C814 8007C014 01007336 */   ori       $s3, $s3, (0x8000001 & 0xFFFF)
  .L8007C018:
    /* 6C818 8007C018 03004010 */  beqz       $v0, .L8007C028
    /* 6C81C 8007C01C 31026228 */   slti      $v0, $v1, 0x231
    /* 6C820 8007C020 0FF00108 */  j          .L8007C03C
    /* 6C824 8007C024 40007336 */   ori       $s3, $s3, (0x8000040 & 0xFFFF)
  .L8007C028:
    /* 6C828 8007C028 03004010 */  beqz       $v0, .L8007C038
    /* 6C82C 8007C02C 00000000 */   nop
    /* 6C830 8007C030 0FF00108 */  j          .L8007C03C
    /* 6C834 8007C034 02007336 */   ori       $s3, $s3, (0x8000002 & 0xFFFF)
  .L8007C038:
    /* 6C838 8007C038 03007336 */  ori        $s3, $s3, (0x8000003 & 0xFFFF)
  .L8007C03C:
    /* 6C83C 8007C03C 12000292 */  lbu        $v0, 0x12($s0)
    /* 6C840 8007C040 06000386 */  lh         $v1, 0x6($s0)
    /* 6C844 8007C044 02004014 */  bnez       $v0, .L8007C050
    /* 6C848 8007C048 21016228 */   slti      $v0, $v1, 0x121
    /* 6C84C 8007C04C 01016228 */  slti       $v0, $v1, 0x101
  .L8007C050:
    /* 6C850 8007C050 02004014 */  bnez       $v0, .L8007C05C
    /* 6C854 8007C054 00000000 */   nop
    /* 6C858 8007C058 24007336 */  ori        $s3, $s3, (0x8000024 & 0xFFFF)
  .L8007C05C:
    /* 6C85C 8007C05C 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C860 8007C060 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C864 8007C064 00000000 */  nop
    /* 6C868 8007C068 1000428C */  lw         $v0, 0x10($v0)
    /* 6C86C 8007C06C 00000000 */  nop
    /* 6C870 8007C070 09F84000 */  jalr       $v0
    /* 6C874 8007C074 21206002 */   addu      $a0, $s3, $zero
  .L8007C078:
    /* 6C878 8007C078 0A80043C */  lui        $a0, %hi(D_8009BEE0)
    /* 6C87C 8007C07C E0BE8424 */  addiu      $a0, $a0, %lo(D_8009BEE0)
    /* 6C880 8007C080 21280002 */  addu       $a1, $s0, $zero
    /* 6C884 8007C084 48E4010C */  jal        bb2_memcpy
    /* 6C888 8007C088 14000624 */   addiu     $a2, $zero, 0x14
    /* 6C88C 8007C08C 21100002 */  addu       $v0, $s0, $zero
    /* 6C890 8007C090 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6C894 8007C094 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6C898 8007C098 1800B28F */  lw         $s2, 0x18($sp)
    /* 6C89C 8007C09C 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C8A0 8007C0A0 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C8A4 8007C0A4 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6C8A8 8007C0A8 0800E003 */  jr         $ra
    /* 6C8AC 8007C0AC 00000000 */   nop
endlabel func_8007BC08

nonmatching func_8007C0B0, 0x38

glabel func_8007C0B0
    /* 6C8B0 8007C0B0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C8B4 8007C0B4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C8B8 8007C0B8 21808000 */  addu       $s0, $a0, $zero
    /* 6C8BC 8007C0BC 0A80053C */  lui        $a1, %hi(D_8009BEE0)
    /* 6C8C0 8007C0C0 E0BEA524 */  addiu      $a1, $a1, %lo(D_8009BEE0)
    /* 6C8C4 8007C0C4 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6C8C8 8007C0C8 48E4010C */  jal        bb2_memcpy
    /* 6C8CC 8007C0CC 14000624 */   addiu     $a2, $zero, 0x14
    /* 6C8D0 8007C0D0 21100002 */  addu       $v0, $s0, $zero
    /* 6C8D4 8007C0D4 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6C8D8 8007C0D8 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C8DC 8007C0DC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6C8E0 8007C0E0 0800E003 */  jr         $ra
    /* 6C8E4 8007C0E4 00000000 */   nop
endlabel func_8007C0B0

nonmatching func_8007C0E8, 0x30

glabel func_8007C0E8
    /* 6C8E8 8007C0E8 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C8EC 8007C0EC 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C8F0 8007C0F0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C8F4 8007C0F4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6C8F8 8007C0F8 3800428C */  lw         $v0, 0x38($v0)
    /* 6C8FC 8007C0FC 00000000 */  nop
    /* 6C900 8007C100 09F84000 */  jalr       $v0
    /* 6C904 8007C104 00000000 */   nop
    /* 6C908 8007C108 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6C90C 8007C10C C2170200 */  srl        $v0, $v0, 31
    /* 6C910 8007C110 0800E003 */  jr         $ra
    /* 6C914 8007C114 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_8007C0E8

nonmatching func_8007C118, 0x3C

glabel func_8007C118
    /* 6C918 8007C118 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C91C 8007C11C 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C920 8007C120 21808000 */  addu       $s0, $a0, $zero
    /* 6C924 8007C124 02000224 */  addiu      $v0, $zero, 0x2
    /* 6C928 8007C128 2120A000 */  addu       $a0, $a1, $zero
    /* 6C92C 8007C12C 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6C930 8007C130 5FF2010C */  jal        func_8007C97C
    /* 6C934 8007C134 030002A2 */   sb        $v0, 0x3($s0)
    /* 6C938 8007C138 040002AE */  sw         $v0, 0x4($s0)
    /* 6C93C 8007C13C 080000AE */  sw         $zero, 0x8($s0)
    /* 6C940 8007C140 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6C944 8007C144 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C948 8007C148 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6C94C 8007C14C 0800E003 */  jr         $ra
    /* 6C950 8007C150 00000000 */   nop
endlabel func_8007C118

nonmatching func_8007C154, 0x84

glabel func_8007C154
    /* 6C954 8007C154 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6C958 8007C158 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6C95C 8007C15C 21888000 */  addu       $s1, $a0, $zero
    /* 6C960 8007C160 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C964 8007C164 2180A000 */  addu       $s0, $a1, $zero
    /* 6C968 8007C168 02000224 */  addiu      $v0, $zero, 0x2
    /* 6C96C 8007C16C 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6C970 8007C170 030022A2 */  sb         $v0, 0x3($s1)
    /* 6C974 8007C174 00000486 */  lh         $a0, 0x0($s0)
    /* 6C978 8007C178 02000586 */  lh         $a1, 0x2($s0)
    /* 6C97C 8007C17C E8F1010C */  jal        func_8007C7A0
    /* 6C980 8007C180 00000000 */   nop
    /* 6C984 8007C184 040022AE */  sw         $v0, 0x4($s1)
    /* 6C988 8007C188 00000496 */  lhu        $a0, 0x0($s0)
    /* 6C98C 8007C18C 04000296 */  lhu        $v0, 0x4($s0)
    /* 6C990 8007C190 02000596 */  lhu        $a1, 0x2($s0)
    /* 6C994 8007C194 21208200 */  addu       $a0, $a0, $v0
    /* 6C998 8007C198 FFFF8424 */  addiu      $a0, $a0, -0x1
    /* 6C99C 8007C19C 00240400 */  sll        $a0, $a0, 16
    /* 6C9A0 8007C1A0 06000296 */  lhu        $v0, 0x6($s0)
    /* 6C9A4 8007C1A4 03240400 */  sra        $a0, $a0, 16
    /* 6C9A8 8007C1A8 2128A200 */  addu       $a1, $a1, $v0
    /* 6C9AC 8007C1AC FFFFA524 */  addiu      $a1, $a1, -0x1
    /* 6C9B0 8007C1B0 002C0500 */  sll        $a1, $a1, 16
    /* 6C9B4 8007C1B4 1BF2010C */  jal        func_8007C86C
    /* 6C9B8 8007C1B8 032C0500 */   sra       $a1, $a1, 16
    /* 6C9BC 8007C1BC 080022AE */  sw         $v0, 0x8($s1)
    /* 6C9C0 8007C1C0 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6C9C4 8007C1C4 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C9C8 8007C1C8 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C9CC 8007C1CC 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6C9D0 8007C1D0 0800E003 */  jr         $ra
    /* 6C9D4 8007C1D4 00000000 */   nop
endlabel func_8007C154

nonmatching func_8007C1D8, 0x44

glabel func_8007C1D8
    /* 6C9D8 8007C1D8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C9DC 8007C1DC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C9E0 8007C1E0 21808000 */  addu       $s0, $a0, $zero
    /* 6C9E4 8007C1E4 02000224 */  addiu      $v0, $zero, 0x2
    /* 6C9E8 8007C1E8 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6C9EC 8007C1EC 030002A2 */  sb         $v0, 0x3($s0)
    /* 6C9F0 8007C1F0 0000A484 */  lh         $a0, 0x0($a1)
    /* 6C9F4 8007C1F4 0200A584 */  lh         $a1, 0x2($a1)
    /* 6C9F8 8007C1F8 4EF2010C */  jal        func_8007C938
    /* 6C9FC 8007C1FC 00000000 */   nop
    /* 6CA00 8007C200 040002AE */  sw         $v0, 0x4($s0)
    /* 6CA04 8007C204 080000AE */  sw         $zero, 0x8($s0)
    /* 6CA08 8007C208 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6CA0C 8007C20C 1000B08F */  lw         $s0, 0x10($sp)
    /* 6CA10 8007C210 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6CA14 8007C214 0800E003 */  jr         $ra
    /* 6CA18 8007C218 00000000 */   nop
endlabel func_8007C1D8

nonmatching func_8007C21C, 0x2C

glabel func_8007C21C
    /* 6CA1C 8007C21C 02000224 */  addiu      $v0, $zero, 0x2
    /* 6CA20 8007C220 030082A0 */  sb         $v0, 0x3($a0)
    /* 6CA24 8007C224 0200A010 */  beqz       $a1, .L8007C230
    /* 6CA28 8007C228 00E6023C */   lui       $v0, (0xE6000002 >> 16)
    /* 6CA2C 8007C22C 02004234 */  ori        $v0, $v0, (0xE6000002 & 0xFFFF)
  .L8007C230:
    /* 6CA30 8007C230 0200C010 */  beqz       $a2, .L8007C23C
    /* 6CA34 8007C234 00000000 */   nop
    /* 6CA38 8007C238 01004234 */  ori        $v0, $v0, (0xE6000001 & 0xFFFF)
  .L8007C23C:
    /* 6CA3C 8007C23C 040082AC */  sw         $v0, 0x4($a0)
    /* 6CA40 8007C240 0800E003 */  jr         $ra
    /* 6CA44 8007C244 080080AC */   sw        $zero, 0x8($a0)
endlabel func_8007C21C

nonmatching func_8007C248, 0x58

glabel func_8007C248
    /* 6CA48 8007C248 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6CA4C 8007C24C 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6CA50 8007C250 21808000 */  addu       $s0, $a0, $zero
    /* 6CA54 8007C254 2120A000 */  addu       $a0, $a1, $zero
    /* 6CA58 8007C258 02000224 */  addiu      $v0, $zero, 0x2
    /* 6CA5C 8007C25C 2128C000 */  addu       $a1, $a2, $zero
    /* 6CA60 8007C260 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6CA64 8007C264 3000B18F */  lw         $s1, 0x30($sp)
    /* 6CA68 8007C268 FFFFE630 */  andi       $a2, $a3, 0xFFFF
    /* 6CA6C 8007C26C 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6CA70 8007C270 D2F1010C */  jal        func_8007C748
    /* 6CA74 8007C274 030002A2 */   sb        $v0, 0x3($s0)
    /* 6CA78 8007C278 040002AE */  sw         $v0, 0x4($s0)
    /* 6CA7C 8007C27C 5FF2010C */  jal        func_8007C97C
    /* 6CA80 8007C280 21202002 */   addu      $a0, $s1, $zero
    /* 6CA84 8007C284 080002AE */  sw         $v0, 0x8($s0)
    /* 6CA88 8007C288 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6CA8C 8007C28C 1400B18F */  lw         $s1, 0x14($sp)
    /* 6CA90 8007C290 1000B08F */  lw         $s0, 0x10($sp)
    /* 6CA94 8007C294 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6CA98 8007C298 0800E003 */  jr         $ra
    /* 6CA9C 8007C29C 00000000 */   nop
endlabel func_8007C248

nonmatching func_8007C2A0, 0x218

glabel func_8007C2A0
    /* 6CAA0 8007C2A0 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 6CAA4 8007C2A4 3000B0AF */  sw         $s0, 0x30($sp)
    /* 6CAA8 8007C2A8 2180A000 */  addu       $s0, $a1, $zero
    /* 6CAAC 8007C2AC 3400B1AF */  sw         $s1, 0x34($sp)
    /* 6CAB0 8007C2B0 21888000 */  addu       $s1, $a0, $zero
    /* 6CAB4 8007C2B4 3800BFAF */  sw         $ra, 0x38($sp)
    /* 6CAB8 8007C2B8 00000486 */  lh         $a0, 0x0($s0)
    /* 6CABC 8007C2BC 02000586 */  lh         $a1, 0x2($s0)
    /* 6CAC0 8007C2C0 E8F1010C */  jal        func_8007C7A0
    /* 6CAC4 8007C2C4 00000000 */   nop
    /* 6CAC8 8007C2C8 040022AE */  sw         $v0, 0x4($s1)
    /* 6CACC 8007C2CC 04000496 */  lhu        $a0, 0x4($s0)
    /* 6CAD0 8007C2D0 00000296 */  lhu        $v0, 0x0($s0)
    /* 6CAD4 8007C2D4 02000596 */  lhu        $a1, 0x2($s0)
    /* 6CAD8 8007C2D8 21208200 */  addu       $a0, $a0, $v0
    /* 6CADC 8007C2DC FFFF8424 */  addiu      $a0, $a0, -0x1
    /* 6CAE0 8007C2E0 00240400 */  sll        $a0, $a0, 16
    /* 6CAE4 8007C2E4 06000296 */  lhu        $v0, 0x6($s0)
    /* 6CAE8 8007C2E8 03240400 */  sra        $a0, $a0, 16
    /* 6CAEC 8007C2EC 2128A200 */  addu       $a1, $a1, $v0
    /* 6CAF0 8007C2F0 FFFFA524 */  addiu      $a1, $a1, -0x1
    /* 6CAF4 8007C2F4 002C0500 */  sll        $a1, $a1, 16
    /* 6CAF8 8007C2F8 1BF2010C */  jal        func_8007C86C
    /* 6CAFC 8007C2FC 032C0500 */   sra       $a1, $a1, 16
    /* 6CB00 8007C300 080022AE */  sw         $v0, 0x8($s1)
    /* 6CB04 8007C304 08000486 */  lh         $a0, 0x8($s0)
    /* 6CB08 8007C308 0A000586 */  lh         $a1, 0xA($s0)
    /* 6CB0C 8007C30C 4EF2010C */  jal        func_8007C938
    /* 6CB10 8007C310 00000000 */   nop
    /* 6CB14 8007C314 0C0022AE */  sw         $v0, 0xC($s1)
    /* 6CB18 8007C318 17000492 */  lbu        $a0, 0x17($s0)
    /* 6CB1C 8007C31C 16000592 */  lbu        $a1, 0x16($s0)
    /* 6CB20 8007C320 14000696 */  lhu        $a2, 0x14($s0)
    /* 6CB24 8007C324 D2F1010C */  jal        func_8007C748
    /* 6CB28 8007C328 00000000 */   nop
    /* 6CB2C 8007C32C 0C000426 */  addiu      $a0, $s0, 0xC
    /* 6CB30 8007C330 5FF2010C */  jal        func_8007C97C
    /* 6CB34 8007C334 100022AE */   sw        $v0, 0x10($s1)
    /* 6CB38 8007C338 140022AE */  sw         $v0, 0x14($s1)
    /* 6CB3C 8007C33C 00E6023C */  lui        $v0, (0xE6000000 >> 16)
    /* 6CB40 8007C340 180022AE */  sw         $v0, 0x18($s1)
    /* 6CB44 8007C344 18000292 */  lbu        $v0, 0x18($s0)
    /* 6CB48 8007C348 00000000 */  nop
    /* 6CB4C 8007C34C 52004010 */  beqz       $v0, .L8007C498
    /* 6CB50 8007C350 07000724 */   addiu     $a3, $zero, 0x7
    /* 6CB54 8007C354 00000296 */  lhu        $v0, 0x0($s0)
    /* 6CB58 8007C358 00000000 */  nop
    /* 6CB5C 8007C35C 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 6CB60 8007C360 02000296 */  lhu        $v0, 0x2($s0)
    /* 6CB64 8007C364 00000000 */  nop
    /* 6CB68 8007C368 1200A2A7 */  sh         $v0, 0x12($sp)
    /* 6CB6C 8007C36C 04000496 */  lhu        $a0, 0x4($s0)
    /* 6CB70 8007C370 00000000 */  nop
    /* 6CB74 8007C374 1400A4A7 */  sh         $a0, 0x14($sp)
    /* 6CB78 8007C378 06000296 */  lhu        $v0, 0x6($s0)
    /* 6CB7C 8007C37C 00000000 */  nop
    /* 6CB80 8007C380 1600A2A7 */  sh         $v0, 0x16($sp)
    /* 6CB84 8007C384 00140400 */  sll        $v0, $a0, 16
    /* 6CB88 8007C388 032C0200 */  sra        $a1, $v0, 16
    /* 6CB8C 8007C38C 0A00A004 */  bltz       $a1, .L8007C3B8
    /* 6CB90 8007C390 21100000 */   addu      $v0, $zero, $zero
    /* 6CB94 8007C394 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6CB98 8007C398 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6CB9C 8007C39C 00000000 */  nop
    /* 6CBA0 8007C3A0 21184000 */  addu       $v1, $v0, $zero
    /* 6CBA4 8007C3A4 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6CBA8 8007C3A8 2A104500 */  slt        $v0, $v0, $a1
    /* 6CBAC 8007C3AC 02004014 */  bnez       $v0, .L8007C3B8
    /* 6CBB0 8007C3B0 FFFF6224 */   addiu     $v0, $v1, -0x1
    /* 6CBB4 8007C3B4 21108000 */  addu       $v0, $a0, $zero
  .L8007C3B8:
    /* 6CBB8 8007C3B8 1600A587 */  lh         $a1, 0x16($sp)
    /* 6CBBC 8007C3BC 1400A2A7 */  sh         $v0, 0x14($sp)
    /* 6CBC0 8007C3C0 0B00A004 */  bltz       $a1, .L8007C3F0
    /* 6CBC4 8007C3C4 2120A000 */   addu      $a0, $a1, $zero
    /* 6CBC8 8007C3C8 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6CBCC 8007C3CC 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6CBD0 8007C3D0 00000000 */  nop
    /* 6CBD4 8007C3D4 21184000 */  addu       $v1, $v0, $zero
    /* 6CBD8 8007C3D8 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6CBDC 8007C3DC 2A104500 */  slt        $v0, $v0, $a1
    /* 6CBE0 8007C3E0 04004014 */  bnez       $v0, .L8007C3F4
    /* 6CBE4 8007C3E4 FFFF6224 */   addiu     $v0, $v1, -0x1
    /* 6CBE8 8007C3E8 FDF00108 */  j          .L8007C3F4
    /* 6CBEC 8007C3EC 21108000 */   addu      $v0, $a0, $zero
  .L8007C3F0:
    /* 6CBF0 8007C3F0 21100000 */  addu       $v0, $zero, $zero
  .L8007C3F4:
    /* 6CBF4 8007C3F4 80300700 */  sll        $a2, $a3, 2
    /* 6CBF8 8007C3F8 0100E724 */  addiu      $a3, $a3, 0x1
    /* 6CBFC 8007C3FC 80280700 */  sll        $a1, $a3, 2
    /* 6CC00 8007C400 0100E724 */  addiu      $a3, $a3, 0x1
    /* 6CC04 8007C404 1600A2A7 */  sh         $v0, 0x16($sp)
    /* 6CC08 8007C408 1000A297 */  lhu        $v0, 0x10($sp)
    /* 6CC0C 8007C40C 08000396 */  lhu        $v1, 0x8($s0)
    /* 6CC10 8007C410 2130D100 */  addu       $a2, $a2, $s1
    /* 6CC14 8007C414 23104300 */  subu       $v0, $v0, $v1
    /* 6CC18 8007C418 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 6CC1C 8007C41C 1200A297 */  lhu        $v0, 0x12($sp)
    /* 6CC20 8007C420 0A000396 */  lhu        $v1, 0xA($s0)
    /* 6CC24 8007C424 0060043C */  lui        $a0, (0x60000000 >> 16)
    /* 6CC28 8007C428 23104300 */  subu       $v0, $v0, $v1
    /* 6CC2C 8007C42C 1200A2A7 */  sh         $v0, 0x12($sp)
    /* 6CC30 8007C430 1B000292 */  lbu        $v0, 0x1B($s0)
    /* 6CC34 8007C434 1A000392 */  lbu        $v1, 0x1A($s0)
    /* 6CC38 8007C438 00140200 */  sll        $v0, $v0, 16
    /* 6CC3C 8007C43C 001A0300 */  sll        $v1, $v1, 8
    /* 6CC40 8007C440 25186400 */  or         $v1, $v1, $a0
    /* 6CC44 8007C444 19000492 */  lbu        $a0, 0x19($s0)
    /* 6CC48 8007C448 25104300 */  or         $v0, $v0, $v1
    /* 6CC4C 8007C44C 25104400 */  or         $v0, $v0, $a0
    /* 6CC50 8007C450 0000C2AC */  sw         $v0, 0x0($a2)
    /* 6CC54 8007C454 1000A28F */  lw         $v0, 0x10($sp)
    /* 6CC58 8007C458 2128B100 */  addu       $a1, $a1, $s1
    /* 6CC5C 8007C45C 0000A2AC */  sw         $v0, 0x0($a1)
    /* 6CC60 8007C460 80100700 */  sll        $v0, $a3, 2
    /* 6CC64 8007C464 1400A38F */  lw         $v1, 0x14($sp)
    /* 6CC68 8007C468 21105100 */  addu       $v0, $v0, $s1
    /* 6CC6C 8007C46C 000043AC */  sw         $v1, 0x0($v0)
    /* 6CC70 8007C470 1000A297 */  lhu        $v0, 0x10($sp)
    /* 6CC74 8007C474 08000396 */  lhu        $v1, 0x8($s0)
    /* 6CC78 8007C478 00000000 */  nop
    /* 6CC7C 8007C47C 21104300 */  addu       $v0, $v0, $v1
    /* 6CC80 8007C480 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 6CC84 8007C484 1200A297 */  lhu        $v0, 0x12($sp)
    /* 6CC88 8007C488 0A000396 */  lhu        $v1, 0xA($s0)
    /* 6CC8C 8007C48C 0100E724 */  addiu      $a3, $a3, 0x1
    /* 6CC90 8007C490 21104300 */  addu       $v0, $v0, $v1
    /* 6CC94 8007C494 1200A2A7 */  sh         $v0, 0x12($sp)
  .L8007C498:
    /* 6CC98 8007C498 FFFFE224 */  addiu      $v0, $a3, -0x1
    /* 6CC9C 8007C49C 030022A2 */  sb         $v0, 0x3($s1)
    /* 6CCA0 8007C4A0 3800BF8F */  lw         $ra, 0x38($sp)
    /* 6CCA4 8007C4A4 3400B18F */  lw         $s1, 0x34($sp)
    /* 6CCA8 8007C4A8 3000B08F */  lw         $s0, 0x30($sp)
    /* 6CCAC 8007C4AC 4000BD27 */  addiu      $sp, $sp, 0x40
    /* 6CCB0 8007C4B0 0800E003 */  jr         $ra
    /* 6CCB4 8007C4B4 00000000 */   nop
endlabel func_8007C2A0

nonmatching func_8007C4B8, 0x290

glabel func_8007C4B8
    /* 6CCB8 8007C4B8 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 6CCBC 8007C4BC 3000B0AF */  sw         $s0, 0x30($sp)
    /* 6CCC0 8007C4C0 2180A000 */  addu       $s0, $a1, $zero
    /* 6CCC4 8007C4C4 3400B1AF */  sw         $s1, 0x34($sp)
    /* 6CCC8 8007C4C8 21888000 */  addu       $s1, $a0, $zero
    /* 6CCCC 8007C4CC 3800BFAF */  sw         $ra, 0x38($sp)
    /* 6CCD0 8007C4D0 00000486 */  lh         $a0, 0x0($s0)
    /* 6CCD4 8007C4D4 02000586 */  lh         $a1, 0x2($s0)
    /* 6CCD8 8007C4D8 E8F1010C */  jal        func_8007C7A0
    /* 6CCDC 8007C4DC 00000000 */   nop
    /* 6CCE0 8007C4E0 040022AE */  sw         $v0, 0x4($s1)
    /* 6CCE4 8007C4E4 04000496 */  lhu        $a0, 0x4($s0)
    /* 6CCE8 8007C4E8 00000296 */  lhu        $v0, 0x0($s0)
    /* 6CCEC 8007C4EC 02000596 */  lhu        $a1, 0x2($s0)
    /* 6CCF0 8007C4F0 21208200 */  addu       $a0, $a0, $v0
    /* 6CCF4 8007C4F4 FFFF8424 */  addiu      $a0, $a0, -0x1
    /* 6CCF8 8007C4F8 00240400 */  sll        $a0, $a0, 16
    /* 6CCFC 8007C4FC 06000296 */  lhu        $v0, 0x6($s0)
    /* 6CD00 8007C500 03240400 */  sra        $a0, $a0, 16
    /* 6CD04 8007C504 2128A200 */  addu       $a1, $a1, $v0
    /* 6CD08 8007C508 FFFFA524 */  addiu      $a1, $a1, -0x1
    /* 6CD0C 8007C50C 002C0500 */  sll        $a1, $a1, 16
    /* 6CD10 8007C510 1BF2010C */  jal        func_8007C86C
    /* 6CD14 8007C514 032C0500 */   sra       $a1, $a1, 16
    /* 6CD18 8007C518 080022AE */  sw         $v0, 0x8($s1)
    /* 6CD1C 8007C51C 08000486 */  lh         $a0, 0x8($s0)
    /* 6CD20 8007C520 0A000586 */  lh         $a1, 0xA($s0)
    /* 6CD24 8007C524 4EF2010C */  jal        func_8007C938
    /* 6CD28 8007C528 00000000 */   nop
    /* 6CD2C 8007C52C 0C0022AE */  sw         $v0, 0xC($s1)
    /* 6CD30 8007C530 17000492 */  lbu        $a0, 0x17($s0)
    /* 6CD34 8007C534 16000592 */  lbu        $a1, 0x16($s0)
    /* 6CD38 8007C538 14000696 */  lhu        $a2, 0x14($s0)
    /* 6CD3C 8007C53C D2F1010C */  jal        func_8007C748
    /* 6CD40 8007C540 00000000 */   nop
    /* 6CD44 8007C544 0C000426 */  addiu      $a0, $s0, 0xC
    /* 6CD48 8007C548 5FF2010C */  jal        func_8007C97C
    /* 6CD4C 8007C54C 100022AE */   sw        $v0, 0x10($s1)
    /* 6CD50 8007C550 140022AE */  sw         $v0, 0x14($s1)
    /* 6CD54 8007C554 00E6023C */  lui        $v0, (0xE6000000 >> 16)
    /* 6CD58 8007C558 180022AE */  sw         $v0, 0x18($s1)
    /* 6CD5C 8007C55C 18000292 */  lbu        $v0, 0x18($s0)
    /* 6CD60 8007C560 00000000 */  nop
    /* 6CD64 8007C564 70004010 */  beqz       $v0, .L8007C728
    /* 6CD68 8007C568 07000824 */   addiu     $t0, $zero, 0x7
    /* 6CD6C 8007C56C 00000296 */  lhu        $v0, 0x0($s0)
    /* 6CD70 8007C570 00000000 */  nop
    /* 6CD74 8007C574 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 6CD78 8007C578 02000296 */  lhu        $v0, 0x2($s0)
    /* 6CD7C 8007C57C 00000000 */  nop
    /* 6CD80 8007C580 1200A2A7 */  sh         $v0, 0x12($sp)
    /* 6CD84 8007C584 04000496 */  lhu        $a0, 0x4($s0)
    /* 6CD88 8007C588 00000000 */  nop
    /* 6CD8C 8007C58C 1400A4A7 */  sh         $a0, 0x14($sp)
    /* 6CD90 8007C590 06000296 */  lhu        $v0, 0x6($s0)
    /* 6CD94 8007C594 00000000 */  nop
    /* 6CD98 8007C598 1600A2A7 */  sh         $v0, 0x16($sp)
    /* 6CD9C 8007C59C 00140400 */  sll        $v0, $a0, 16
    /* 6CDA0 8007C5A0 032C0200 */  sra        $a1, $v0, 16
    /* 6CDA4 8007C5A4 0A00A004 */  bltz       $a1, .L8007C5D0
    /* 6CDA8 8007C5A8 21100000 */   addu      $v0, $zero, $zero
    /* 6CDAC 8007C5AC 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6CDB0 8007C5B0 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6CDB4 8007C5B4 00000000 */  nop
    /* 6CDB8 8007C5B8 21184000 */  addu       $v1, $v0, $zero
    /* 6CDBC 8007C5BC FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6CDC0 8007C5C0 2A104500 */  slt        $v0, $v0, $a1
    /* 6CDC4 8007C5C4 02004014 */  bnez       $v0, .L8007C5D0
    /* 6CDC8 8007C5C8 FFFF6224 */   addiu     $v0, $v1, -0x1
    /* 6CDCC 8007C5CC 21108000 */  addu       $v0, $a0, $zero
  .L8007C5D0:
    /* 6CDD0 8007C5D0 1600A587 */  lh         $a1, 0x16($sp)
    /* 6CDD4 8007C5D4 1400A2A7 */  sh         $v0, 0x14($sp)
    /* 6CDD8 8007C5D8 0B00A004 */  bltz       $a1, .L8007C608
    /* 6CDDC 8007C5DC 2120A000 */   addu      $a0, $a1, $zero
    /* 6CDE0 8007C5E0 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6CDE4 8007C5E4 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6CDE8 8007C5E8 00000000 */  nop
    /* 6CDEC 8007C5EC 21184000 */  addu       $v1, $v0, $zero
    /* 6CDF0 8007C5F0 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6CDF4 8007C5F4 2A104500 */  slt        $v0, $v0, $a1
    /* 6CDF8 8007C5F8 04004014 */  bnez       $v0, .L8007C60C
    /* 6CDFC 8007C5FC FFFF6224 */   addiu     $v0, $v1, -0x1
    /* 6CE00 8007C600 83F10108 */  j          .L8007C60C
    /* 6CE04 8007C604 21108000 */   addu      $v0, $a0, $zero
  .L8007C608:
    /* 6CE08 8007C608 21100000 */  addu       $v0, $zero, $zero
  .L8007C60C:
    /* 6CE0C 8007C60C 1000A397 */  lhu        $v1, 0x10($sp)
    /* 6CE10 8007C610 1600A2A7 */  sh         $v0, 0x16($sp)
    /* 6CE14 8007C614 3F006230 */  andi       $v0, $v1, 0x3F
    /* 6CE18 8007C618 06004014 */  bnez       $v0, .L8007C634
    /* 6CE1C 8007C61C 80300800 */   sll       $a2, $t0, 2
    /* 6CE20 8007C620 1400A297 */  lhu        $v0, 0x14($sp)
    /* 6CE24 8007C624 00000000 */  nop
    /* 6CE28 8007C628 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6CE2C 8007C62C 28004010 */  beqz       $v0, .L8007C6D0
    /* 6CE30 8007C630 80280800 */   sll       $a1, $t0, 2
  .L8007C634:
    /* 6CE34 8007C634 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CE38 8007C638 80280800 */  sll        $a1, $t0, 2
    /* 6CE3C 8007C63C 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CE40 8007C640 08000296 */  lhu        $v0, 0x8($s0)
    /* 6CE44 8007C644 2130D100 */  addu       $a2, $a2, $s1
    /* 6CE48 8007C648 23106200 */  subu       $v0, $v1, $v0
    /* 6CE4C 8007C64C 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 6CE50 8007C650 1200A297 */  lhu        $v0, 0x12($sp)
    /* 6CE54 8007C654 0A000396 */  lhu        $v1, 0xA($s0)
    /* 6CE58 8007C658 0060043C */  lui        $a0, (0x60000000 >> 16)
    /* 6CE5C 8007C65C 23104300 */  subu       $v0, $v0, $v1
    /* 6CE60 8007C660 1200A2A7 */  sh         $v0, 0x12($sp)
    /* 6CE64 8007C664 1B000292 */  lbu        $v0, 0x1B($s0)
    /* 6CE68 8007C668 1A000392 */  lbu        $v1, 0x1A($s0)
    /* 6CE6C 8007C66C 00140200 */  sll        $v0, $v0, 16
    /* 6CE70 8007C670 001A0300 */  sll        $v1, $v1, 8
    /* 6CE74 8007C674 25186400 */  or         $v1, $v1, $a0
    /* 6CE78 8007C678 19000492 */  lbu        $a0, 0x19($s0)
    /* 6CE7C 8007C67C 25104300 */  or         $v0, $v0, $v1
    /* 6CE80 8007C680 25104400 */  or         $v0, $v0, $a0
    /* 6CE84 8007C684 0000C2AC */  sw         $v0, 0x0($a2)
    /* 6CE88 8007C688 1000A28F */  lw         $v0, 0x10($sp)
    /* 6CE8C 8007C68C 2128B100 */  addu       $a1, $a1, $s1
    /* 6CE90 8007C690 0000A2AC */  sw         $v0, 0x0($a1)
    /* 6CE94 8007C694 80100800 */  sll        $v0, $t0, 2
    /* 6CE98 8007C698 1400A38F */  lw         $v1, 0x14($sp)
    /* 6CE9C 8007C69C 21105100 */  addu       $v0, $v0, $s1
    /* 6CEA0 8007C6A0 000043AC */  sw         $v1, 0x0($v0)
    /* 6CEA4 8007C6A4 1000A297 */  lhu        $v0, 0x10($sp)
    /* 6CEA8 8007C6A8 08000396 */  lhu        $v1, 0x8($s0)
    /* 6CEAC 8007C6AC 00000000 */  nop
    /* 6CEB0 8007C6B0 21104300 */  addu       $v0, $v0, $v1
    /* 6CEB4 8007C6B4 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 6CEB8 8007C6B8 1200A297 */  lhu        $v0, 0x12($sp)
    /* 6CEBC 8007C6BC 0A000396 */  lhu        $v1, 0xA($s0)
    /* 6CEC0 8007C6C0 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CEC4 8007C6C4 21104300 */  addu       $v0, $v0, $v1
    /* 6CEC8 8007C6C8 CAF10108 */  j          .L8007C728
    /* 6CECC 8007C6CC 1200A2A7 */   sh        $v0, 0x12($sp)
  .L8007C6D0:
    /* 6CED0 8007C6D0 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CED4 8007C6D4 80300800 */  sll        $a2, $t0, 2
    /* 6CED8 8007C6D8 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CEDC 8007C6DC 80380800 */  sll        $a3, $t0, 2
    /* 6CEE0 8007C6E0 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CEE4 8007C6E4 2128B100 */  addu       $a1, $a1, $s1
    /* 6CEE8 8007C6E8 0002043C */  lui        $a0, (0x2000000 >> 16)
    /* 6CEEC 8007C6EC 1B000292 */  lbu        $v0, 0x1B($s0)
    /* 6CEF0 8007C6F0 1A000392 */  lbu        $v1, 0x1A($s0)
    /* 6CEF4 8007C6F4 00140200 */  sll        $v0, $v0, 16
    /* 6CEF8 8007C6F8 001A0300 */  sll        $v1, $v1, 8
    /* 6CEFC 8007C6FC 25186400 */  or         $v1, $v1, $a0
    /* 6CF00 8007C700 19000492 */  lbu        $a0, 0x19($s0)
    /* 6CF04 8007C704 25104300 */  or         $v0, $v0, $v1
    /* 6CF08 8007C708 25104400 */  or         $v0, $v0, $a0
    /* 6CF0C 8007C70C 0000A2AC */  sw         $v0, 0x0($a1)
    /* 6CF10 8007C710 1000A28F */  lw         $v0, 0x10($sp)
    /* 6CF14 8007C714 2130D100 */  addu       $a2, $a2, $s1
    /* 6CF18 8007C718 0000C2AC */  sw         $v0, 0x0($a2)
    /* 6CF1C 8007C71C 1400A28F */  lw         $v0, 0x14($sp)
    /* 6CF20 8007C720 2138F100 */  addu       $a3, $a3, $s1
    /* 6CF24 8007C724 0000E2AC */  sw         $v0, 0x0($a3)
  .L8007C728:
    /* 6CF28 8007C728 FFFF0225 */  addiu      $v0, $t0, -0x1
    /* 6CF2C 8007C72C 030022A2 */  sb         $v0, 0x3($s1)
    /* 6CF30 8007C730 3800BF8F */  lw         $ra, 0x38($sp)
    /* 6CF34 8007C734 3400B18F */  lw         $s1, 0x34($sp)
    /* 6CF38 8007C738 3000B08F */  lw         $s0, 0x30($sp)
    /* 6CF3C 8007C73C 4000BD27 */  addiu      $sp, $sp, 0x40
    /* 6CF40 8007C740 0800E003 */  jr         $ra
    /* 6CF44 8007C744 00000000 */   nop
endlabel func_8007C4B8

nonmatching func_8007C748, 0x58

glabel func_8007C748
    /* 6CF48 8007C748 0A80023C */  lui        $v0, %hi(D_8009BE74)
    /* 6CF4C 8007C74C 74BE4290 */  lbu        $v0, %lo(D_8009BE74)($v0)
    /* 6CF50 8007C750 00000000 */  nop
    /* 6CF54 8007C754 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6CF58 8007C758 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6CF5C 8007C75C 08004010 */  beqz       $v0, .L8007C780
    /* 6CF60 8007C760 00000000 */   nop
    /* 6CF64 8007C764 0200A010 */  beqz       $a1, .L8007C770
    /* 6CF68 8007C768 00E1033C */   lui       $v1, (0xE1000800 >> 16)
    /* 6CF6C 8007C76C 00086334 */  ori        $v1, $v1, (0xE1000800 & 0xFFFF)
  .L8007C770:
    /* 6CF70 8007C770 09008010 */  beqz       $a0, .L8007C798
    /* 6CF74 8007C774 FF27C230 */   andi      $v0, $a2, 0x27FF
    /* 6CF78 8007C778 E6F10108 */  j          .L8007C798
    /* 6CF7C 8007C77C 00104234 */   ori       $v0, $v0, 0x1000
  .L8007C780:
    /* 6CF80 8007C780 0200A010 */  beqz       $a1, .L8007C78C
    /* 6CF84 8007C784 00E1033C */   lui       $v1, (0xE1000200 >> 16)
    /* 6CF88 8007C788 00026334 */  ori        $v1, $v1, (0xE1000200 & 0xFFFF)
  .L8007C78C:
    /* 6CF8C 8007C78C 02008010 */  beqz       $a0, .L8007C798
    /* 6CF90 8007C790 FF09C230 */   andi      $v0, $a2, 0x9FF
    /* 6CF94 8007C794 00044234 */  ori        $v0, $v0, 0x400
  .L8007C798:
    /* 6CF98 8007C798 0800E003 */  jr         $ra
    /* 6CF9C 8007C79C 25106200 */   or        $v0, $v1, $v0
endlabel func_8007C748

nonmatching func_8007C7A0, 0xCC

glabel func_8007C7A0
    /* 6CFA0 8007C7A0 21388000 */  addu       $a3, $a0, $zero
    /* 6CFA4 8007C7A4 00240400 */  sll        $a0, $a0, 16
    /* 6CFA8 8007C7A8 03240400 */  sra        $a0, $a0, 16
    /* 6CFAC 8007C7AC 0B008004 */  bltz       $a0, .L8007C7DC
    /* 6CFB0 8007C7B0 F0FFBD27 */   addiu     $sp, $sp, -0x10
    /* 6CFB4 8007C7B4 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6CFB8 8007C7B8 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6CFBC 8007C7BC 00000000 */  nop
    /* 6CFC0 8007C7C0 21304000 */  addu       $a2, $v0, $zero
    /* 6CFC4 8007C7C4 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6CFC8 8007C7C8 2A104400 */  slt        $v0, $v0, $a0
    /* 6CFCC 8007C7CC 04004014 */  bnez       $v0, .L8007C7E0
    /* 6CFD0 8007C7D0 FFFFC224 */   addiu     $v0, $a2, -0x1
    /* 6CFD4 8007C7D4 F8F10108 */  j          .L8007C7E0
    /* 6CFD8 8007C7D8 2110E000 */   addu      $v0, $a3, $zero
  .L8007C7DC:
    /* 6CFDC 8007C7DC 21100000 */  addu       $v0, $zero, $zero
  .L8007C7E0:
    /* 6CFE0 8007C7E0 21384000 */  addu       $a3, $v0, $zero
    /* 6CFE4 8007C7E4 00140500 */  sll        $v0, $a1, 16
    /* 6CFE8 8007C7E8 03340200 */  sra        $a2, $v0, 16
    /* 6CFEC 8007C7EC 0B00C004 */  bltz       $a2, .L8007C81C
    /* 6CFF0 8007C7F0 00000000 */   nop
    /* 6CFF4 8007C7F4 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6CFF8 8007C7F8 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6CFFC 8007C7FC 00000000 */  nop
    /* 6D000 8007C800 21204000 */  addu       $a0, $v0, $zero
    /* 6D004 8007C804 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D008 8007C808 2A104600 */  slt        $v0, $v0, $a2
    /* 6D00C 8007C80C 04004010 */  beqz       $v0, .L8007C820
    /* 6D010 8007C810 00000000 */   nop
    /* 6D014 8007C814 08F20108 */  j          .L8007C820
    /* 6D018 8007C818 FFFF8524 */   addiu     $a1, $a0, -0x1
  .L8007C81C:
    /* 6D01C 8007C81C 21280000 */  addu       $a1, $zero, $zero
  .L8007C820:
    /* 6D020 8007C820 0A80023C */  lui        $v0, %hi(D_8009BE74)
    /* 6D024 8007C824 74BE4290 */  lbu        $v0, %lo(D_8009BE74)($v0)
    /* 6D028 8007C828 00000000 */  nop
    /* 6D02C 8007C82C FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D030 8007C830 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6D034 8007C834 05004014 */  bnez       $v0, .L8007C84C
    /* 6D038 8007C838 FF0FA330 */   andi      $v1, $a1, 0xFFF
    /* 6D03C 8007C83C FF03A330 */  andi       $v1, $a1, 0x3FF
    /* 6D040 8007C840 801A0300 */  sll        $v1, $v1, 10
    /* 6D044 8007C844 15F20108 */  j          .L8007C854
    /* 6D048 8007C848 FF03E230 */   andi      $v0, $a3, 0x3FF
  .L8007C84C:
    /* 6D04C 8007C84C 001B0300 */  sll        $v1, $v1, 12
    /* 6D050 8007C850 FF0FE230 */  andi       $v0, $a3, 0xFFF
  .L8007C854:
    /* 6D054 8007C854 00E3043C */  lui        $a0, (0xE3000000 >> 16)
    /* 6D058 8007C858 25104400 */  or         $v0, $v0, $a0
    /* 6D05C 8007C85C 25106200 */  or         $v0, $v1, $v0
    /* 6D060 8007C860 1000BD27 */  addiu      $sp, $sp, 0x10
    /* 6D064 8007C864 0800E003 */  jr         $ra
    /* 6D068 8007C868 00000000 */   nop
endlabel func_8007C7A0

nonmatching func_8007C86C, 0xCC

glabel func_8007C86C
    /* 6D06C 8007C86C 21388000 */  addu       $a3, $a0, $zero
    /* 6D070 8007C870 00240400 */  sll        $a0, $a0, 16
    /* 6D074 8007C874 03240400 */  sra        $a0, $a0, 16
    /* 6D078 8007C878 0B008004 */  bltz       $a0, .L8007C8A8
    /* 6D07C 8007C87C F0FFBD27 */   addiu     $sp, $sp, -0x10
    /* 6D080 8007C880 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6D084 8007C884 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6D088 8007C888 00000000 */  nop
    /* 6D08C 8007C88C 21304000 */  addu       $a2, $v0, $zero
    /* 6D090 8007C890 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D094 8007C894 2A104400 */  slt        $v0, $v0, $a0
    /* 6D098 8007C898 04004014 */  bnez       $v0, .L8007C8AC
    /* 6D09C 8007C89C FFFFC224 */   addiu     $v0, $a2, -0x1
    /* 6D0A0 8007C8A0 2BF20108 */  j          .L8007C8AC
    /* 6D0A4 8007C8A4 2110E000 */   addu      $v0, $a3, $zero
  .L8007C8A8:
    /* 6D0A8 8007C8A8 21100000 */  addu       $v0, $zero, $zero
  .L8007C8AC:
    /* 6D0AC 8007C8AC 21384000 */  addu       $a3, $v0, $zero
    /* 6D0B0 8007C8B0 00140500 */  sll        $v0, $a1, 16
    /* 6D0B4 8007C8B4 03340200 */  sra        $a2, $v0, 16
    /* 6D0B8 8007C8B8 0B00C004 */  bltz       $a2, .L8007C8E8
    /* 6D0BC 8007C8BC 00000000 */   nop
    /* 6D0C0 8007C8C0 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6D0C4 8007C8C4 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6D0C8 8007C8C8 00000000 */  nop
    /* 6D0CC 8007C8CC 21204000 */  addu       $a0, $v0, $zero
    /* 6D0D0 8007C8D0 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D0D4 8007C8D4 2A104600 */  slt        $v0, $v0, $a2
    /* 6D0D8 8007C8D8 04004010 */  beqz       $v0, .L8007C8EC
    /* 6D0DC 8007C8DC 00000000 */   nop
    /* 6D0E0 8007C8E0 3BF20108 */  j          .L8007C8EC
    /* 6D0E4 8007C8E4 FFFF8524 */   addiu     $a1, $a0, -0x1
  .L8007C8E8:
    /* 6D0E8 8007C8E8 21280000 */  addu       $a1, $zero, $zero
  .L8007C8EC:
    /* 6D0EC 8007C8EC 0A80023C */  lui        $v0, %hi(D_8009BE74)
    /* 6D0F0 8007C8F0 74BE4290 */  lbu        $v0, %lo(D_8009BE74)($v0)
    /* 6D0F4 8007C8F4 00000000 */  nop
    /* 6D0F8 8007C8F8 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D0FC 8007C8FC 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6D100 8007C900 05004014 */  bnez       $v0, .L8007C918
    /* 6D104 8007C904 FF0FA330 */   andi      $v1, $a1, 0xFFF
    /* 6D108 8007C908 FF03A330 */  andi       $v1, $a1, 0x3FF
    /* 6D10C 8007C90C 801A0300 */  sll        $v1, $v1, 10
    /* 6D110 8007C910 48F20108 */  j          .L8007C920
    /* 6D114 8007C914 FF03E230 */   andi      $v0, $a3, 0x3FF
  .L8007C918:
    /* 6D118 8007C918 001B0300 */  sll        $v1, $v1, 12
    /* 6D11C 8007C91C FF0FE230 */  andi       $v0, $a3, 0xFFF
  .L8007C920:
    /* 6D120 8007C920 00E4043C */  lui        $a0, (0xE4000000 >> 16)
    /* 6D124 8007C924 25104400 */  or         $v0, $v0, $a0
    /* 6D128 8007C928 25106200 */  or         $v0, $v1, $v0
    /* 6D12C 8007C92C 1000BD27 */  addiu      $sp, $sp, 0x10
    /* 6D130 8007C930 0800E003 */  jr         $ra
    /* 6D134 8007C934 00000000 */   nop
endlabel func_8007C86C

nonmatching func_8007C938, 0x44

glabel func_8007C938
    /* 6D138 8007C938 0A80023C */  lui        $v0, %hi(D_8009BE74)
    /* 6D13C 8007C93C 74BE4290 */  lbu        $v0, %lo(D_8009BE74)($v0)
    /* 6D140 8007C940 00000000 */  nop
    /* 6D144 8007C944 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D148 8007C948 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6D14C 8007C94C 05004014 */  bnez       $v0, .L8007C964
    /* 6D150 8007C950 FF0FA330 */   andi      $v1, $a1, 0xFFF
    /* 6D154 8007C954 FF07A330 */  andi       $v1, $a1, 0x7FF
    /* 6D158 8007C958 C01A0300 */  sll        $v1, $v1, 11
    /* 6D15C 8007C95C 5BF20108 */  j          .L8007C96C
    /* 6D160 8007C960 FF078230 */   andi      $v0, $a0, 0x7FF
  .L8007C964:
    /* 6D164 8007C964 001B0300 */  sll        $v1, $v1, 12
    /* 6D168 8007C968 FF0F8230 */  andi       $v0, $a0, 0xFFF
  .L8007C96C:
    /* 6D16C 8007C96C 00E5043C */  lui        $a0, (0xE5000000 >> 16)
    /* 6D170 8007C970 25104400 */  or         $v0, $v0, $a0
    /* 6D174 8007C974 0800E003 */  jr         $ra
    /* 6D178 8007C978 25106200 */   or        $v0, $v1, $v0
endlabel func_8007C938

nonmatching func_8007C97C, 0x84

glabel func_8007C97C
    /* 6D17C 8007C97C 03008014 */  bnez       $a0, .L8007C98C
    /* 6D180 8007C980 F0FFBD27 */   addiu     $sp, $sp, -0x10
    /* 6D184 8007C984 7DF20108 */  j          .L8007C9F4
    /* 6D188 8007C988 21100000 */   addu      $v0, $zero, $zero
  .L8007C98C:
    /* 6D18C 8007C98C 00008590 */  lbu        $a1, 0x0($a0)
    /* 6D190 8007C990 00000000 */  nop
    /* 6D194 8007C994 C2280500 */  srl        $a1, $a1, 3
    /* 6D198 8007C998 0000A5AF */  sw         $a1, 0x0($sp)
    /* 6D19C 8007C99C 04008684 */  lh         $a2, 0x4($a0)
    /* 6D1A0 8007C9A0 00000000 */  nop
    /* 6D1A4 8007C9A4 23300600 */  negu       $a2, $a2
    /* 6D1A8 8007C9A8 FF00C630 */  andi       $a2, $a2, 0xFF
    /* 6D1AC 8007C9AC C3300600 */  sra        $a2, $a2, 3
    /* 6D1B0 8007C9B0 0800A6AF */  sw         $a2, 0x8($sp)
    /* 6D1B4 8007C9B4 02008290 */  lbu        $v0, 0x2($a0)
    /* 6D1B8 8007C9B8 802A0500 */  sll        $a1, $a1, 10
    /* 6D1BC 8007C9BC C2100200 */  srl        $v0, $v0, 3
    /* 6D1C0 8007C9C0 0400A2AF */  sw         $v0, 0x4($sp)
    /* 6D1C4 8007C9C4 C0130200 */  sll        $v0, $v0, 15
    /* 6D1C8 8007C9C8 06008384 */  lh         $v1, 0x6($a0)
    /* 6D1CC 8007C9CC 00E2043C */  lui        $a0, (0xE2000000 >> 16)
    /* 6D1D0 8007C9D0 2528A400 */  or         $a1, $a1, $a0
    /* 6D1D4 8007C9D4 25104500 */  or         $v0, $v0, $a1
    /* 6D1D8 8007C9D8 23180300 */  negu       $v1, $v1
    /* 6D1DC 8007C9DC FF006330 */  andi       $v1, $v1, 0xFF
    /* 6D1E0 8007C9E0 C3180300 */  sra        $v1, $v1, 3
    /* 6D1E4 8007C9E4 40210300 */  sll        $a0, $v1, 5
    /* 6D1E8 8007C9E8 25104400 */  or         $v0, $v0, $a0
    /* 6D1EC 8007C9EC 25104600 */  or         $v0, $v0, $a2
    /* 6D1F0 8007C9F0 0C00A3AF */  sw         $v1, 0xC($sp)
  .L8007C9F4:
    /* 6D1F4 8007C9F4 1000BD27 */  addiu      $sp, $sp, 0x10
    /* 6D1F8 8007C9F8 0800E003 */  jr         $ra
    /* 6D1FC 8007C9FC 00000000 */   nop
endlabel func_8007C97C

nonmatching func_8007CA00, 0xB0

glabel func_8007CA00
    /* 6D200 8007CA00 0A80033C */  lui        $v1, %hi(D_8009BE74)
    /* 6D204 8007CA04 74BE6390 */  lbu        $v1, %lo(D_8009BE74)($v1)
    /* 6D208 8007CA08 01000224 */  addiu      $v0, $zero, 0x1
    /* 6D20C 8007CA0C 05006210 */  beq        $v1, $v0, .L8007CA24
    /* 6D210 8007CA10 02000224 */   addiu     $v0, $zero, 0x2
    /* 6D214 8007CA14 0D006210 */  beq        $v1, $v0, .L8007CA4C
    /* 6D218 8007CA18 00000000 */   nop
    /* 6D21C 8007CA1C A9F20108 */  j          .L8007CAA4
    /* 6D220 8007CA20 00000000 */   nop
  .L8007CA24:
    /* 6D224 8007CA24 0A80023C */  lui        $v0, %hi(D_8009BE77)
    /* 6D228 8007CA28 77BE4290 */  lbu        $v0, %lo(D_8009BE77)($v0)
    /* 6D22C 8007CA2C 00000000 */  nop
    /* 6D230 8007CA30 1C004010 */  beqz       $v0, .L8007CAA4
    /* 6D234 8007CA34 00040224 */   addiu     $v0, $zero, 0x400
    /* 6D238 8007CA38 04008384 */  lh         $v1, 0x4($a0)
    /* 6D23C 8007CA3C 00008484 */  lh         $a0, 0x0($a0)
  .L8007CA40:
    /* 6D240 8007CA40 23104300 */  subu       $v0, $v0, $v1
    /* 6D244 8007CA44 AAF20108 */  j          .L8007CAA8
    /* 6D248 8007CA48 23104400 */   subu      $v0, $v0, $a0
  .L8007CA4C:
    /* 6D24C 8007CA4C 0A80023C */  lui        $v0, %hi(D_8009BE77)
    /* 6D250 8007CA50 77BE4290 */  lbu        $v0, %lo(D_8009BE77)($v0)
    /* 6D254 8007CA54 00000000 */  nop
    /* 6D258 8007CA58 0A004010 */  beqz       $v0, .L8007CA84
    /* 6D25C 8007CA5C 00000000 */   nop
    /* 6D260 8007CA60 04008294 */  lhu        $v0, 0x4($a0)
    /* 6D264 8007CA64 00008484 */  lh         $a0, 0x0($a0)
    /* 6D268 8007CA68 00140200 */  sll        $v0, $v0, 16
    /* 6D26C 8007CA6C 031C0200 */  sra        $v1, $v0, 16
    /* 6D270 8007CA70 C2170200 */  srl        $v0, $v0, 31
    /* 6D274 8007CA74 21186200 */  addu       $v1, $v1, $v0
    /* 6D278 8007CA78 43180300 */  sra        $v1, $v1, 1
    /* 6D27C 8007CA7C 90F20108 */  j          .L8007CA40
    /* 6D280 8007CA80 00040224 */   addiu     $v0, $zero, 0x400
  .L8007CA84:
    /* 6D284 8007CA84 00008294 */  lhu        $v0, 0x0($a0)
    /* 6D288 8007CA88 00000000 */  nop
    /* 6D28C 8007CA8C 00140200 */  sll        $v0, $v0, 16
    /* 6D290 8007CA90 031C0200 */  sra        $v1, $v0, 16
    /* 6D294 8007CA94 C2170200 */  srl        $v0, $v0, 31
    /* 6D298 8007CA98 21186200 */  addu       $v1, $v1, $v0
    /* 6D29C 8007CA9C AAF20108 */  j          .L8007CAA8
    /* 6D2A0 8007CAA0 43100300 */   sra       $v0, $v1, 1
  .L8007CAA4:
    /* 6D2A4 8007CAA4 00008284 */  lh         $v0, 0x0($a0)
  .L8007CAA8:
    /* 6D2A8 8007CAA8 0800E003 */  jr         $ra
    /* 6D2AC 8007CAAC 00000000 */   nop
endlabel func_8007CA00

nonmatching func_8007CAB0, 0x18

glabel func_8007CAB0
    /* 6D2B0 8007CAB0 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D2B4 8007CAB4 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D2B8 8007CAB8 00000000 */  nop
    /* 6D2BC 8007CABC 0000428C */  lw         $v0, 0x0($v0)
    /* 6D2C0 8007CAC0 0800E003 */  jr         $ra
    /* 6D2C4 8007CAC4 00000000 */   nop
endlabel func_8007CAB0

nonmatching func_8007CAC8, 0xE8

glabel func_8007CAC8
    /* 6D2C8 8007CAC8 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6D2CC 8007CACC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6D2D0 8007CAD0 2180A000 */  addu       $s0, $a1, $zero
    /* 6D2D4 8007CAD4 0A80053C */  lui        $a1, %hi(D_8009BF64)
    /* 6D2D8 8007CAD8 64BFA58C */  lw         $a1, %lo(D_8009BF64)($a1)
    /* 6D2DC 8007CADC 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6D2E0 8007CAE0 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6D2E4 8007CAE4 0000A28C */  lw         $v0, 0x0($a1)
    /* 6D2E8 8007CAE8 0008033C */  lui        $v1, (0x8000000 >> 16)
    /* 6D2EC 8007CAEC 25104300 */  or         $v0, $v0, $v1
    /* 6D2F0 8007CAF0 0000A2AC */  sw         $v0, 0x0($a1)
    /* 6D2F4 8007CAF4 0A80023C */  lui        $v0, %hi(D_8009BF60)
    /* 6D2F8 8007CAF8 60BF428C */  lw         $v0, %lo(D_8009BF60)($v0)
    /* 6D2FC 8007CAFC 00000000 */  nop
    /* 6D300 8007CB00 000040AC */  sw         $zero, 0x0($v0)
    /* 6D304 8007CB04 80101000 */  sll        $v0, $s0, 2
    /* 6D308 8007CB08 FCFF4224 */  addiu      $v0, $v0, -0x4
    /* 6D30C 8007CB0C 0A80033C */  lui        $v1, %hi(D_8009BF58)
    /* 6D310 8007CB10 58BF638C */  lw         $v1, %lo(D_8009BF58)($v1)
    /* 6D314 8007CB14 21208200 */  addu       $a0, $a0, $v0
    /* 6D318 8007CB18 000064AC */  sw         $a0, 0x0($v1)
    /* 6D31C 8007CB1C 0A80023C */  lui        $v0, %hi(D_8009BF5C)
    /* 6D320 8007CB20 5CBF428C */  lw         $v0, %lo(D_8009BF5C)($v0)
    /* 6D324 8007CB24 0011033C */  lui        $v1, (0x11000002 >> 16)
    /* 6D328 8007CB28 000050AC */  sw         $s0, 0x0($v0)
    /* 6D32C 8007CB2C 0A80023C */  lui        $v0, %hi(D_8009BF60)
    /* 6D330 8007CB30 60BF428C */  lw         $v0, %lo(D_8009BF60)($v0)
    /* 6D334 8007CB34 02006334 */  ori        $v1, $v1, (0x11000002 & 0xFFFF)
    /* 6D338 8007CB38 000043AC */  sw         $v1, 0x0($v0)
    /* 6D33C 8007CB3C 1AF7010C */  jal        func_8007DC68
    /* 6D340 8007CB40 00000000 */   nop
    /* 6D344 8007CB44 0A80023C */  lui        $v0, %hi(D_8009BF60)
    /* 6D348 8007CB48 60BF428C */  lw         $v0, %lo(D_8009BF60)($v0)
    /* 6D34C 8007CB4C 00000000 */  nop
    /* 6D350 8007CB50 0000428C */  lw         $v0, 0x0($v0)
    /* 6D354 8007CB54 0001033C */  lui        $v1, (0x1000000 >> 16)
    /* 6D358 8007CB58 24104300 */  and        $v0, $v0, $v1
    /* 6D35C 8007CB5C 0E004010 */  beqz       $v0, .L8007CB98
    /* 6D360 8007CB60 21100002 */   addu      $v0, $s0, $zero
    /* 6D364 8007CB64 0001113C */  lui        $s1, (0x1000000 >> 16)
  .L8007CB68:
    /* 6D368 8007CB68 27F7010C */  jal        func_8007DC9C
    /* 6D36C 8007CB6C 00000000 */   nop
    /* 6D370 8007CB70 09004014 */  bnez       $v0, .L8007CB98
    /* 6D374 8007CB74 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 6D378 8007CB78 0A80023C */  lui        $v0, %hi(D_8009BF60)
    /* 6D37C 8007CB7C 60BF428C */  lw         $v0, %lo(D_8009BF60)($v0)
    /* 6D380 8007CB80 00000000 */  nop
    /* 6D384 8007CB84 0000428C */  lw         $v0, 0x0($v0)
    /* 6D388 8007CB88 00000000 */  nop
    /* 6D38C 8007CB8C 24105100 */  and        $v0, $v0, $s1
    /* 6D390 8007CB90 F5FF4014 */  bnez       $v0, .L8007CB68
    /* 6D394 8007CB94 21100002 */   addu      $v0, $s0, $zero
  .L8007CB98:
    /* 6D398 8007CB98 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6D39C 8007CB9C 1400B18F */  lw         $s1, 0x14($sp)
    /* 6D3A0 8007CBA0 1000B08F */  lw         $s0, 0x10($sp)
    /* 6D3A4 8007CBA4 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6D3A8 8007CBA8 0800E003 */  jr         $ra
    /* 6D3AC 8007CBAC 00000000 */   nop
endlabel func_8007CAC8

nonmatching func_8007CBB0, 0x25C

glabel func_8007CBB0
    /* 6D3B0 8007CBB0 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 6D3B4 8007CBB4 21408000 */  addu       $t0, $a0, $zero
    /* 6D3B8 8007CBB8 3800BFAF */  sw         $ra, 0x38($sp)
    /* 6D3BC 8007CBBC 3400B1AF */  sw         $s1, 0x34($sp)
    /* 6D3C0 8007CBC0 3000B0AF */  sw         $s0, 0x30($sp)
    /* 6D3C4 8007CBC4 04000485 */  lh         $a0, 0x4($t0)
    /* 6D3C8 8007CBC8 2148A000 */  addu       $t1, $a1, $zero
    /* 6D3CC 8007CBCC 0B008004 */  bltz       $a0, .L8007CBFC
    /* 6D3D0 8007CBD0 21288000 */   addu      $a1, $a0, $zero
    /* 6D3D4 8007CBD4 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6D3D8 8007CBD8 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6D3DC 8007CBDC 00000000 */  nop
    /* 6D3E0 8007CBE0 21184000 */  addu       $v1, $v0, $zero
    /* 6D3E4 8007CBE4 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D3E8 8007CBE8 2A104400 */  slt        $v0, $v0, $a0
    /* 6D3EC 8007CBEC 04004014 */  bnez       $v0, .L8007CC00
    /* 6D3F0 8007CBF0 FFFF6224 */   addiu     $v0, $v1, -0x1
    /* 6D3F4 8007CBF4 00F30108 */  j          .L8007CC00
    /* 6D3F8 8007CBF8 2110A000 */   addu      $v0, $a1, $zero
  .L8007CBFC:
    /* 6D3FC 8007CBFC 21100000 */  addu       $v0, $zero, $zero
  .L8007CC00:
    /* 6D400 8007CC00 06000585 */  lh         $a1, 0x6($t0)
    /* 6D404 8007CC04 040002A5 */  sh         $v0, 0x4($t0)
    /* 6D408 8007CC08 0B00A004 */  bltz       $a1, .L8007CC38
    /* 6D40C 8007CC0C 2120A000 */   addu      $a0, $a1, $zero
    /* 6D410 8007CC10 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6D414 8007CC14 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6D418 8007CC18 00000000 */  nop
    /* 6D41C 8007CC1C 21184000 */  addu       $v1, $v0, $zero
    /* 6D420 8007CC20 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D424 8007CC24 2A104500 */  slt        $v0, $v0, $a1
    /* 6D428 8007CC28 04004014 */  bnez       $v0, .L8007CC3C
    /* 6D42C 8007CC2C FFFF6324 */   addiu     $v1, $v1, -0x1
    /* 6D430 8007CC30 0FF30108 */  j          .L8007CC3C
    /* 6D434 8007CC34 21188000 */   addu      $v1, $a0, $zero
  .L8007CC38:
    /* 6D438 8007CC38 21180000 */  addu       $v1, $zero, $zero
  .L8007CC3C:
    /* 6D43C 8007CC3C 00000295 */  lhu        $v0, 0x0($t0)
    /* 6D440 8007CC40 00000000 */  nop
    /* 6D444 8007CC44 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6D448 8007CC48 06004014 */  bnez       $v0, .L8007CC64
    /* 6D44C 8007CC4C 060003A5 */   sh        $v1, 0x6($t0)
    /* 6D450 8007CC50 04000295 */  lhu        $v0, 0x4($t0)
    /* 6D454 8007CC54 00000000 */  nop
    /* 6D458 8007CC58 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6D45C 8007CC5C 42004010 */  beqz       $v0, .L8007CD68
    /* 6D460 8007CC60 FF05023C */   lui       $v0, (0x5FFFFFF >> 16)
  .L8007CC64:
    /* 6D464 8007CC64 FF00053C */  lui        $a1, (0xFFFFFF >> 16)
    /* 6D468 8007CC68 FFFFA534 */  ori        $a1, $a1, (0xFFFFFF & 0xFFFF)
    /* 6D46C 8007CC6C FFE4043C */  lui        $a0, (0xE4FFFFFF >> 16)
    /* 6D470 8007CC70 FFFF8434 */  ori        $a0, $a0, (0xE4FFFFFF & 0xFFFF)
    /* 6D474 8007CC74 FF03073C */  lui        $a3, (0x3FFFFFF >> 16)
    /* 6D478 8007CC78 0F80063C */  lui        $a2, %hi(D_800F187C)
    /* 6D47C 8007CC7C 7C18C624 */  addiu      $a2, $a2, %lo(D_800F187C)
    /* 6D480 8007CC80 2410C500 */  and        $v0, $a2, $a1
    /* 6D484 8007CC84 0008033C */  lui        $v1, (0x8000000 >> 16)
    /* 6D488 8007CC88 25104300 */  or         $v0, $v0, $v1
    /* 6D48C 8007CC8C 00E3103C */  lui        $s0, (0xE3000000 >> 16)
    /* 6D490 8007CC90 00E5113C */  lui        $s1, (0xE5000000 >> 16)
    /* 6D494 8007CC94 0F80013C */  lui        $at, %hi(D_800F1858)
    /* 6D498 8007CC98 581822AC */  sw         $v0, %lo(D_800F1858)($at)
    /* 6D49C 8007CC9C 00E6023C */  lui        $v0, (0xE6000000 >> 16)
    /* 6D4A0 8007CCA0 24282501 */  and        $a1, $t1, $a1
    /* 6D4A4 8007CCA4 0060033C */  lui        $v1, (0x60000000 >> 16)
    /* 6D4A8 8007CCA8 2528A300 */  or         $a1, $a1, $v1
    /* 6D4AC 8007CCAC 0F80013C */  lui        $at, %hi(D_800F1868)
    /* 6D4B0 8007CCB0 681822AC */  sw         $v0, %lo(D_800F1868)($at)
    /* 6D4B4 8007CCB4 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D4B8 8007CCB8 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D4BC 8007CCBC 00E1033C */  lui        $v1, (0xE1000000 >> 16)
    /* 6D4C0 8007CCC0 0F80013C */  lui        $at, %hi(D_800F185C)
    /* 6D4C4 8007CCC4 5C1830AC */  sw         $s0, %lo(D_800F185C)($at)
    /* 6D4C8 8007CCC8 0F80013C */  lui        $at, %hi(D_800F1860)
    /* 6D4CC 8007CCCC 601824AC */  sw         $a0, %lo(D_800F1860)($at)
    /* 6D4D0 8007CCD0 0F80013C */  lui        $at, %hi(D_800F1864)
    /* 6D4D4 8007CCD4 641831AC */  sw         $s1, %lo(D_800F1864)($at)
    /* 6D4D8 8007CCD8 0000448C */  lw         $a0, 0x0($v0)
    /* 6D4DC 8007CCDC C2170900 */  srl        $v0, $t1, 31
    /* 6D4E0 8007CCE0 80120200 */  sll        $v0, $v0, 10
    /* 6D4E4 8007CCE4 25104300 */  or         $v0, $v0, $v1
    /* 6D4E8 8007CCE8 0F80013C */  lui        $at, %hi(D_800F1870)
    /* 6D4EC 8007CCEC 701825AC */  sw         $a1, %lo(D_800F1870)($at)
    /* 6D4F0 8007CCF0 FF078430 */  andi       $a0, $a0, 0x7FF
    /* 6D4F4 8007CCF4 25208200 */  or         $a0, $a0, $v0
    /* 6D4F8 8007CCF8 0F80013C */  lui        $at, %hi(D_800F186C)
    /* 6D4FC 8007CCFC 6C1824AC */  sw         $a0, %lo(D_800F186C)($at)
    /* 6D500 8007CD00 0000028D */  lw         $v0, 0x0($t0)
    /* 6D504 8007CD04 FFFFE734 */  ori        $a3, $a3, (0x3FFFFFF & 0xFFFF)
    /* 6D508 8007CD08 0F80013C */  lui        $at, %hi(D_800F1874)
    /* 6D50C 8007CD0C 741822AC */  sw         $v0, %lo(D_800F1874)($at)
    /* 6D510 8007CD10 0400028D */  lw         $v0, 0x4($t0)
    /* 6D514 8007CD14 0000C7AC */  sw         $a3, 0x0($a2)
    /* 6D518 8007CD18 0F80013C */  lui        $at, %hi(D_800F1878)
    /* 6D51C 8007CD1C 781822AC */  sw         $v0, %lo(D_800F1878)($at)
    /* 6D520 8007CD20 E9F4010C */  jal        func_8007D3A4
    /* 6D524 8007CD24 03000424 */   addiu     $a0, $zero, 0x3
    /* 6D528 8007CD28 25105000 */  or         $v0, $v0, $s0
    /* 6D52C 8007CD2C 0F80013C */  lui        $at, %hi(D_800F1880)
    /* 6D530 8007CD30 801822AC */  sw         $v0, %lo(D_800F1880)($at)
    /* 6D534 8007CD34 E9F4010C */  jal        func_8007D3A4
    /* 6D538 8007CD38 04000424 */   addiu     $a0, $zero, 0x4
    /* 6D53C 8007CD3C 00E4033C */  lui        $v1, (0xE4000000 >> 16)
    /* 6D540 8007CD40 25104300 */  or         $v0, $v0, $v1
    /* 6D544 8007CD44 0F80013C */  lui        $at, %hi(D_800F1884)
    /* 6D548 8007CD48 841822AC */  sw         $v0, %lo(D_800F1884)($at)
    /* 6D54C 8007CD4C E9F4010C */  jal        func_8007D3A4
    /* 6D550 8007CD50 05000424 */   addiu     $a0, $zero, 0x5
    /* 6D554 8007CD54 25105100 */  or         $v0, $v0, $s1
    /* 6D558 8007CD58 0F80013C */  lui        $at, %hi(D_800F1888)
    /* 6D55C 8007CD5C 881822AC */  sw         $v0, %lo(D_800F1888)($at)
    /* 6D560 8007CD60 78F30108 */  j          .L8007CDE0
    /* 6D564 8007CD64 00000000 */   nop
  .L8007CD68:
    /* 6D568 8007CD68 FFFF4234 */  ori        $v0, $v0, (0x5FFFFFF & 0xFFFF)
    /* 6D56C 8007CD6C FF00033C */  lui        $v1, (0xFFFFFF >> 16)
    /* 6D570 8007CD70 FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 6D574 8007CD74 0F80013C */  lui        $at, %hi(D_800F1858)
    /* 6D578 8007CD78 581822AC */  sw         $v0, %lo(D_800F1858)($at)
    /* 6D57C 8007CD7C 00E6023C */  lui        $v0, (0xE6000000 >> 16)
    /* 6D580 8007CD80 24182301 */  and        $v1, $t1, $v1
    /* 6D584 8007CD84 0002053C */  lui        $a1, (0x2000000 >> 16)
    /* 6D588 8007CD88 0F80013C */  lui        $at, %hi(D_800F185C)
    /* 6D58C 8007CD8C 5C1822AC */  sw         $v0, %lo(D_800F185C)($at)
    /* 6D590 8007CD90 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D594 8007CD94 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D598 8007CD98 25186500 */  or         $v1, $v1, $a1
    /* 6D59C 8007CD9C 0000448C */  lw         $a0, 0x0($v0)
    /* 6D5A0 8007CDA0 C2170900 */  srl        $v0, $t1, 31
    /* 6D5A4 8007CDA4 80120200 */  sll        $v0, $v0, 10
    /* 6D5A8 8007CDA8 0F80013C */  lui        $at, %hi(D_800F1864)
    /* 6D5AC 8007CDAC 641823AC */  sw         $v1, %lo(D_800F1864)($at)
    /* 6D5B0 8007CDB0 00E1033C */  lui        $v1, (0xE1000000 >> 16)
    /* 6D5B4 8007CDB4 25104300 */  or         $v0, $v0, $v1
    /* 6D5B8 8007CDB8 FF078430 */  andi       $a0, $a0, 0x7FF
    /* 6D5BC 8007CDBC 25208200 */  or         $a0, $a0, $v0
    /* 6D5C0 8007CDC0 0F80013C */  lui        $at, %hi(D_800F1860)
    /* 6D5C4 8007CDC4 601824AC */  sw         $a0, %lo(D_800F1860)($at)
    /* 6D5C8 8007CDC8 0000028D */  lw         $v0, 0x0($t0)
    /* 6D5CC 8007CDCC 0F80013C */  lui        $at, %hi(D_800F1868)
    /* 6D5D0 8007CDD0 681822AC */  sw         $v0, %lo(D_800F1868)($at)
    /* 6D5D4 8007CDD4 0400028D */  lw         $v0, 0x4($t0)
    /* 6D5D8 8007CDD8 0F80013C */  lui        $at, %hi(D_800F186C)
    /* 6D5DC 8007CDDC 6C1822AC */  sw         $v0, %lo(D_800F186C)($at)
  .L8007CDE0:
    /* 6D5E0 8007CDE0 0F80043C */  lui        $a0, %hi(D_800F1858)
    /* 6D5E4 8007CDE4 58188424 */  addiu      $a0, $a0, %lo(D_800F1858)
    /* 6D5E8 8007CDE8 D6F4010C */  jal        func_8007D358
    /* 6D5EC 8007CDEC 00000000 */   nop
    /* 6D5F0 8007CDF0 21100000 */  addu       $v0, $zero, $zero
    /* 6D5F4 8007CDF4 3800BF8F */  lw         $ra, 0x38($sp)
    /* 6D5F8 8007CDF8 3400B18F */  lw         $s1, 0x34($sp)
    /* 6D5FC 8007CDFC 3000B08F */  lw         $s0, 0x30($sp)
    /* 6D600 8007CE00 4000BD27 */  addiu      $sp, $sp, 0x40
    /* 6D604 8007CE04 0800E003 */  jr         $ra
    /* 6D608 8007CE08 00000000 */   nop
endlabel func_8007CBB0

nonmatching func_8007CE0C, 0x23C

glabel func_8007CE0C
    /* 6D60C 8007CE0C B0FFBD27 */  addiu      $sp, $sp, -0x50
    /* 6D610 8007CE10 3400B1AF */  sw         $s1, 0x34($sp)
    /* 6D614 8007CE14 21888000 */  addu       $s1, $a0, $zero
    /* 6D618 8007CE18 3800B2AF */  sw         $s2, 0x38($sp)
    /* 6D61C 8007CE1C 2190A000 */  addu       $s2, $a1, $zero
    /* 6D620 8007CE20 4800BFAF */  sw         $ra, 0x48($sp)
    /* 6D624 8007CE24 4400B5AF */  sw         $s5, 0x44($sp)
    /* 6D628 8007CE28 4000B4AF */  sw         $s4, 0x40($sp)
    /* 6D62C 8007CE2C 3C00B3AF */  sw         $s3, 0x3C($sp)
    /* 6D630 8007CE30 1AF7010C */  jal        func_8007DC68
    /* 6D634 8007CE34 3000B0AF */   sw        $s0, 0x30($sp)
    /* 6D638 8007CE38 04002586 */  lh         $a1, 0x4($s1)
    /* 6D63C 8007CE3C 21A80000 */  addu       $s5, $zero, $zero
    /* 6D640 8007CE40 0A00A004 */  bltz       $a1, .L8007CE6C
    /* 6D644 8007CE44 2118A000 */   addu      $v1, $a1, $zero
    /* 6D648 8007CE48 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6D64C 8007CE4C 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6D650 8007CE50 00000000 */  nop
    /* 6D654 8007CE54 21204000 */  addu       $a0, $v0, $zero
    /* 6D658 8007CE58 2A104500 */  slt        $v0, $v0, $a1
    /* 6D65C 8007CE5C 04004010 */  beqz       $v0, .L8007CE70
    /* 6D660 8007CE60 00000000 */   nop
    /* 6D664 8007CE64 9CF30108 */  j          .L8007CE70
    /* 6D668 8007CE68 21188000 */   addu      $v1, $a0, $zero
  .L8007CE6C:
    /* 6D66C 8007CE6C 21180000 */  addu       $v1, $zero, $zero
  .L8007CE70:
    /* 6D670 8007CE70 06002586 */  lh         $a1, 0x6($s1)
    /* 6D674 8007CE74 040023A6 */  sh         $v1, 0x4($s1)
    /* 6D678 8007CE78 0A00A004 */  bltz       $a1, .L8007CEA4
    /* 6D67C 8007CE7C 2118A000 */   addu      $v1, $a1, $zero
    /* 6D680 8007CE80 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6D684 8007CE84 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6D688 8007CE88 21206000 */  addu       $a0, $v1, $zero
    /* 6D68C 8007CE8C 21184000 */  addu       $v1, $v0, $zero
    /* 6D690 8007CE90 2A104500 */  slt        $v0, $v0, $a1
    /* 6D694 8007CE94 05004010 */  beqz       $v0, .L8007CEAC
    /* 6D698 8007CE98 00140400 */   sll       $v0, $a0, 16
    /* 6D69C 8007CE9C AAF30108 */  j          .L8007CEA8
    /* 6D6A0 8007CEA0 21206000 */   addu      $a0, $v1, $zero
  .L8007CEA4:
    /* 6D6A4 8007CEA4 21200000 */  addu       $a0, $zero, $zero
  .L8007CEA8:
    /* 6D6A8 8007CEA8 00140400 */  sll        $v0, $a0, 16
  .L8007CEAC:
    /* 6D6AC 8007CEAC 04002386 */  lh         $v1, 0x4($s1)
    /* 6D6B0 8007CEB0 03140200 */  sra        $v0, $v0, 16
    /* 6D6B4 8007CEB4 18006200 */  mult       $v1, $v0
    /* 6D6B8 8007CEB8 060024A6 */  sh         $a0, 0x6($s1)
    /* 6D6BC 8007CEBC 12300000 */  mflo       $a2
    /* 6D6C0 8007CEC0 0100C324 */  addiu      $v1, $a2, 0x1
    /* 6D6C4 8007CEC4 C2170300 */  srl        $v0, $v1, 31
    /* 6D6C8 8007CEC8 21186200 */  addu       $v1, $v1, $v0
    /* 6D6CC 8007CECC 43200300 */  sra        $a0, $v1, 1
    /* 6D6D0 8007CED0 0300801C */  bgtz       $a0, .L8007CEE0
    /* 6D6D4 8007CED4 43810300 */   sra       $s0, $v1, 5
    /* 6D6D8 8007CED8 08F40108 */  j          .L8007D020
    /* 6D6DC 8007CEDC FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8007CEE0:
    /* 6D6E0 8007CEE0 21180002 */  addu       $v1, $s0, $zero
    /* 6D6E4 8007CEE4 00110300 */  sll        $v0, $v1, 4
    /* 6D6E8 8007CEE8 23808200 */  subu       $s0, $a0, $v0
    /* 6D6EC 8007CEEC 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D6F0 8007CEF0 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D6F4 8007CEF4 21A06000 */  addu       $s4, $v1, $zero
    /* 6D6F8 8007CEF8 0000428C */  lw         $v0, 0x0($v0)
    /* 6D6FC 8007CEFC 0004033C */  lui        $v1, (0x4000000 >> 16)
    /* 6D700 8007CF00 24104300 */  and        $v0, $v0, $v1
    /* 6D704 8007CF04 0E004014 */  bnez       $v0, .L8007CF40
    /* 6D708 8007CF08 00A0043C */   lui       $a0, (0xA0000000 >> 16)
    /* 6D70C 8007CF0C 0004133C */  lui        $s3, (0x4000000 >> 16)
  .L8007CF10:
    /* 6D710 8007CF10 27F7010C */  jal        func_8007DC9C
    /* 6D714 8007CF14 00000000 */   nop
    /* 6D718 8007CF18 41004014 */  bnez       $v0, .L8007D020
    /* 6D71C 8007CF1C FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 6D720 8007CF20 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D724 8007CF24 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D728 8007CF28 00000000 */  nop
    /* 6D72C 8007CF2C 0000428C */  lw         $v0, 0x0($v0)
    /* 6D730 8007CF30 00000000 */  nop
    /* 6D734 8007CF34 24105300 */  and        $v0, $v0, $s3
    /* 6D738 8007CF38 F5FF4010 */  beqz       $v0, .L8007CF10
    /* 6D73C 8007CF3C 00A0043C */   lui       $a0, (0xA0000000 >> 16)
  .L8007CF40:
    /* 6D740 8007CF40 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6D744 8007CF44 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6D748 8007CF48 0004023C */  lui        $v0, (0x4000000 >> 16)
    /* 6D74C 8007CF4C 000062AC */  sw         $v0, 0x0($v1)
    /* 6D750 8007CF50 0A80033C */  lui        $v1, %hi(D_8009BF44)
    /* 6D754 8007CF54 44BF638C */  lw         $v1, %lo(D_8009BF44)($v1)
    /* 6D758 8007CF58 0001023C */  lui        $v0, (0x1000000 >> 16)
    /* 6D75C 8007CF5C 000062AC */  sw         $v0, 0x0($v1)
    /* 6D760 8007CF60 0A80023C */  lui        $v0, %hi(D_8009BF44)
    /* 6D764 8007CF64 44BF428C */  lw         $v0, %lo(D_8009BF44)($v0)
    /* 6D768 8007CF68 0200A012 */  beqz       $s5, .L8007CF74
    /* 6D76C 8007CF6C 00000000 */   nop
    /* 6D770 8007CF70 00B0043C */  lui        $a0, (0xB0000000 >> 16)
  .L8007CF74:
    /* 6D774 8007CF74 000044AC */  sw         $a0, 0x0($v0)
    /* 6D778 8007CF78 0A80033C */  lui        $v1, %hi(D_8009BF44)
    /* 6D77C 8007CF7C 44BF638C */  lw         $v1, %lo(D_8009BF44)($v1)
    /* 6D780 8007CF80 0000228E */  lw         $v0, 0x0($s1)
    /* 6D784 8007CF84 00000000 */  nop
    /* 6D788 8007CF88 000062AC */  sw         $v0, 0x0($v1)
    /* 6D78C 8007CF8C 0A80033C */  lui        $v1, %hi(D_8009BF44)
    /* 6D790 8007CF90 44BF638C */  lw         $v1, %lo(D_8009BF44)($v1)
    /* 6D794 8007CF94 0400228E */  lw         $v0, 0x4($s1)
    /* 6D798 8007CF98 FFFF1026 */  addiu      $s0, $s0, -0x1
    /* 6D79C 8007CF9C 000062AC */  sw         $v0, 0x0($v1)
    /* 6D7A0 8007CFA0 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 6D7A4 8007CFA4 0A000212 */  beq        $s0, $v0, .L8007CFD0
    /* 6D7A8 8007CFA8 00000000 */   nop
    /* 6D7AC 8007CFAC FFFF0424 */  addiu      $a0, $zero, -0x1
  .L8007CFB0:
    /* 6D7B0 8007CFB0 0000438E */  lw         $v1, 0x0($s2)
    /* 6D7B4 8007CFB4 04005226 */  addiu      $s2, $s2, 0x4
    /* 6D7B8 8007CFB8 0A80023C */  lui        $v0, %hi(D_8009BF44)
    /* 6D7BC 8007CFBC 44BF428C */  lw         $v0, %lo(D_8009BF44)($v0)
    /* 6D7C0 8007CFC0 FFFF1026 */  addiu      $s0, $s0, -0x1
    /* 6D7C4 8007CFC4 000043AC */  sw         $v1, 0x0($v0)
    /* 6D7C8 8007CFC8 F9FF0416 */  bne        $s0, $a0, .L8007CFB0
    /* 6D7CC 8007CFCC 00000000 */   nop
  .L8007CFD0:
    /* 6D7D0 8007CFD0 12008012 */  beqz       $s4, .L8007D01C
    /* 6D7D4 8007CFD4 0004033C */   lui       $v1, (0x4000002 >> 16)
    /* 6D7D8 8007CFD8 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D7DC 8007CFDC 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D7E0 8007CFE0 02006334 */  ori        $v1, $v1, (0x4000002 & 0xFFFF)
    /* 6D7E4 8007CFE4 000043AC */  sw         $v1, 0x0($v0)
    /* 6D7E8 8007CFE8 0A80023C */  lui        $v0, %hi(D_8009BF4C)
    /* 6D7EC 8007CFEC 4CBF428C */  lw         $v0, %lo(D_8009BF4C)($v0)
    /* 6D7F0 8007CFF0 0001043C */  lui        $a0, (0x1000201 >> 16)
    /* 6D7F4 8007CFF4 000052AC */  sw         $s2, 0x0($v0)
    /* 6D7F8 8007CFF8 00141400 */  sll        $v0, $s4, 16
    /* 6D7FC 8007CFFC 0A80033C */  lui        $v1, %hi(D_8009BF50)
    /* 6D800 8007D000 50BF638C */  lw         $v1, %lo(D_8009BF50)($v1)
    /* 6D804 8007D004 10004234 */  ori        $v0, $v0, 0x10
    /* 6D808 8007D008 000062AC */  sw         $v0, 0x0($v1)
    /* 6D80C 8007D00C 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6D810 8007D010 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6D814 8007D014 01028434 */  ori        $a0, $a0, (0x1000201 & 0xFFFF)
    /* 6D818 8007D018 000044AC */  sw         $a0, 0x0($v0)
  .L8007D01C:
    /* 6D81C 8007D01C 21100000 */  addu       $v0, $zero, $zero
  .L8007D020:
    /* 6D820 8007D020 4800BF8F */  lw         $ra, 0x48($sp)
    /* 6D824 8007D024 4400B58F */  lw         $s5, 0x44($sp)
    /* 6D828 8007D028 4000B48F */  lw         $s4, 0x40($sp)
    /* 6D82C 8007D02C 3C00B38F */  lw         $s3, 0x3C($sp)
    /* 6D830 8007D030 3800B28F */  lw         $s2, 0x38($sp)
    /* 6D834 8007D034 3400B18F */  lw         $s1, 0x34($sp)
    /* 6D838 8007D038 3000B08F */  lw         $s0, 0x30($sp)
    /* 6D83C 8007D03C 5000BD27 */  addiu      $sp, $sp, 0x50
    /* 6D840 8007D040 0800E003 */  jr         $ra
    /* 6D844 8007D044 00000000 */   nop
endlabel func_8007CE0C

nonmatching func_8007D048, 0x284

glabel func_8007D048
    /* 6D848 8007D048 B8FFBD27 */  addiu      $sp, $sp, -0x48
    /* 6D84C 8007D04C 3400B1AF */  sw         $s1, 0x34($sp)
    /* 6D850 8007D050 21888000 */  addu       $s1, $a0, $zero
    /* 6D854 8007D054 3800B2AF */  sw         $s2, 0x38($sp)
    /* 6D858 8007D058 2190A000 */  addu       $s2, $a1, $zero
    /* 6D85C 8007D05C 4400BFAF */  sw         $ra, 0x44($sp)
    /* 6D860 8007D060 4000B4AF */  sw         $s4, 0x40($sp)
    /* 6D864 8007D064 3C00B3AF */  sw         $s3, 0x3C($sp)
    /* 6D868 8007D068 1AF7010C */  jal        func_8007DC68
    /* 6D86C 8007D06C 3000B0AF */   sw        $s0, 0x30($sp)
    /* 6D870 8007D070 04002586 */  lh         $a1, 0x4($s1)
    /* 6D874 8007D074 00000000 */  nop
    /* 6D878 8007D078 0A00A004 */  bltz       $a1, .L8007D0A4
    /* 6D87C 8007D07C 2118A000 */   addu      $v1, $a1, $zero
    /* 6D880 8007D080 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6D884 8007D084 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6D888 8007D088 00000000 */  nop
    /* 6D88C 8007D08C 21204000 */  addu       $a0, $v0, $zero
    /* 6D890 8007D090 2A104500 */  slt        $v0, $v0, $a1
    /* 6D894 8007D094 04004010 */  beqz       $v0, .L8007D0A8
    /* 6D898 8007D098 00000000 */   nop
    /* 6D89C 8007D09C 2AF40108 */  j          .L8007D0A8
    /* 6D8A0 8007D0A0 21188000 */   addu      $v1, $a0, $zero
  .L8007D0A4:
    /* 6D8A4 8007D0A4 21180000 */  addu       $v1, $zero, $zero
  .L8007D0A8:
    /* 6D8A8 8007D0A8 06002586 */  lh         $a1, 0x6($s1)
    /* 6D8AC 8007D0AC 040023A6 */  sh         $v1, 0x4($s1)
    /* 6D8B0 8007D0B0 0A00A004 */  bltz       $a1, .L8007D0DC
    /* 6D8B4 8007D0B4 2118A000 */   addu      $v1, $a1, $zero
    /* 6D8B8 8007D0B8 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6D8BC 8007D0BC 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6D8C0 8007D0C0 21206000 */  addu       $a0, $v1, $zero
    /* 6D8C4 8007D0C4 21184000 */  addu       $v1, $v0, $zero
    /* 6D8C8 8007D0C8 2A104500 */  slt        $v0, $v0, $a1
    /* 6D8CC 8007D0CC 05004010 */  beqz       $v0, .L8007D0E4
    /* 6D8D0 8007D0D0 00140400 */   sll       $v0, $a0, 16
    /* 6D8D4 8007D0D4 38F40108 */  j          .L8007D0E0
    /* 6D8D8 8007D0D8 21206000 */   addu      $a0, $v1, $zero
  .L8007D0DC:
    /* 6D8DC 8007D0DC 21200000 */  addu       $a0, $zero, $zero
  .L8007D0E0:
    /* 6D8E0 8007D0E0 00140400 */  sll        $v0, $a0, 16
  .L8007D0E4:
    /* 6D8E4 8007D0E4 04002386 */  lh         $v1, 0x4($s1)
    /* 6D8E8 8007D0E8 03140200 */  sra        $v0, $v0, 16
    /* 6D8EC 8007D0EC 18006200 */  mult       $v1, $v0
    /* 6D8F0 8007D0F0 060024A6 */  sh         $a0, 0x6($s1)
    /* 6D8F4 8007D0F4 12300000 */  mflo       $a2
    /* 6D8F8 8007D0F8 0100C324 */  addiu      $v1, $a2, 0x1
    /* 6D8FC 8007D0FC C2170300 */  srl        $v0, $v1, 31
    /* 6D900 8007D100 21186200 */  addu       $v1, $v1, $v0
    /* 6D904 8007D104 43200300 */  sra        $a0, $v1, 1
    /* 6D908 8007D108 0300801C */  bgtz       $a0, .L8007D118
    /* 6D90C 8007D10C 43810300 */   sra       $s0, $v1, 5
    /* 6D910 8007D110 AAF40108 */  j          .L8007D2A8
    /* 6D914 8007D114 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8007D118:
    /* 6D918 8007D118 21180002 */  addu       $v1, $s0, $zero
    /* 6D91C 8007D11C 00110300 */  sll        $v0, $v1, 4
    /* 6D920 8007D120 23808200 */  subu       $s0, $a0, $v0
    /* 6D924 8007D124 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D928 8007D128 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D92C 8007D12C 21A06000 */  addu       $s4, $v1, $zero
    /* 6D930 8007D130 0000428C */  lw         $v0, 0x0($v0)
    /* 6D934 8007D134 0004033C */  lui        $v1, (0x4000000 >> 16)
    /* 6D938 8007D138 24104300 */  and        $v0, $v0, $v1
    /* 6D93C 8007D13C 0E004014 */  bnez       $v0, .L8007D178
    /* 6D940 8007D140 00000000 */   nop
    /* 6D944 8007D144 0004133C */  lui        $s3, (0x4000000 >> 16)
  .L8007D148:
    /* 6D948 8007D148 27F7010C */  jal        func_8007DC9C
    /* 6D94C 8007D14C 00000000 */   nop
    /* 6D950 8007D150 55004014 */  bnez       $v0, .L8007D2A8
    /* 6D954 8007D154 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 6D958 8007D158 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D95C 8007D15C 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D960 8007D160 00000000 */  nop
    /* 6D964 8007D164 0000428C */  lw         $v0, 0x0($v0)
    /* 6D968 8007D168 00000000 */  nop
    /* 6D96C 8007D16C 24105300 */  and        $v0, $v0, $s3
    /* 6D970 8007D170 F5FF4010 */  beqz       $v0, .L8007D148
    /* 6D974 8007D174 00000000 */   nop
  .L8007D178:
    /* 6D978 8007D178 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6D97C 8007D17C 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6D980 8007D180 0004023C */  lui        $v0, (0x4000000 >> 16)
    /* 6D984 8007D184 000062AC */  sw         $v0, 0x0($v1)
    /* 6D988 8007D188 0A80033C */  lui        $v1, %hi(D_8009BF44)
    /* 6D98C 8007D18C 44BF638C */  lw         $v1, %lo(D_8009BF44)($v1)
    /* 6D990 8007D190 0001023C */  lui        $v0, (0x1000000 >> 16)
    /* 6D994 8007D194 000062AC */  sw         $v0, 0x0($v1)
    /* 6D998 8007D198 0A80033C */  lui        $v1, %hi(D_8009BF44)
    /* 6D99C 8007D19C 44BF638C */  lw         $v1, %lo(D_8009BF44)($v1)
    /* 6D9A0 8007D1A0 00C0023C */  lui        $v0, (0xC0000000 >> 16)
    /* 6D9A4 8007D1A4 000062AC */  sw         $v0, 0x0($v1)
    /* 6D9A8 8007D1A8 0A80033C */  lui        $v1, %hi(D_8009BF44)
    /* 6D9AC 8007D1AC 44BF638C */  lw         $v1, %lo(D_8009BF44)($v1)
    /* 6D9B0 8007D1B0 0000228E */  lw         $v0, 0x0($s1)
    /* 6D9B4 8007D1B4 00000000 */  nop
    /* 6D9B8 8007D1B8 000062AC */  sw         $v0, 0x0($v1)
    /* 6D9BC 8007D1BC 0A80033C */  lui        $v1, %hi(D_8009BF44)
    /* 6D9C0 8007D1C0 44BF638C */  lw         $v1, %lo(D_8009BF44)($v1)
    /* 6D9C4 8007D1C4 0400228E */  lw         $v0, 0x4($s1)
    /* 6D9C8 8007D1C8 00000000 */  nop
    /* 6D9CC 8007D1CC 000062AC */  sw         $v0, 0x0($v1)
    /* 6D9D0 8007D1D0 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D9D4 8007D1D4 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D9D8 8007D1D8 00000000 */  nop
    /* 6D9DC 8007D1DC 0000428C */  lw         $v0, 0x0($v0)
    /* 6D9E0 8007D1E0 0008033C */  lui        $v1, (0x8000000 >> 16)
    /* 6D9E4 8007D1E4 24104300 */  and        $v0, $v0, $v1
    /* 6D9E8 8007D1E8 0E004014 */  bnez       $v0, .L8007D224
    /* 6D9EC 8007D1EC 00000000 */   nop
    /* 6D9F0 8007D1F0 0008113C */  lui        $s1, (0x8000000 >> 16)
  .L8007D1F4:
    /* 6D9F4 8007D1F4 27F7010C */  jal        func_8007DC9C
    /* 6D9F8 8007D1F8 00000000 */   nop
    /* 6D9FC 8007D1FC 2A004014 */  bnez       $v0, .L8007D2A8
    /* 6DA00 8007D200 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 6DA04 8007D204 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6DA08 8007D208 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6DA0C 8007D20C 00000000 */  nop
    /* 6DA10 8007D210 0000428C */  lw         $v0, 0x0($v0)
    /* 6DA14 8007D214 00000000 */  nop
    /* 6DA18 8007D218 24105100 */  and        $v0, $v0, $s1
    /* 6DA1C 8007D21C F5FF4010 */  beqz       $v0, .L8007D1F4
    /* 6DA20 8007D220 00000000 */   nop
  .L8007D224:
    /* 6DA24 8007D224 FFFF1026 */  addiu      $s0, $s0, -0x1
    /* 6DA28 8007D228 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 6DA2C 8007D22C 0A000212 */  beq        $s0, $v0, .L8007D258
    /* 6DA30 8007D230 00000000 */   nop
    /* 6DA34 8007D234 FFFF0324 */  addiu      $v1, $zero, -0x1
  .L8007D238:
    /* 6DA38 8007D238 0A80023C */  lui        $v0, %hi(D_8009BF44)
    /* 6DA3C 8007D23C 44BF428C */  lw         $v0, %lo(D_8009BF44)($v0)
    /* 6DA40 8007D240 00000000 */  nop
    /* 6DA44 8007D244 0000428C */  lw         $v0, 0x0($v0)
    /* 6DA48 8007D248 FFFF1026 */  addiu      $s0, $s0, -0x1
    /* 6DA4C 8007D24C 000042AE */  sw         $v0, 0x0($s2)
    /* 6DA50 8007D250 F9FF0316 */  bne        $s0, $v1, .L8007D238
    /* 6DA54 8007D254 04005226 */   addiu     $s2, $s2, 0x4
  .L8007D258:
    /* 6DA58 8007D258 12008012 */  beqz       $s4, .L8007D2A4
    /* 6DA5C 8007D25C 0004033C */   lui       $v1, (0x4000003 >> 16)
    /* 6DA60 8007D260 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6DA64 8007D264 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6DA68 8007D268 03006334 */  ori        $v1, $v1, (0x4000003 & 0xFFFF)
    /* 6DA6C 8007D26C 000043AC */  sw         $v1, 0x0($v0)
    /* 6DA70 8007D270 0A80023C */  lui        $v0, %hi(D_8009BF4C)
    /* 6DA74 8007D274 4CBF428C */  lw         $v0, %lo(D_8009BF4C)($v0)
    /* 6DA78 8007D278 0001043C */  lui        $a0, (0x1000200 >> 16)
    /* 6DA7C 8007D27C 000052AC */  sw         $s2, 0x0($v0)
    /* 6DA80 8007D280 00141400 */  sll        $v0, $s4, 16
    /* 6DA84 8007D284 0A80033C */  lui        $v1, %hi(D_8009BF50)
    /* 6DA88 8007D288 50BF638C */  lw         $v1, %lo(D_8009BF50)($v1)
    /* 6DA8C 8007D28C 10004234 */  ori        $v0, $v0, 0x10
    /* 6DA90 8007D290 000062AC */  sw         $v0, 0x0($v1)
    /* 6DA94 8007D294 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6DA98 8007D298 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6DA9C 8007D29C 00028434 */  ori        $a0, $a0, (0x1000200 & 0xFFFF)
    /* 6DAA0 8007D2A0 000044AC */  sw         $a0, 0x0($v0)
  .L8007D2A4:
    /* 6DAA4 8007D2A4 21100000 */  addu       $v0, $zero, $zero
  .L8007D2A8:
    /* 6DAA8 8007D2A8 4400BF8F */  lw         $ra, 0x44($sp)
    /* 6DAAC 8007D2AC 4000B48F */  lw         $s4, 0x40($sp)
    /* 6DAB0 8007D2B0 3C00B38F */  lw         $s3, 0x3C($sp)
    /* 6DAB4 8007D2B4 3800B28F */  lw         $s2, 0x38($sp)
    /* 6DAB8 8007D2B8 3400B18F */  lw         $s1, 0x34($sp)
    /* 6DABC 8007D2BC 3000B08F */  lw         $s0, 0x30($sp)
    /* 6DAC0 8007D2C0 4800BD27 */  addiu      $sp, $sp, 0x48
    /* 6DAC4 8007D2C4 0800E003 */  jr         $ra
    /* 6DAC8 8007D2C8 00000000 */   nop
endlabel func_8007D048

nonmatching func_8007D2CC, 0x28

glabel func_8007D2CC
    /* 6DACC 8007D2CC 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6DAD0 8007D2D0 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6DAD4 8007D2D4 00000000 */  nop
    /* 6DAD8 8007D2D8 000044AC */  sw         $a0, 0x0($v0)
    /* 6DADC 8007D2DC 02160400 */  srl        $v0, $a0, 24
    /* 6DAE0 8007D2E0 0F80013C */  lui        $at, %hi(D_800F189C)
    /* 6DAE4 8007D2E4 21082200 */  addu       $at, $at, $v0
    /* 6DAE8 8007D2E8 9C1824A0 */  sb         $a0, %lo(D_800F189C)($at)
    /* 6DAEC 8007D2EC 0800E003 */  jr         $ra
    /* 6DAF0 8007D2F0 00000000 */   nop
endlabel func_8007D2CC

nonmatching func_8007D2F4, 0x14

glabel func_8007D2F4
    /* 6DAF4 8007D2F4 0F80013C */  lui        $at, %hi(D_800F189C)
    /* 6DAF8 8007D2F8 21082400 */  addu       $at, $at, $a0
    /* 6DAFC 8007D2FC 9C182290 */  lbu        $v0, %lo(D_800F189C)($at)
    /* 6DB00 8007D300 0800E003 */  jr         $ra
    /* 6DB04 8007D304 00000000 */   nop
endlabel func_8007D2F4

nonmatching func_8007D308, 0x50

glabel func_8007D308
    /* 6DB08 8007D308 F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 6DB0C 8007D30C FFFFA624 */  addiu      $a2, $a1, -0x1
    /* 6DB10 8007D310 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6DB14 8007D314 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6DB18 8007D318 0004023C */  lui        $v0, (0x4000000 >> 16)
    /* 6DB1C 8007D31C 000062AC */  sw         $v0, 0x0($v1)
    /* 6DB20 8007D320 0A00A010 */  beqz       $a1, .L8007D34C
    /* 6DB24 8007D324 21100000 */   addu      $v0, $zero, $zero
    /* 6DB28 8007D328 FFFF0524 */  addiu      $a1, $zero, -0x1
  .L8007D32C:
    /* 6DB2C 8007D32C 0000838C */  lw         $v1, 0x0($a0)
    /* 6DB30 8007D330 04008424 */  addiu      $a0, $a0, 0x4
    /* 6DB34 8007D334 0A80023C */  lui        $v0, %hi(D_8009BF44)
    /* 6DB38 8007D338 44BF428C */  lw         $v0, %lo(D_8009BF44)($v0)
    /* 6DB3C 8007D33C FFFFC624 */  addiu      $a2, $a2, -0x1
    /* 6DB40 8007D340 000043AC */  sw         $v1, 0x0($v0)
    /* 6DB44 8007D344 F9FFC514 */  bne        $a2, $a1, .L8007D32C
    /* 6DB48 8007D348 21100000 */   addu      $v0, $zero, $zero
  .L8007D34C:
    /* 6DB4C 8007D34C 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 6DB50 8007D350 0800E003 */  jr         $ra
    /* 6DB54 8007D354 00000000 */   nop
endlabel func_8007D308

nonmatching func_8007D358, 0x4C

glabel func_8007D358
    /* 6DB58 8007D358 0004033C */  lui        $v1, (0x4000002 >> 16)
    /* 6DB5C 8007D35C 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6DB60 8007D360 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6DB64 8007D364 02006334 */  ori        $v1, $v1, (0x4000002 & 0xFFFF)
    /* 6DB68 8007D368 000043AC */  sw         $v1, 0x0($v0)
    /* 6DB6C 8007D36C 0A80023C */  lui        $v0, %hi(D_8009BF4C)
    /* 6DB70 8007D370 4CBF428C */  lw         $v0, %lo(D_8009BF4C)($v0)
    /* 6DB74 8007D374 00000000 */  nop
    /* 6DB78 8007D378 000044AC */  sw         $a0, 0x0($v0)
    /* 6DB7C 8007D37C 0A80023C */  lui        $v0, %hi(D_8009BF50)
    /* 6DB80 8007D380 50BF428C */  lw         $v0, %lo(D_8009BF50)($v0)
    /* 6DB84 8007D384 0001033C */  lui        $v1, (0x1000401 >> 16)
    /* 6DB88 8007D388 000040AC */  sw         $zero, 0x0($v0)
    /* 6DB8C 8007D38C 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6DB90 8007D390 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6DB94 8007D394 01046334 */  ori        $v1, $v1, (0x1000401 & 0xFFFF)
    /* 6DB98 8007D398 000043AC */  sw         $v1, 0x0($v0)
    /* 6DB9C 8007D39C 0800E003 */  jr         $ra
    /* 6DBA0 8007D3A0 00000000 */   nop
endlabel func_8007D358

nonmatching func_8007D3A4, 0x30

glabel func_8007D3A4
    /* 6DBA4 8007D3A4 0010023C */  lui        $v0, (0x10000000 >> 16)
    /* 6DBA8 8007D3A8 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6DBAC 8007D3AC 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6DBB0 8007D3B0 25208200 */  or         $a0, $a0, $v0
    /* 6DBB4 8007D3B4 000064AC */  sw         $a0, 0x0($v1)
    /* 6DBB8 8007D3B8 0A80023C */  lui        $v0, %hi(D_8009BF44)
    /* 6DBBC 8007D3BC 44BF428C */  lw         $v0, %lo(D_8009BF44)($v0)
    /* 6DBC0 8007D3C0 FF00033C */  lui        $v1, (0xFFFFFF >> 16)
    /* 6DBC4 8007D3C4 0000428C */  lw         $v0, 0x0($v0)
    /* 6DBC8 8007D3C8 FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 6DBCC 8007D3CC 0800E003 */  jr         $ra
    /* 6DBD0 8007D3D0 24104300 */   and       $v0, $v0, $v1
endlabel func_8007D3A4

nonmatching func_8007D3D4, 0x24

glabel func_8007D3D4
    /* 6DBD4 8007D3D4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6DBD8 8007D3D8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6DBDC 8007D3DC 2138C000 */  addu       $a3, $a2, $zero
    /* 6DBE0 8007D3E0 FEF4010C */  jal        func_8007D3F8
    /* 6DBE4 8007D3E4 21300000 */   addu      $a2, $zero, $zero
    /* 6DBE8 8007D3E8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6DBEC 8007D3EC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6DBF0 8007D3F0 0800E003 */  jr         $ra
    /* 6DBF4 8007D3F4 00000000 */   nop
endlabel func_8007D3D4

nonmatching func_8007D3F8, 0x2E0

glabel func_8007D3F8
    /* 6DBF8 8007D3F8 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6DBFC 8007D3FC 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6DC00 8007D400 21988000 */  addu       $s3, $a0, $zero
    /* 6DC04 8007D404 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6DC08 8007D408 2180A000 */  addu       $s0, $a1, $zero
    /* 6DC0C 8007D40C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6DC10 8007D410 2188C000 */  addu       $s1, $a2, $zero
    /* 6DC14 8007D414 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6DC18 8007D418 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6DC1C 8007D41C 1AF7010C */  jal        func_8007DC68
    /* 6DC20 8007D420 2190E000 */   addu      $s2, $a3, $zero
    /* 6DC24 8007D424 11F50108 */  j          .L8007D444
    /* 6DC28 8007D428 00000000 */   nop
  .L8007D42C:
    /* 6DC2C 8007D42C 27F7010C */  jal        func_8007DC9C
    /* 6DC30 8007D430 00000000 */   nop
    /* 6DC34 8007D434 A0004014 */  bnez       $v0, .L8007D6B8
    /* 6DC38 8007D438 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 6DC3C 8007D43C B6F5010C */  jal        func_8007D6D8
    /* 6DC40 8007D440 00000000 */   nop
  .L8007D444:
    /* 6DC44 8007D444 0A80023C */  lui        $v0, %hi(D_8009BF78)
    /* 6DC48 8007D448 78BF428C */  lw         $v0, %lo(D_8009BF78)($v0)
    /* 6DC4C 8007D44C 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6DC50 8007D450 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6DC54 8007D454 01004224 */  addiu      $v0, $v0, 0x1
    /* 6DC58 8007D458 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6DC5C 8007D45C F3FF4310 */  beq        $v0, $v1, .L8007D42C
    /* 6DC60 8007D460 00000000 */   nop
    /* 6DC64 8007D464 0F0B020C */  jal        motion_make_table
    /* 6DC68 8007D468 21200000 */   addu      $a0, $zero, $zero
    /* 6DC6C 8007D46C 0A80033C */  lui        $v1, %hi(D_8009BE75)
    /* 6DC70 8007D470 75BE6390 */  lbu        $v1, %lo(D_8009BE75)($v1)
    /* 6DC74 8007D474 0A80013C */  lui        $at, %hi(D_8009BF80)
    /* 6DC78 8007D478 80BF22AC */  sw         $v0, %lo(D_8009BF80)($at)
    /* 6DC7C 8007D47C 01000224 */  addiu      $v0, $zero, 0x1
    /* 6DC80 8007D480 0A80013C */  lui        $at, %hi(D_8009BE7C)
    /* 6DC84 8007D484 7CBE22AC */  sw         $v0, %lo(D_8009BE7C)($at)
    /* 6DC88 8007D488 15006010 */  beqz       $v1, .L8007D4E0
    /* 6DC8C 8007D48C 00000000 */   nop
    /* 6DC90 8007D490 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DC94 8007D494 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DC98 8007D498 0A80023C */  lui        $v0, %hi(D_8009BF7C)
    /* 6DC9C 8007D49C 7CBF428C */  lw         $v0, %lo(D_8009BF7C)($v0)
    /* 6DCA0 8007D4A0 00000000 */  nop
    /* 6DCA4 8007D4A4 26006214 */  bne        $v1, $v0, .L8007D540
    /* 6DCA8 8007D4A8 00000000 */   nop
    /* 6DCAC 8007D4AC 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6DCB0 8007D4B0 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6DCB4 8007D4B4 00000000 */  nop
    /* 6DCB8 8007D4B8 0000428C */  lw         $v0, 0x0($v0)
    /* 6DCBC 8007D4BC 0001033C */  lui        $v1, (0x1000000 >> 16)
    /* 6DCC0 8007D4C0 24104300 */  and        $v0, $v0, $v1
    /* 6DCC4 8007D4C4 1E004014 */  bnez       $v0, .L8007D540
    /* 6DCC8 8007D4C8 00000000 */   nop
    /* 6DCCC 8007D4CC 0A80023C */  lui        $v0, %hi(D_8009BE80)
    /* 6DCD0 8007D4D0 80BE428C */  lw         $v0, %lo(D_8009BE80)($v0)
    /* 6DCD4 8007D4D4 00000000 */  nop
    /* 6DCD8 8007D4D8 19004014 */  bnez       $v0, .L8007D540
    /* 6DCDC 8007D4DC 00000000 */   nop
  .L8007D4E0:
    /* 6DCE0 8007D4E0 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6DCE4 8007D4E4 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6DCE8 8007D4E8 0004043C */  lui        $a0, (0x4000000 >> 16)
  .L8007D4EC:
    /* 6DCEC 8007D4EC 0000628C */  lw         $v0, 0x0($v1)
    /* 6DCF0 8007D4F0 00000000 */  nop
    /* 6DCF4 8007D4F4 24104400 */  and        $v0, $v0, $a0
    /* 6DCF8 8007D4F8 FCFF4010 */  beqz       $v0, .L8007D4EC
    /* 6DCFC 8007D4FC 00000000 */   nop
    /* 6DD00 8007D500 21200002 */  addu       $a0, $s0, $zero
    /* 6DD04 8007D504 09F86002 */  jalr       $s3
    /* 6DD08 8007D508 21284002 */   addu      $a1, $s2, $zero
    /* 6DD0C 8007D50C 0A80043C */  lui        $a0, %hi(D_8009BF80)
    /* 6DD10 8007D510 80BF848C */  lw         $a0, %lo(D_8009BF80)($a0)
    /* 6DD14 8007D514 0A80023C */  lui        $v0, %hi(D_8009BF68)
    /* 6DD18 8007D518 68BF4224 */  addiu      $v0, $v0, %lo(D_8009BF68)
    /* 6DD1C 8007D51C 000053AC */  sw         $s3, 0x0($v0)
    /* 6DD20 8007D520 0A80013C */  lui        $at, %hi(D_8009BF6C)
    /* 6DD24 8007D524 6CBF30AC */  sw         $s0, %lo(D_8009BF6C)($at)
    /* 6DD28 8007D528 0A80013C */  lui        $at, %hi(D_8009BF70)
    /* 6DD2C 8007D52C 70BF32AC */  sw         $s2, %lo(D_8009BF70)($at)
    /* 6DD30 8007D530 0F0B020C */  jal        motion_make_table
    /* 6DD34 8007D534 00000000 */   nop
    /* 6DD38 8007D538 AEF50108 */  j          .L8007D6B8
    /* 6DD3C 8007D53C 21100000 */   addu      $v0, $zero, $zero
  .L8007D540:
    /* 6DD40 8007D540 0880053C */  lui        $a1, %hi(func_8007D6D8)
    /* 6DD44 8007D544 D8D6A524 */  addiu      $a1, $a1, %lo(func_8007D6D8)
    /* 6DD48 8007D548 C80A020C */  jal        irq_AcknowledgeVblank
    /* 6DD4C 8007D54C 02000424 */   addiu     $a0, $zero, 0x2
    /* 6DD50 8007D550 2B002012 */  beqz       $s1, .L8007D600
    /* 6DD54 8007D554 21300000 */   addu      $a2, $zero, $zero
    /* 6DD58 8007D558 1080083C */  lui        $t0, %hi(D_8010368C)
    /* 6DD5C 8007D55C 8C360825 */  addiu      $t0, $t0, %lo(D_8010368C)
    /* 6DD60 8007D560 21380002 */  addu       $a3, $s0, $zero
    /* 6DD64 8007D564 21102002 */  addu       $v0, $s1, $zero
  .L8007D568:
    /* 6DD68 8007D568 02004104 */  bgez       $v0, .L8007D574
    /* 6DD6C 8007D56C 00000000 */   nop
    /* 6DD70 8007D570 03004224 */  addiu      $v0, $v0, 0x3
  .L8007D574:
    /* 6DD74 8007D574 83100200 */  sra        $v0, $v0, 2
    /* 6DD78 8007D578 2A10C200 */  slt        $v0, $a2, $v0
    /* 6DD7C 8007D57C 0E004010 */  beqz       $v0, .L8007D5B8
    /* 6DD80 8007D580 80200600 */   sll       $a0, $a2, 2
    /* 6DD84 8007D584 0000E58C */  lw         $a1, 0x0($a3)
    /* 6DD88 8007D588 0400E724 */  addiu      $a3, $a3, 0x4
    /* 6DD8C 8007D58C 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DD90 8007D590 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DD94 8007D594 0100C624 */  addiu      $a2, $a2, 0x1
    /* 6DD98 8007D598 40100300 */  sll        $v0, $v1, 1
    /* 6DD9C 8007D59C 21104300 */  addu       $v0, $v0, $v1
    /* 6DDA0 8007D5A0 40110200 */  sll        $v0, $v0, 5
    /* 6DDA4 8007D5A4 21104800 */  addu       $v0, $v0, $t0
    /* 6DDA8 8007D5A8 21208200 */  addu       $a0, $a0, $v0
    /* 6DDAC 8007D5AC 000085AC */  sw         $a1, 0x0($a0)
    /* 6DDB0 8007D5B0 5AF50108 */  j          .L8007D568
    /* 6DDB4 8007D5B4 21102002 */   addu      $v0, $s1, $zero
  .L8007D5B8:
    /* 6DDB8 8007D5B8 0A80023C */  lui        $v0, %hi(D_8009BF78)
    /* 6DDBC 8007D5BC 78BF428C */  lw         $v0, %lo(D_8009BF78)($v0)
    /* 6DDC0 8007D5C0 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DDC4 8007D5C4 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DDC8 8007D5C8 40200200 */  sll        $a0, $v0, 1
    /* 6DDCC 8007D5CC 21208200 */  addu       $a0, $a0, $v0
    /* 6DDD0 8007D5D0 40210400 */  sll        $a0, $a0, 5
    /* 6DDD4 8007D5D4 40100300 */  sll        $v0, $v1, 1
    /* 6DDD8 8007D5D8 21104300 */  addu       $v0, $v0, $v1
    /* 6DDDC 8007D5DC 40110200 */  sll        $v0, $v0, 5
    /* 6DDE0 8007D5E0 1080033C */  lui        $v1, %hi(D_8010368C)
    /* 6DDE4 8007D5E4 8C366324 */  addiu      $v1, $v1, %lo(D_8010368C)
    /* 6DDE8 8007D5E8 21104300 */  addu       $v0, $v0, $v1
    /* 6DDEC 8007D5EC 1080013C */  lui        $at, %hi(D_80103684)
    /* 6DDF0 8007D5F0 21082400 */  addu       $at, $at, $a0
    /* 6DDF4 8007D5F4 843622AC */  sw         $v0, %lo(D_80103684)($at)
    /* 6DDF8 8007D5F8 89F50108 */  j          .L8007D624
    /* 6DDFC 8007D5FC 00000000 */   nop
  .L8007D600:
    /* 6DE00 8007D600 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DE04 8007D604 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DE08 8007D608 00000000 */  nop
    /* 6DE0C 8007D60C 40100300 */  sll        $v0, $v1, 1
    /* 6DE10 8007D610 21104300 */  addu       $v0, $v0, $v1
    /* 6DE14 8007D614 40110200 */  sll        $v0, $v0, 5
    /* 6DE18 8007D618 1080013C */  lui        $at, %hi(D_80103684)
    /* 6DE1C 8007D61C 21082200 */  addu       $at, $at, $v0
    /* 6DE20 8007D620 843630AC */  sw         $s0, %lo(D_80103684)($at)
  .L8007D624:
    /* 6DE24 8007D624 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DE28 8007D628 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DE2C 8007D62C 00000000 */  nop
    /* 6DE30 8007D630 40100300 */  sll        $v0, $v1, 1
    /* 6DE34 8007D634 21104300 */  addu       $v0, $v0, $v1
    /* 6DE38 8007D638 40110200 */  sll        $v0, $v0, 5
    /* 6DE3C 8007D63C 1080013C */  lui        $at, %hi(D_80103688)
    /* 6DE40 8007D640 21082200 */  addu       $at, $at, $v0
    /* 6DE44 8007D644 883632AC */  sw         $s2, %lo(D_80103688)($at)
    /* 6DE48 8007D648 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DE4C 8007D64C 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DE50 8007D650 00000000 */  nop
    /* 6DE54 8007D654 40100300 */  sll        $v0, $v1, 1
    /* 6DE58 8007D658 21104300 */  addu       $v0, $v0, $v1
    /* 6DE5C 8007D65C 40110200 */  sll        $v0, $v0, 5
    /* 6DE60 8007D660 1080013C */  lui        $at, %hi(D_80103680)
    /* 6DE64 8007D664 21082200 */  addu       $at, $at, $v0
    /* 6DE68 8007D668 803633AC */  sw         $s3, %lo(D_80103680)($at)
    /* 6DE6C 8007D66C 0A80023C */  lui        $v0, %hi(D_8009BF78)
    /* 6DE70 8007D670 78BF428C */  lw         $v0, %lo(D_8009BF78)($v0)
    /* 6DE74 8007D674 0A80043C */  lui        $a0, %hi(D_8009BF80)
    /* 6DE78 8007D678 80BF848C */  lw         $a0, %lo(D_8009BF80)($a0)
    /* 6DE7C 8007D67C 01004224 */  addiu      $v0, $v0, 0x1
    /* 6DE80 8007D680 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6DE84 8007D684 0A80013C */  lui        $at, %hi(D_8009BF78)
    /* 6DE88 8007D688 78BF22AC */  sw         $v0, %lo(D_8009BF78)($at)
    /* 6DE8C 8007D68C 0F0B020C */  jal        motion_make_table
    /* 6DE90 8007D690 00000000 */   nop
    /* 6DE94 8007D694 B6F5010C */  jal        func_8007D6D8
    /* 6DE98 8007D698 00000000 */   nop
    /* 6DE9C 8007D69C 0A80023C */  lui        $v0, %hi(D_8009BF78)
    /* 6DEA0 8007D6A0 78BF428C */  lw         $v0, %lo(D_8009BF78)($v0)
    /* 6DEA4 8007D6A4 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6DEA8 8007D6A8 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6DEAC 8007D6AC 00000000 */  nop
    /* 6DEB0 8007D6B0 23104300 */  subu       $v0, $v0, $v1
    /* 6DEB4 8007D6B4 3F004230 */  andi       $v0, $v0, 0x3F
  .L8007D6B8:
    /* 6DEB8 8007D6B8 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6DEBC 8007D6BC 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6DEC0 8007D6C0 1800B28F */  lw         $s2, 0x18($sp)
    /* 6DEC4 8007D6C4 1400B18F */  lw         $s1, 0x14($sp)
    /* 6DEC8 8007D6C8 1000B08F */  lw         $s0, 0x10($sp)
    /* 6DECC 8007D6CC 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6DED0 8007D6D0 0800E003 */  jr         $ra
    /* 6DED4 8007D6D4 00000000 */   nop
endlabel func_8007D3F8

nonmatching func_8007D6D8, 0x2EC

glabel func_8007D6D8
    /* 6DED8 8007D6D8 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6DEDC 8007D6DC 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6DEE0 8007D6E0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6DEE4 8007D6E4 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6DEE8 8007D6E8 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6DEEC 8007D6EC 0000428C */  lw         $v0, 0x0($v0)
    /* 6DEF0 8007D6F0 0001103C */  lui        $s0, (0x1000000 >> 16)
    /* 6DEF4 8007D6F4 24105000 */  and        $v0, $v0, $s0
    /* 6DEF8 8007D6F8 AD004014 */  bnez       $v0, .L8007D9B0
    /* 6DEFC 8007D6FC 01000224 */   addiu     $v0, $zero, 0x1
    /* 6DF00 8007D700 0F0B020C */  jal        motion_make_table
    /* 6DF04 8007D704 21200000 */   addu      $a0, $zero, $zero
    /* 6DF08 8007D708 0A80043C */  lui        $a0, %hi(D_8009BF78)
    /* 6DF0C 8007D70C 78BF848C */  lw         $a0, %lo(D_8009BF78)($a0)
    /* 6DF10 8007D710 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6DF14 8007D714 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6DF18 8007D718 0A80013C */  lui        $at, %hi(D_8009BF84)
    /* 6DF1C 8007D71C 84BF22AC */  sw         $v0, %lo(D_8009BF84)($at)
    /* 6DF20 8007D720 7B008310 */  beq        $a0, $v1, .L8007D910
    /* 6DF24 8007D724 00000000 */   nop
    /* 6DF28 8007D728 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6DF2C 8007D72C 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6DF30 8007D730 00000000 */  nop
    /* 6DF34 8007D734 0000428C */  lw         $v0, 0x0($v0)
    /* 6DF38 8007D738 00000000 */  nop
    /* 6DF3C 8007D73C 24105000 */  and        $v0, $v0, $s0
    /* 6DF40 8007D740 73004014 */  bnez       $v0, .L8007D910
    /* 6DF44 8007D744 00000000 */   nop
    /* 6DF48 8007D748 0A80103C */  lui        $s0, %hi(D_8009BF68)
    /* 6DF4C 8007D74C 68BF1026 */  addiu      $s0, $s0, %lo(D_8009BF68)
  .L8007D750:
    /* 6DF50 8007D750 0A80023C */  lui        $v0, %hi(D_8009BF7C)
    /* 6DF54 8007D754 7CBF428C */  lw         $v0, %lo(D_8009BF7C)($v0)
    /* 6DF58 8007D758 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DF5C 8007D75C 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DF60 8007D760 01004224 */  addiu      $v0, $v0, 0x1
    /* 6DF64 8007D764 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6DF68 8007D768 08004314 */  bne        $v0, $v1, .L8007D78C
    /* 6DF6C 8007D76C 00000000 */   nop
    /* 6DF70 8007D770 0A80023C */  lui        $v0, %hi(D_8009BE80)
    /* 6DF74 8007D774 80BE428C */  lw         $v0, %lo(D_8009BE80)($v0)
    /* 6DF78 8007D778 00000000 */  nop
    /* 6DF7C 8007D77C 03004014 */  bnez       $v0, .L8007D78C
    /* 6DF80 8007D780 02000424 */   addiu     $a0, $zero, 0x2
    /* 6DF84 8007D784 C80A020C */  jal        irq_AcknowledgeVblank
    /* 6DF88 8007D788 21280000 */   addu      $a1, $zero, $zero
  .L8007D78C:
    /* 6DF8C 8007D78C 0A80043C */  lui        $a0, %hi(D_8009BF48)
    /* 6DF90 8007D790 48BF848C */  lw         $a0, %lo(D_8009BF48)($a0)
    /* 6DF94 8007D794 00000000 */  nop
    /* 6DF98 8007D798 0000828C */  lw         $v0, 0x0($a0)
    /* 6DF9C 8007D79C 0004033C */  lui        $v1, (0x4000000 >> 16)
    /* 6DFA0 8007D7A0 24104300 */  and        $v0, $v0, $v1
    /* 6DFA4 8007D7A4 07004014 */  bnez       $v0, .L8007D7C4
    /* 6DFA8 8007D7A8 21188000 */   addu      $v1, $a0, $zero
    /* 6DFAC 8007D7AC 0004043C */  lui        $a0, (0x4000000 >> 16)
  .L8007D7B0:
    /* 6DFB0 8007D7B0 0000628C */  lw         $v0, 0x0($v1)
    /* 6DFB4 8007D7B4 00000000 */  nop
    /* 6DFB8 8007D7B8 24104400 */  and        $v0, $v0, $a0
    /* 6DFBC 8007D7BC FCFF4010 */  beqz       $v0, .L8007D7B0
    /* 6DFC0 8007D7C0 00000000 */   nop
  .L8007D7C4:
    /* 6DFC4 8007D7C4 0A80053C */  lui        $a1, %hi(D_8009BF7C)
    /* 6DFC8 8007D7C8 7CBFA58C */  lw         $a1, %lo(D_8009BF7C)($a1)
    /* 6DFCC 8007D7CC 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6DFD0 8007D7D0 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6DFD4 8007D7D4 00000000 */  nop
    /* 6DFD8 8007D7D8 40100300 */  sll        $v0, $v1, 1
    /* 6DFDC 8007D7DC 21104300 */  addu       $v0, $v0, $v1
    /* 6DFE0 8007D7E0 40110200 */  sll        $v0, $v0, 5
    /* 6DFE4 8007D7E4 40180500 */  sll        $v1, $a1, 1
    /* 6DFE8 8007D7E8 21186500 */  addu       $v1, $v1, $a1
    /* 6DFEC 8007D7EC 1080013C */  lui        $at, %hi(D_80103684)
    /* 6DFF0 8007D7F0 21082200 */  addu       $at, $at, $v0
    /* 6DFF4 8007D7F4 8436248C */  lw         $a0, %lo(D_80103684)($at)
    /* 6DFF8 8007D7F8 0A80053C */  lui        $a1, %hi(D_8009BF7C)
    /* 6DFFC 8007D7FC 7CBFA58C */  lw         $a1, %lo(D_8009BF7C)($a1)
    /* 6E000 8007D800 40190300 */  sll        $v1, $v1, 5
    /* 6E004 8007D804 40100500 */  sll        $v0, $a1, 1
    /* 6E008 8007D808 21104500 */  addu       $v0, $v0, $a1
    /* 6E00C 8007D80C 40110200 */  sll        $v0, $v0, 5
    /* 6E010 8007D810 1080013C */  lui        $at, %hi(D_80103688)
    /* 6E014 8007D814 21082200 */  addu       $at, $at, $v0
    /* 6E018 8007D818 8836258C */  lw         $a1, %lo(D_80103688)($at)
    /* 6E01C 8007D81C 1080013C */  lui        $at, %hi(D_80103680)
    /* 6E020 8007D820 21082300 */  addu       $at, $at, $v1
    /* 6E024 8007D824 8036228C */  lw         $v0, %lo(D_80103680)($at)
    /* 6E028 8007D828 00000000 */  nop
    /* 6E02C 8007D82C 09F84000 */  jalr       $v0
    /* 6E030 8007D830 00000000 */   nop
    /* 6E034 8007D834 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6E038 8007D838 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6E03C 8007D83C 00000000 */  nop
    /* 6E040 8007D840 40100300 */  sll        $v0, $v1, 1
    /* 6E044 8007D844 21104300 */  addu       $v0, $v0, $v1
    /* 6E048 8007D848 40110200 */  sll        $v0, $v0, 5
    /* 6E04C 8007D84C 1080013C */  lui        $at, %hi(D_80103680)
    /* 6E050 8007D850 21082200 */  addu       $at, $at, $v0
    /* 6E054 8007D854 8036228C */  lw         $v0, %lo(D_80103680)($at)
    /* 6E058 8007D858 00000000 */  nop
    /* 6E05C 8007D85C 000002AE */  sw         $v0, 0x0($s0)
    /* 6E060 8007D860 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6E064 8007D864 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6E068 8007D868 00000000 */  nop
    /* 6E06C 8007D86C 40100300 */  sll        $v0, $v1, 1
    /* 6E070 8007D870 21104300 */  addu       $v0, $v0, $v1
    /* 6E074 8007D874 40110200 */  sll        $v0, $v0, 5
    /* 6E078 8007D878 1080013C */  lui        $at, %hi(D_80103684)
    /* 6E07C 8007D87C 21082200 */  addu       $at, $at, $v0
    /* 6E080 8007D880 8436228C */  lw         $v0, %lo(D_80103684)($at)
    /* 6E084 8007D884 0A80013C */  lui        $at, %hi(D_8009BF6C)
    /* 6E088 8007D888 6CBF22AC */  sw         $v0, %lo(D_8009BF6C)($at)
    /* 6E08C 8007D88C 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6E090 8007D890 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6E094 8007D894 00000000 */  nop
    /* 6E098 8007D898 40100300 */  sll        $v0, $v1, 1
    /* 6E09C 8007D89C 21104300 */  addu       $v0, $v0, $v1
    /* 6E0A0 8007D8A0 40110200 */  sll        $v0, $v0, 5
    /* 6E0A4 8007D8A4 1080013C */  lui        $at, %hi(D_80103688)
    /* 6E0A8 8007D8A8 21082200 */  addu       $at, $at, $v0
    /* 6E0AC 8007D8AC 8836228C */  lw         $v0, %lo(D_80103688)($at)
    /* 6E0B0 8007D8B0 0A80013C */  lui        $at, %hi(D_8009BF70)
    /* 6E0B4 8007D8B4 70BF22AC */  sw         $v0, %lo(D_8009BF70)($at)
    /* 6E0B8 8007D8B8 0A80023C */  lui        $v0, %hi(D_8009BF7C)
    /* 6E0BC 8007D8BC 7CBF428C */  lw         $v0, %lo(D_8009BF7C)($v0)
    /* 6E0C0 8007D8C0 00000000 */  nop
    /* 6E0C4 8007D8C4 01004224 */  addiu      $v0, $v0, 0x1
    /* 6E0C8 8007D8C8 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6E0CC 8007D8CC 0A80013C */  lui        $at, %hi(D_8009BF7C)
    /* 6E0D0 8007D8D0 7CBF22AC */  sw         $v0, %lo(D_8009BF7C)($at)
    /* 6E0D4 8007D8D4 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6E0D8 8007D8D8 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6E0DC 8007D8DC 0A80023C */  lui        $v0, %hi(D_8009BF7C)
    /* 6E0E0 8007D8E0 7CBF428C */  lw         $v0, %lo(D_8009BF7C)($v0)
    /* 6E0E4 8007D8E4 00000000 */  nop
    /* 6E0E8 8007D8E8 09006210 */  beq        $v1, $v0, .L8007D910
    /* 6E0EC 8007D8EC 00000000 */   nop
    /* 6E0F0 8007D8F0 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6E0F4 8007D8F4 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6E0F8 8007D8F8 00000000 */  nop
    /* 6E0FC 8007D8FC 0000428C */  lw         $v0, 0x0($v0)
    /* 6E100 8007D900 0001033C */  lui        $v1, (0x1000000 >> 16)
    /* 6E104 8007D904 24104300 */  and        $v0, $v0, $v1
    /* 6E108 8007D908 91FF4010 */  beqz       $v0, .L8007D750
    /* 6E10C 8007D90C 00000000 */   nop
  .L8007D910:
    /* 6E110 8007D910 0A80043C */  lui        $a0, %hi(D_8009BF84)
    /* 6E114 8007D914 84BF848C */  lw         $a0, %lo(D_8009BF84)($a0)
    /* 6E118 8007D918 0F0B020C */  jal        motion_make_table
    /* 6E11C 8007D91C 00000000 */   nop
    /* 6E120 8007D920 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6E124 8007D924 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6E128 8007D928 0A80023C */  lui        $v0, %hi(D_8009BF7C)
    /* 6E12C 8007D92C 7CBF428C */  lw         $v0, %lo(D_8009BF7C)($v0)
    /* 6E130 8007D930 00000000 */  nop
    /* 6E134 8007D934 17006214 */  bne        $v1, $v0, .L8007D994
    /* 6E138 8007D938 00000000 */   nop
    /* 6E13C 8007D93C 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6E140 8007D940 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6E144 8007D944 00000000 */  nop
    /* 6E148 8007D948 0000428C */  lw         $v0, 0x0($v0)
    /* 6E14C 8007D94C 0001033C */  lui        $v1, (0x1000000 >> 16)
    /* 6E150 8007D950 24104300 */  and        $v0, $v0, $v1
    /* 6E154 8007D954 0F004014 */  bnez       $v0, .L8007D994
    /* 6E158 8007D958 00000000 */   nop
    /* 6E15C 8007D95C 0A80033C */  lui        $v1, %hi(D_8009BE7C)
    /* 6E160 8007D960 7CBE6324 */  addiu      $v1, $v1, %lo(D_8009BE7C)
    /* 6E164 8007D964 0000628C */  lw         $v0, 0x0($v1)
    /* 6E168 8007D968 00000000 */  nop
    /* 6E16C 8007D96C 09004010 */  beqz       $v0, .L8007D994
    /* 6E170 8007D970 00000000 */   nop
    /* 6E174 8007D974 0A80023C */  lui        $v0, %hi(D_8009BE80)
    /* 6E178 8007D978 80BE428C */  lw         $v0, %lo(D_8009BE80)($v0)
    /* 6E17C 8007D97C 00000000 */  nop
    /* 6E180 8007D980 04004010 */  beqz       $v0, .L8007D994
    /* 6E184 8007D984 00000000 */   nop
    /* 6E188 8007D988 000060AC */  sw         $zero, 0x0($v1)
    /* 6E18C 8007D98C 09F84000 */  jalr       $v0
    /* 6E190 8007D990 00000000 */   nop
  .L8007D994:
    /* 6E194 8007D994 0A80023C */  lui        $v0, %hi(D_8009BF78)
    /* 6E198 8007D998 78BF428C */  lw         $v0, %lo(D_8009BF78)($v0)
    /* 6E19C 8007D99C 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6E1A0 8007D9A0 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6E1A4 8007D9A4 00000000 */  nop
    /* 6E1A8 8007D9A8 23104300 */  subu       $v0, $v0, $v1
    /* 6E1AC 8007D9AC 3F004230 */  andi       $v0, $v0, 0x3F
  .L8007D9B0:
    /* 6E1B0 8007D9B0 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6E1B4 8007D9B4 1000B08F */  lw         $s0, 0x10($sp)
    /* 6E1B8 8007D9B8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6E1BC 8007D9BC 0800E003 */  jr         $ra
    /* 6E1C0 8007D9C0 00000000 */   nop
endlabel func_8007D6D8

nonmatching func_8007D9C4, 0x15C

glabel func_8007D9C4
    /* 6E1C4 8007D9C4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6E1C8 8007D9C8 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6E1CC 8007D9CC 21808000 */  addu       $s0, $a0, $zero
    /* 6E1D0 8007D9D0 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6E1D4 8007D9D4 0F0B020C */  jal        motion_make_table
    /* 6E1D8 8007D9D8 21200000 */   addu      $a0, $zero, $zero
    /* 6E1DC 8007D9DC 0A80013C */  lui        $at, %hi(D_8009BF7C)
    /* 6E1E0 8007D9E0 7CBF20AC */  sw         $zero, %lo(D_8009BF7C)($at)
    /* 6E1E4 8007D9E4 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6E1E8 8007D9E8 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6E1EC 8007D9EC 0A80013C */  lui        $at, %hi(D_8009BF88)
    /* 6E1F0 8007D9F0 88BF22AC */  sw         $v0, %lo(D_8009BF88)($at)
    /* 6E1F4 8007D9F4 01000224 */  addiu      $v0, $zero, 0x1
    /* 6E1F8 8007D9F8 0A80013C */  lui        $at, %hi(D_8009BF78)
    /* 6E1FC 8007D9FC 78BF23AC */  sw         $v1, %lo(D_8009BF78)($at)
    /* 6E200 8007DA00 07000332 */  andi       $v1, $s0, 0x7
    /* 6E204 8007DA04 25006210 */  beq        $v1, $v0, .L8007DA9C
    /* 6E208 8007DA08 02006228 */   slti      $v0, $v1, 0x2
    /* 6E20C 8007DA0C 05004010 */  beqz       $v0, .L8007DA24
    /* 6E210 8007DA10 00000000 */   nop
    /* 6E214 8007DA14 08006010 */  beqz       $v1, .L8007DA38
    /* 6E218 8007DA18 00000000 */   nop
    /* 6E21C 8007DA1C BAF60108 */  j          .L8007DAE8
    /* 6E220 8007DA20 00000000 */   nop
  .L8007DA24:
    /* 6E224 8007DA24 03000224 */  addiu      $v0, $zero, 0x3
    /* 6E228 8007DA28 1C006210 */  beq        $v1, $v0, .L8007DA9C
    /* 6E22C 8007DA2C 05000224 */   addiu     $v0, $zero, 0x5
    /* 6E230 8007DA30 2D006214 */  bne        $v1, $v0, .L8007DAE8
    /* 6E234 8007DA34 00000000 */   nop
  .L8007DA38:
    /* 6E238 8007DA38 0A80033C */  lui        $v1, %hi(D_8009BF54)
    /* 6E23C 8007DA3C 54BF638C */  lw         $v1, %lo(D_8009BF54)($v1)
    /* 6E240 8007DA40 01040224 */  addiu      $v0, $zero, 0x401
    /* 6E244 8007DA44 000062AC */  sw         $v0, 0x0($v1)
    /* 6E248 8007DA48 0A80033C */  lui        $v1, %hi(D_8009BF64)
    /* 6E24C 8007DA4C 64BF638C */  lw         $v1, %lo(D_8009BF64)($v1)
    /* 6E250 8007DA50 0F80043C */  lui        $a0, %hi(D_800F189C)
    /* 6E254 8007DA54 9C188424 */  addiu      $a0, $a0, %lo(D_800F189C)
    /* 6E258 8007DA58 0000628C */  lw         $v0, 0x0($v1)
    /* 6E25C 8007DA5C 21280000 */  addu       $a1, $zero, $zero
    /* 6E260 8007DA60 00084234 */  ori        $v0, $v0, 0x800
    /* 6E264 8007DA64 000062AC */  sw         $v0, 0x0($v1)
    /* 6E268 8007DA68 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6E26C 8007DA6C 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6E270 8007DA70 00010624 */  addiu      $a2, $zero, 0x100
    /* 6E274 8007DA74 000040AC */  sw         $zero, 0x0($v0)
    /* 6E278 8007DA78 B9F7010C */  jal        func_8007DEE4
    /* 6E27C 8007DA7C 00000000 */   nop
    /* 6E280 8007DA80 1080043C */  lui        $a0, %hi(D_80103680)
    /* 6E284 8007DA84 80368424 */  addiu      $a0, $a0, %lo(D_80103680)
    /* 6E288 8007DA88 21280000 */  addu       $a1, $zero, $zero
    /* 6E28C 8007DA8C B9F7010C */  jal        func_8007DEE4
    /* 6E290 8007DA90 00180624 */   addiu     $a2, $zero, 0x1800
    /* 6E294 8007DA94 BAF60108 */  j          .L8007DAE8
    /* 6E298 8007DA98 00000000 */   nop
  .L8007DA9C:
    /* 6E29C 8007DA9C 0A80033C */  lui        $v1, %hi(D_8009BF54)
    /* 6E2A0 8007DAA0 54BF638C */  lw         $v1, %lo(D_8009BF54)($v1)
    /* 6E2A4 8007DAA4 01040224 */  addiu      $v0, $zero, 0x401
    /* 6E2A8 8007DAA8 000062AC */  sw         $v0, 0x0($v1)
    /* 6E2AC 8007DAAC 0A80033C */  lui        $v1, %hi(D_8009BF64)
    /* 6E2B0 8007DAB0 64BF638C */  lw         $v1, %lo(D_8009BF64)($v1)
    /* 6E2B4 8007DAB4 00000000 */  nop
    /* 6E2B8 8007DAB8 0000628C */  lw         $v0, 0x0($v1)
    /* 6E2BC 8007DABC 00000000 */  nop
    /* 6E2C0 8007DAC0 00084234 */  ori        $v0, $v0, 0x800
    /* 6E2C4 8007DAC4 000062AC */  sw         $v0, 0x0($v1)
    /* 6E2C8 8007DAC8 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E2CC 8007DACC 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E2D0 8007DAD0 0002023C */  lui        $v0, (0x2000000 >> 16)
    /* 6E2D4 8007DAD4 000062AC */  sw         $v0, 0x0($v1)
    /* 6E2D8 8007DAD8 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E2DC 8007DADC 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E2E0 8007DAE0 0001023C */  lui        $v0, (0x1000000 >> 16)
    /* 6E2E4 8007DAE4 000062AC */  sw         $v0, 0x0($v1)
  .L8007DAE8:
    /* 6E2E8 8007DAE8 0A80043C */  lui        $a0, %hi(D_8009BF88)
    /* 6E2EC 8007DAEC 88BF848C */  lw         $a0, %lo(D_8009BF88)($a0)
    /* 6E2F0 8007DAF0 0F0B020C */  jal        motion_make_table
    /* 6E2F4 8007DAF4 00000000 */   nop
    /* 6E2F8 8007DAF8 07000232 */  andi       $v0, $s0, 0x7
    /* 6E2FC 8007DAFC 03004014 */  bnez       $v0, .L8007DB0C
    /* 6E300 8007DB00 21100000 */   addu      $v0, $zero, $zero
    /* 6E304 8007DB04 82F7010C */  jal        func_8007DE08
    /* 6E308 8007DB08 21200002 */   addu      $a0, $s0, $zero
  .L8007DB0C:
    /* 6E30C 8007DB0C 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6E310 8007DB10 1000B08F */  lw         $s0, 0x10($sp)
    /* 6E314 8007DB14 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6E318 8007DB18 0800E003 */  jr         $ra
    /* 6E31C 8007DB1C 00000000 */   nop
endlabel func_8007D9C4

nonmatching func_8007DB20, 0x148

glabel func_8007DB20
    /* 6E320 8007DB20 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6E324 8007DB24 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6E328 8007DB28 2A008014 */  bnez       $a0, .L8007DBD4
    /* 6E32C 8007DB2C 1000B0AF */   sw        $s0, 0x10($sp)
    /* 6E330 8007DB30 1AF7010C */  jal        func_8007DC68
    /* 6E334 8007DB34 00000000 */   nop
    /* 6E338 8007DB38 D6F60108 */  j          .L8007DB58
    /* 6E33C 8007DB3C 00000000 */   nop
  .L8007DB40:
    /* 6E340 8007DB40 B6F5010C */  jal        func_8007D6D8
    /* 6E344 8007DB44 00000000 */   nop
    /* 6E348 8007DB48 27F7010C */  jal        func_8007DC9C
    /* 6E34C 8007DB4C 00000000 */   nop
    /* 6E350 8007DB50 40004014 */  bnez       $v0, .L8007DC54
    /* 6E354 8007DB54 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8007DB58:
    /* 6E358 8007DB58 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6E35C 8007DB5C 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6E360 8007DB60 0A80023C */  lui        $v0, %hi(D_8009BF7C)
    /* 6E364 8007DB64 7CBF428C */  lw         $v0, %lo(D_8009BF7C)($v0)
    /* 6E368 8007DB68 00000000 */  nop
    /* 6E36C 8007DB6C 07006210 */  beq        $v1, $v0, .L8007DB8C
    /* 6E370 8007DB70 00000000 */   nop
    /* 6E374 8007DB74 D0F60108 */  j          .L8007DB40
    /* 6E378 8007DB78 00000000 */   nop
  .L8007DB7C:
    /* 6E37C 8007DB7C 27F7010C */  jal        func_8007DC9C
    /* 6E380 8007DB80 00000000 */   nop
    /* 6E384 8007DB84 33004014 */  bnez       $v0, .L8007DC54
    /* 6E388 8007DB88 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8007DB8C:
    /* 6E38C 8007DB8C 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6E390 8007DB90 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6E394 8007DB94 00000000 */  nop
    /* 6E398 8007DB98 0000428C */  lw         $v0, 0x0($v0)
    /* 6E39C 8007DB9C 0001033C */  lui        $v1, (0x1000000 >> 16)
    /* 6E3A0 8007DBA0 24104300 */  and        $v0, $v0, $v1
    /* 6E3A4 8007DBA4 F5FF4014 */  bnez       $v0, .L8007DB7C
    /* 6E3A8 8007DBA8 00000000 */   nop
    /* 6E3AC 8007DBAC 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6E3B0 8007DBB0 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6E3B4 8007DBB4 00000000 */  nop
    /* 6E3B8 8007DBB8 0000428C */  lw         $v0, 0x0($v0)
    /* 6E3BC 8007DBBC 0004033C */  lui        $v1, (0x4000000 >> 16)
    /* 6E3C0 8007DBC0 24104300 */  and        $v0, $v0, $v1
    /* 6E3C4 8007DBC4 EDFF4010 */  beqz       $v0, .L8007DB7C
    /* 6E3C8 8007DBC8 21100000 */   addu      $v0, $zero, $zero
    /* 6E3CC 8007DBCC 15F70108 */  j          .L8007DC54
    /* 6E3D0 8007DBD0 00000000 */   nop
  .L8007DBD4:
    /* 6E3D4 8007DBD4 0A80023C */  lui        $v0, %hi(D_8009BF78)
    /* 6E3D8 8007DBD8 78BF428C */  lw         $v0, %lo(D_8009BF78)($v0)
    /* 6E3DC 8007DBDC 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6E3E0 8007DBE0 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6E3E4 8007DBE4 00000000 */  nop
    /* 6E3E8 8007DBE8 23104300 */  subu       $v0, $v0, $v1
    /* 6E3EC 8007DBEC 3F005030 */  andi       $s0, $v0, 0x3F
    /* 6E3F0 8007DBF0 03000012 */  beqz       $s0, .L8007DC00
    /* 6E3F4 8007DBF4 00000000 */   nop
    /* 6E3F8 8007DBF8 B6F5010C */  jal        func_8007D6D8
    /* 6E3FC 8007DBFC 00000000 */   nop
  .L8007DC00:
    /* 6E400 8007DC00 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6E404 8007DC04 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6E408 8007DC08 00000000 */  nop
    /* 6E40C 8007DC0C 0000428C */  lw         $v0, 0x0($v0)
    /* 6E410 8007DC10 0001033C */  lui        $v1, (0x1000000 >> 16)
    /* 6E414 8007DC14 24104300 */  and        $v0, $v0, $v1
    /* 6E418 8007DC18 09004014 */  bnez       $v0, .L8007DC40
    /* 6E41C 8007DC1C 00000000 */   nop
    /* 6E420 8007DC20 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6E424 8007DC24 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6E428 8007DC28 00000000 */  nop
    /* 6E42C 8007DC2C 0000428C */  lw         $v0, 0x0($v0)
    /* 6E430 8007DC30 0004033C */  lui        $v1, (0x4000000 >> 16)
    /* 6E434 8007DC34 24104300 */  and        $v0, $v0, $v1
    /* 6E438 8007DC38 05004014 */  bnez       $v0, .L8007DC50
    /* 6E43C 8007DC3C 00000000 */   nop
  .L8007DC40:
    /* 6E440 8007DC40 04000016 */  bnez       $s0, .L8007DC54
    /* 6E444 8007DC44 21100002 */   addu      $v0, $s0, $zero
    /* 6E448 8007DC48 15F70108 */  j          .L8007DC54
    /* 6E44C 8007DC4C 01000224 */   addiu     $v0, $zero, 0x1
  .L8007DC50:
    /* 6E450 8007DC50 21100002 */  addu       $v0, $s0, $zero
  .L8007DC54:
    /* 6E454 8007DC54 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6E458 8007DC58 1000B08F */  lw         $s0, 0x10($sp)
    /* 6E45C 8007DC5C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6E460 8007DC60 0800E003 */  jr         $ra
    /* 6E464 8007DC64 00000000 */   nop
endlabel func_8007DB20

nonmatching func_8007DC68, 0x34

glabel func_8007DC68
    /* 6E468 8007DC68 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6E46C 8007DC6C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6E470 8007DC70 330A020C */  jal        sys_VSync
    /* 6E474 8007DC74 FFFF0424 */   addiu     $a0, $zero, -0x1
    /* 6E478 8007DC78 F0004224 */  addiu      $v0, $v0, 0xF0
    /* 6E47C 8007DC7C 0A80013C */  lui        $at, %hi(D_8009BF8C)
    /* 6E480 8007DC80 8CBF22AC */  sw         $v0, %lo(D_8009BF8C)($at)
    /* 6E484 8007DC84 0A80013C */  lui        $at, %hi(D_8009BF90)
    /* 6E488 8007DC88 90BF20AC */  sw         $zero, %lo(D_8009BF90)($at)
    /* 6E48C 8007DC8C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6E490 8007DC90 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6E494 8007DC94 0800E003 */  jr         $ra
    /* 6E498 8007DC98 00000000 */   nop
endlabel func_8007DC68

nonmatching func_8007DC9C, 0x16C

glabel func_8007DC9C
    /* 6E49C 8007DC9C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6E4A0 8007DCA0 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6E4A4 8007DCA4 330A020C */  jal        sys_VSync
    /* 6E4A8 8007DCA8 FFFF0424 */   addiu     $a0, $zero, -0x1
    /* 6E4AC 8007DCAC 0A80033C */  lui        $v1, %hi(D_8009BF8C)
    /* 6E4B0 8007DCB0 8CBF638C */  lw         $v1, %lo(D_8009BF8C)($v1)
    /* 6E4B4 8007DCB4 00000000 */  nop
    /* 6E4B8 8007DCB8 2A186200 */  slt        $v1, $v1, $v0
    /* 6E4BC 8007DCBC 0B006014 */  bnez       $v1, .L8007DCEC
    /* 6E4C0 8007DCC0 00000000 */   nop
    /* 6E4C4 8007DCC4 0A80033C */  lui        $v1, %hi(D_8009BF90)
    /* 6E4C8 8007DCC8 90BF638C */  lw         $v1, %lo(D_8009BF90)($v1)
    /* 6E4CC 8007DCCC 00000000 */  nop
    /* 6E4D0 8007DCD0 01006224 */  addiu      $v0, $v1, 0x1
    /* 6E4D4 8007DCD4 0A80013C */  lui        $at, %hi(D_8009BF90)
    /* 6E4D8 8007DCD8 90BF22AC */  sw         $v0, %lo(D_8009BF90)($at)
    /* 6E4DC 8007DCDC 0F00023C */  lui        $v0, (0xF0000 >> 16)
    /* 6E4E0 8007DCE0 2A104300 */  slt        $v0, $v0, $v1
    /* 6E4E4 8007DCE4 43004010 */  beqz       $v0, .L8007DDF4
    /* 6E4E8 8007DCE8 00000000 */   nop
  .L8007DCEC:
    /* 6E4EC 8007DCEC 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E4F0 8007DCF0 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E4F4 8007DCF4 0180043C */  lui        $a0, %hi(D_80016010)
    /* 6E4F8 8007DCF8 10608424 */  addiu      $a0, $a0, %lo(D_80016010)
    /* 6E4FC 8007DCFC 0000628C */  lw         $v0, 0x0($v1)
    /* 6E500 8007DD00 0A80053C */  lui        $a1, %hi(D_8009BF78)
    /* 6E504 8007DD04 78BFA58C */  lw         $a1, %lo(D_8009BF78)($a1)
    /* 6E508 8007DD08 0A80023C */  lui        $v0, %hi(D_8009BF4C)
    /* 6E50C 8007DD0C 4CBF428C */  lw         $v0, %lo(D_8009BF4C)($v0)
    /* 6E510 8007DD10 0A80083C */  lui        $t0, %hi(D_8009BF7C)
    /* 6E514 8007DD14 7CBF088D */  lw         $t0, %lo(D_8009BF7C)($t0)
    /* 6E518 8007DD18 0000428C */  lw         $v0, 0x0($v0)
    /* 6E51C 8007DD1C 2328A800 */  subu       $a1, $a1, $t0
    /* 6E520 8007DD20 1000A2AF */  sw         $v0, 0x10($sp)
    /* 6E524 8007DD24 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6E528 8007DD28 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6E52C 8007DD2C 0000668C */  lw         $a2, 0x0($v1)
    /* 6E530 8007DD30 0000478C */  lw         $a3, 0x0($v0)
    /* 6E534 8007DD34 82E4010C */  jal        debug_printf
    /* 6E538 8007DD38 3F00A530 */   andi      $a1, $a1, 0x3F
    /* 6E53C 8007DD3C 0A80023C */  lui        $v0, %hi(D_8009BF68)
    /* 6E540 8007DD40 68BF4224 */  addiu      $v0, $v0, %lo(D_8009BF68)
    /* 6E544 8007DD44 0000458C */  lw         $a1, 0x0($v0)
    /* 6E548 8007DD48 0A80063C */  lui        $a2, %hi(D_8009BF6C)
    /* 6E54C 8007DD4C 6CBFC68C */  lw         $a2, %lo(D_8009BF6C)($a2)
    /* 6E550 8007DD50 0A80073C */  lui        $a3, %hi(D_8009BF70)
    /* 6E554 8007DD54 70BFE78C */  lw         $a3, %lo(D_8009BF70)($a3)
    /* 6E558 8007DD58 0180043C */  lui        $a0, %hi(D_80016044)
    /* 6E55C 8007DD5C 44608424 */  addiu      $a0, $a0, %lo(D_80016044)
    /* 6E560 8007DD60 82E4010C */  jal        debug_printf
    /* 6E564 8007DD64 00000000 */   nop
    /* 6E568 8007DD68 0F0B020C */  jal        motion_make_table
    /* 6E56C 8007DD6C 21200000 */   addu      $a0, $zero, $zero
    /* 6E570 8007DD70 0A80013C */  lui        $at, %hi(D_8009BF7C)
    /* 6E574 8007DD74 7CBF20AC */  sw         $zero, %lo(D_8009BF7C)($at)
    /* 6E578 8007DD78 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6E57C 8007DD7C 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6E580 8007DD80 0A80013C */  lui        $at, %hi(D_8009BF88)
    /* 6E584 8007DD84 88BF22AC */  sw         $v0, %lo(D_8009BF88)($at)
    /* 6E588 8007DD88 0A80013C */  lui        $at, %hi(D_8009BF78)
    /* 6E58C 8007DD8C 78BF23AC */  sw         $v1, %lo(D_8009BF78)($at)
    /* 6E590 8007DD90 0A80033C */  lui        $v1, %hi(D_8009BF54)
    /* 6E594 8007DD94 54BF638C */  lw         $v1, %lo(D_8009BF54)($v1)
    /* 6E598 8007DD98 01040224 */  addiu      $v0, $zero, 0x401
    /* 6E59C 8007DD9C 000062AC */  sw         $v0, 0x0($v1)
    /* 6E5A0 8007DDA0 0A80033C */  lui        $v1, %hi(D_8009BF64)
    /* 6E5A4 8007DDA4 64BF638C */  lw         $v1, %lo(D_8009BF64)($v1)
    /* 6E5A8 8007DDA8 00000000 */  nop
    /* 6E5AC 8007DDAC 0000628C */  lw         $v0, 0x0($v1)
    /* 6E5B0 8007DDB0 00000000 */  nop
    /* 6E5B4 8007DDB4 00084234 */  ori        $v0, $v0, 0x800
    /* 6E5B8 8007DDB8 000062AC */  sw         $v0, 0x0($v1)
    /* 6E5BC 8007DDBC 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E5C0 8007DDC0 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E5C4 8007DDC4 0002023C */  lui        $v0, (0x2000000 >> 16)
    /* 6E5C8 8007DDC8 000062AC */  sw         $v0, 0x0($v1)
    /* 6E5CC 8007DDCC 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E5D0 8007DDD0 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E5D4 8007DDD4 0001023C */  lui        $v0, (0x1000000 >> 16)
    /* 6E5D8 8007DDD8 000062AC */  sw         $v0, 0x0($v1)
    /* 6E5DC 8007DDDC 0A80043C */  lui        $a0, %hi(D_8009BF88)
    /* 6E5E0 8007DDE0 88BF848C */  lw         $a0, %lo(D_8009BF88)($a0)
    /* 6E5E4 8007DDE4 0F0B020C */  jal        motion_make_table
    /* 6E5E8 8007DDE8 00000000 */   nop
    /* 6E5EC 8007DDEC 7EF70108 */  j          .L8007DDF8
    /* 6E5F0 8007DDF0 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8007DDF4:
    /* 6E5F4 8007DDF4 21100000 */  addu       $v0, $zero, $zero
  .L8007DDF8:
    /* 6E5F8 8007DDF8 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6E5FC 8007DDFC 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6E600 8007DE00 0800E003 */  jr         $ra
    /* 6E604 8007DE04 00000000 */   nop
endlabel func_8007DC9C

nonmatching func_8007DE08, 0xDC

glabel func_8007DE08
    /* 6E608 8007DE08 0010033C */  lui        $v1, (0x10000007 >> 16)
    /* 6E60C 8007DE0C 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6E610 8007DE10 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6E614 8007DE14 07006334 */  ori        $v1, $v1, (0x10000007 & 0xFFFF)
    /* 6E618 8007DE18 000043AC */  sw         $v1, 0x0($v0)
    /* 6E61C 8007DE1C 0A80053C */  lui        $a1, %hi(D_8009BF44)
    /* 6E620 8007DE20 44BFA58C */  lw         $a1, %lo(D_8009BF44)($a1)
    /* 6E624 8007DE24 FF00033C */  lui        $v1, (0xFFFFFF >> 16)
    /* 6E628 8007DE28 0000A28C */  lw         $v0, 0x0($a1)
    /* 6E62C 8007DE2C FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 6E630 8007DE30 24104300 */  and        $v0, $v0, $v1
    /* 6E634 8007DE34 02000324 */  addiu      $v1, $zero, 0x2
    /* 6E638 8007DE38 1D004310 */  beq        $v0, $v1, .L8007DEB0
    /* 6E63C 8007DE3C 00E1033C */   lui       $v1, (0xE1001000 >> 16)
    /* 6E640 8007DE40 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6E644 8007DE44 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6E648 8007DE48 00000000 */  nop
    /* 6E64C 8007DE4C 0000428C */  lw         $v0, 0x0($v0)
    /* 6E650 8007DE50 00106334 */  ori        $v1, $v1, (0xE1001000 & 0xFFFF)
    /* 6E654 8007DE54 FF3F4230 */  andi       $v0, $v0, 0x3FFF
    /* 6E658 8007DE58 25104300 */  or         $v0, $v0, $v1
    /* 6E65C 8007DE5C 0000A2AC */  sw         $v0, 0x0($a1)
    /* 6E660 8007DE60 0A80023C */  lui        $v0, %hi(D_8009BF44)
    /* 6E664 8007DE64 44BF428C */  lw         $v0, %lo(D_8009BF44)($v0)
    /* 6E668 8007DE68 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E66C 8007DE6C 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E670 8007DE70 0000428C */  lw         $v0, 0x0($v0)
    /* 6E674 8007DE74 0000628C */  lw         $v0, 0x0($v1)
    /* 6E678 8007DE78 00000000 */  nop
    /* 6E67C 8007DE7C 00104230 */  andi       $v0, $v0, 0x1000
    /* 6E680 8007DE80 03004014 */  bnez       $v0, .L8007DE90
    /* 6E684 8007DE84 08008230 */   andi      $v0, $a0, 0x8
    /* 6E688 8007DE88 B7F70108 */  j          .L8007DEDC
    /* 6E68C 8007DE8C 21100000 */   addu      $v0, $zero, $zero
  .L8007DE90:
    /* 6E690 8007DE90 03004014 */  bnez       $v0, .L8007DEA0
    /* 6E694 8007DE94 0020023C */   lui       $v0, (0x20000504 >> 16)
    /* 6E698 8007DE98 B7F70108 */  j          .L8007DEDC
    /* 6E69C 8007DE9C 01000224 */   addiu     $v0, $zero, 0x1
  .L8007DEA0:
    /* 6E6A0 8007DEA0 04054234 */  ori        $v0, $v0, (0x20000504 & 0xFFFF)
    /* 6E6A4 8007DEA4 000062AC */  sw         $v0, 0x0($v1)
    /* 6E6A8 8007DEA8 B7F70108 */  j          .L8007DEDC
    /* 6E6AC 8007DEAC 02000224 */   addiu     $v0, $zero, 0x2
  .L8007DEB0:
    /* 6E6B0 8007DEB0 08008230 */  andi       $v0, $a0, 0x8
    /* 6E6B4 8007DEB4 08004010 */  beqz       $v0, .L8007DED8
    /* 6E6B8 8007DEB8 0009043C */   lui       $a0, (0x9000001 >> 16)
    /* 6E6BC 8007DEBC 01008434 */  ori        $a0, $a0, (0x9000001 & 0xFFFF)
    /* 6E6C0 8007DEC0 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E6C4 8007DEC4 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E6C8 8007DEC8 04000224 */  addiu      $v0, $zero, 0x4
    /* 6E6CC 8007DECC 000064AC */  sw         $a0, 0x0($v1)
    /* 6E6D0 8007DED0 B7F70108 */  j          .L8007DEDC
    /* 6E6D4 8007DED4 00000000 */   nop
  .L8007DED8:
    /* 6E6D8 8007DED8 03000224 */  addiu      $v0, $zero, 0x3
  .L8007DEDC:
    /* 6E6DC 8007DEDC 0800E003 */  jr         $ra
    /* 6E6E0 8007DEE0 00000000 */   nop
endlabel func_8007DE08

nonmatching func_8007DEE4, 0x2C

glabel func_8007DEE4
    /* 6E6E4 8007DEE4 F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 6E6E8 8007DEE8 0600C010 */  beqz       $a2, .L8007DF04
    /* 6E6EC 8007DEEC FFFFC224 */   addiu     $v0, $a2, -0x1
    /* 6E6F0 8007DEF0 FFFF0324 */  addiu      $v1, $zero, -0x1
  .L8007DEF4:
    /* 6E6F4 8007DEF4 000085A0 */  sb         $a1, 0x0($a0)
    /* 6E6F8 8007DEF8 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6E6FC 8007DEFC FDFF4314 */  bne        $v0, $v1, .L8007DEF4
    /* 6E700 8007DF00 01008424 */   addiu     $a0, $a0, 0x1
  .L8007DF04:
    /* 6E704 8007DF04 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 6E708 8007DF08 0800E003 */  jr         $ra
    /* 6E70C 8007DF0C 00000000 */   nop
endlabel func_8007DEE4

nonmatching func_8007DF10, 0xC

glabel func_8007DF10
    /* 6E710 8007DF10 A0000A24 */  addiu      $t2, $zero, 0xA0
    /* 6E714 8007DF14 08004001 */  jr         $t2
    /* 6E718 8007DF18 49000924 */   addiu     $t1, $zero, 0x49
endlabel func_8007DF10
    /* 6E71C 8007DF1C 00000000 */  nop

nonmatching func_8007DF20, 0x3C

glabel func_8007DF20
    /* 6E720 8007DF20 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6E724 8007DF24 05008004 */  bltz       $a0, .L8007DF3C
    /* 6E728 8007DF28 1000BFAF */   sw        $ra, 0x10($sp)
    /* 6E72C 8007DF2C D7F7010C */  jal        func_8007DF5C
    /* 6E730 8007DF30 FF0F8430 */   andi      $a0, $a0, 0xFFF
    /* 6E734 8007DF34 D3F70108 */  j          .L8007DF4C
    /* 6E738 8007DF38 00000000 */   nop
  .L8007DF3C:
    /* 6E73C 8007DF3C 23200400 */  negu       $a0, $a0
    /* 6E740 8007DF40 D7F7010C */  jal        func_8007DF5C
    /* 6E744 8007DF44 FF0F8430 */   andi      $a0, $a0, 0xFFF
    /* 6E748 8007DF48 23100200 */  negu       $v0, $v0
  .L8007DF4C:
    /* 6E74C 8007DF4C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6E750 8007DF50 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6E754 8007DF54 0800E003 */  jr         $ra
    /* 6E758 8007DF58 00000000 */   nop
endlabel func_8007DF20

nonmatching func_8007DF5C, 0x90

glabel func_8007DF5C
    /* 6E75C 8007DF5C 01088228 */  slti       $v0, $a0, 0x801
    /* 6E760 8007DF60 10004010 */  beqz       $v0, .L8007DFA4
    /* 6E764 8007DF64 01048228 */   slti      $v0, $a0, 0x401
    /* 6E768 8007DF68 06004010 */  beqz       $v0, .L8007DF84
    /* 6E76C 8007DF6C 40100400 */   sll       $v0, $a0, 1
    /* 6E770 8007DF70 0A80013C */  lui        $at, %hi(D_8009BF94)
    /* 6E774 8007DF74 21082200 */  addu       $at, $at, $v0
    /* 6E778 8007DF78 94BF2284 */  lh         $v0, %lo(D_8009BF94)($at)
    /* 6E77C 8007DF7C F9F70108 */  j          .L8007DFE4
    /* 6E780 8007DF80 00000000 */   nop
  .L8007DF84:
    /* 6E784 8007DF84 00080224 */  addiu      $v0, $zero, 0x800
    /* 6E788 8007DF88 23104400 */  subu       $v0, $v0, $a0
    /* 6E78C 8007DF8C 40100200 */  sll        $v0, $v0, 1
    /* 6E790 8007DF90 0A80013C */  lui        $at, %hi(D_8009BF94)
    /* 6E794 8007DF94 21082200 */  addu       $at, $at, $v0
    /* 6E798 8007DF98 94BF2284 */  lh         $v0, %lo(D_8009BF94)($at)
    /* 6E79C 8007DF9C F9F70108 */  j          .L8007DFE4
    /* 6E7A0 8007DFA0 00000000 */   nop
  .L8007DFA4:
    /* 6E7A4 8007DFA4 010C8228 */  slti       $v0, $a0, 0xC01
    /* 6E7A8 8007DFA8 09004014 */  bnez       $v0, .L8007DFD0
    /* 6E7AC 8007DFAC 40100400 */   sll       $v0, $a0, 1
    /* 6E7B0 8007DFB0 00100224 */  addiu      $v0, $zero, 0x1000
    /* 6E7B4 8007DFB4 23104400 */  subu       $v0, $v0, $a0
    /* 6E7B8 8007DFB8 40100200 */  sll        $v0, $v0, 1
    /* 6E7BC 8007DFBC 0A80013C */  lui        $at, %hi(D_8009BF94)
    /* 6E7C0 8007DFC0 21082200 */  addu       $at, $at, $v0
    /* 6E7C4 8007DFC4 94BF2284 */  lh         $v0, %lo(D_8009BF94)($at)
    /* 6E7C8 8007DFC8 F9F70108 */  j          .L8007DFE4
    /* 6E7CC 8007DFCC 23100200 */   negu      $v0, $v0
  .L8007DFD0:
    /* 6E7D0 8007DFD0 0A80013C */  lui        $at, %hi(D_8009AF94)
    /* 6E7D4 8007DFD4 21082200 */  addu       $at, $at, $v0
    /* 6E7D8 8007DFD8 94AF2284 */  lh         $v0, %lo(D_8009AF94)($at)
    /* 6E7DC 8007DFDC 00000000 */  nop
    /* 6E7E0 8007DFE0 23100200 */  negu       $v0, $v0
  .L8007DFE4:
    /* 6E7E4 8007DFE4 0800E003 */  jr         $ra
    /* 6E7E8 8007DFE8 00000000 */   nop
endlabel func_8007DF5C

nonmatching func_8007DFEC, 0xA0

glabel func_8007DFEC
    /* 6E7EC 8007DFEC 02008104 */  bgez       $a0, .L8007DFF8
    /* 6E7F0 8007DFF0 00000000 */   nop
    /* 6E7F4 8007DFF4 23200400 */  negu       $a0, $a0
  .L8007DFF8:
    /* 6E7F8 8007DFF8 FF0F8430 */  andi       $a0, $a0, 0xFFF
    /* 6E7FC 8007DFFC 01088228 */  slti       $v0, $a0, 0x801
    /* 6E800 8007E000 10004010 */  beqz       $v0, .L8007E044
    /* 6E804 8007E004 01048228 */   slti      $v0, $a0, 0x401
    /* 6E808 8007E008 08004010 */  beqz       $v0, .L8007E02C
    /* 6E80C 8007E00C 00040224 */   addiu     $v0, $zero, 0x400
    /* 6E810 8007E010 23104400 */  subu       $v0, $v0, $a0
    /* 6E814 8007E014 40100200 */  sll        $v0, $v0, 1
    /* 6E818 8007E018 0A80013C */  lui        $at, %hi(D_8009BF94)
    /* 6E81C 8007E01C 21082200 */  addu       $at, $at, $v0
    /* 6E820 8007E020 94BF2284 */  lh         $v0, %lo(D_8009BF94)($at)
    /* 6E824 8007E024 21F80108 */  j          .L8007E084
    /* 6E828 8007E028 00000000 */   nop
  .L8007E02C:
    /* 6E82C 8007E02C 40100400 */  sll        $v0, $a0, 1
    /* 6E830 8007E030 0A80013C */  lui        $at, %hi(D_8009B794)
    /* 6E834 8007E034 21082200 */  addu       $at, $at, $v0
    /* 6E838 8007E038 94B72284 */  lh         $v0, %lo(D_8009B794)($at)
    /* 6E83C 8007E03C 21F80108 */  j          .L8007E084
    /* 6E840 8007E040 23100200 */   negu      $v0, $v0
  .L8007E044:
    /* 6E844 8007E044 010C8228 */  slti       $v0, $a0, 0xC01
    /* 6E848 8007E048 07004014 */  bnez       $v0, .L8007E068
    /* 6E84C 8007E04C 000C0224 */   addiu     $v0, $zero, 0xC00
    /* 6E850 8007E050 40100400 */  sll        $v0, $a0, 1
    /* 6E854 8007E054 0A80013C */  lui        $at, %hi(D_8009A794)
    /* 6E858 8007E058 21082200 */  addu       $at, $at, $v0
    /* 6E85C 8007E05C 94A72284 */  lh         $v0, %lo(D_8009A794)($at)
    /* 6E860 8007E060 21F80108 */  j          .L8007E084
    /* 6E864 8007E064 00000000 */   nop
  .L8007E068:
    /* 6E868 8007E068 23104400 */  subu       $v0, $v0, $a0
    /* 6E86C 8007E06C 40100200 */  sll        $v0, $v0, 1
    /* 6E870 8007E070 0A80013C */  lui        $at, %hi(D_8009BF94)
    /* 6E874 8007E074 21082200 */  addu       $at, $at, $v0
    /* 6E878 8007E078 94BF2284 */  lh         $v0, %lo(D_8009BF94)($at)
    /* 6E87C 8007E07C 00000000 */  nop
    /* 6E880 8007E080 23100200 */  negu       $v0, $v0
  .L8007E084:
    /* 6E884 8007E084 0800E003 */  jr         $ra
    /* 6E888 8007E088 00000000 */   nop
endlabel func_8007DFEC

nonmatching D_8007E08C

dlabel D_8007E08C
    /* 6E88C 8007E08C 50730915 */ .word 0x15097350
    /* 6E890 8007E090 9C9F4000 */ .word 0x00409F9C /* invalid instruction */
enddlabel D_8007E08C

/* Handwritten function */
nonmatching func_8007E094, 0x80

glabel func_8007E094
    /* 6E894 8007E094 0A80013C */  lui        $at, %hi(D_8009C798)
    /* 6E898 8007E098 98C73FAC */  sw         $ra, %lo(D_8009C798)($at)
    /* 6E89C 8007E09C B7FF010C */  jal        func_8007FEDC
    /* 6E8A0 8007E0A0 00000000 */   nop
    /* 6E8A4 8007E0A4 0A801F3C */  lui        $ra, %hi(D_8009C798)
    /* 6E8A8 8007E0A8 98C7FF8F */  lw         $ra, %lo(D_8009C798)($ra)
    /* 6E8AC 8007E0AC 00000000 */  nop
    /* 6E8B0 8007E0B0 00600240 */  mfc0       $v0, $12 /* handwritten instruction */
    /* 6E8B4 8007E0B4 0040033C */  lui        $v1, (0x40000000 >> 16)
    /* 6E8B8 8007E0B8 25104300 */  or         $v0, $v0, $v1
    /* 6E8BC 8007E0BC 00608240 */  mtc0       $v0, $12 /* handwritten instruction */
    /* 6E8C0 8007E0C0 00000000 */  nop
    /* 6E8C4 8007E0C4 55010824 */  addiu      $t0, $zero, 0x155
    /* 6E8C8 8007E0C8 00E8C848 */  ctc2       $t0, $29 /* handwritten instruction */
    /* 6E8CC 8007E0CC 00000000 */  nop
    /* 6E8D0 8007E0D0 00010824 */  addiu      $t0, $zero, 0x100
    /* 6E8D4 8007E0D4 00F0C848 */  ctc2       $t0, $30 /* handwritten instruction */
    /* 6E8D8 8007E0D8 00000000 */  nop
    /* 6E8DC 8007E0DC E8030824 */  addiu      $t0, $zero, 0x3E8
    /* 6E8E0 8007E0E0 00D0C848 */  ctc2       $t0, $26 /* handwritten instruction */
    /* 6E8E4 8007E0E4 00000000 */  nop
    /* 6E8E8 8007E0E8 9EEF0824 */  addiu      $t0, $zero, -0x1062
    /* 6E8EC 8007E0EC 00D8C848 */  ctc2       $t0, $27 /* handwritten instruction */
    /* 6E8F0 8007E0F0 00000000 */  nop
    /* 6E8F4 8007E0F4 4001083C */  lui        $t0, (0x1400000 >> 16)
    /* 6E8F8 8007E0F8 00E0C848 */  ctc2       $t0, $28 /* handwritten instruction */
    /* 6E8FC 8007E0FC 00000000 */  nop
    /* 6E900 8007E100 00C0C048 */  ctc2       $zero, $24 /* handwritten instruction */
    /* 6E904 8007E104 00C8C048 */  ctc2       $zero, $25 /* handwritten instruction */
    /* 6E908 8007E108 00000000 */  nop
    /* 6E90C 8007E10C 0800E003 */  jr         $ra
    /* 6E910 8007E110 00000000 */   nop
endlabel func_8007E094
    /* 6E914 8007E114 00000000 */  nop
    /* 6E918 8007E118 00000000 */  nop

/* Handwritten function */
nonmatching func_8007E11C, 0x84

glabel func_8007E11C
    /* 6E91C 8007E11C 00F08448 */  mtc2       $a0, $30 /* handwritten instruction */
    /* 6E920 8007E120 00000000 */  nop
    /* 6E924 8007E124 00000000 */  nop
    /* 6E928 8007E128 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6E92C 8007E12C 20000124 */  addiu      $at, $zero, 0x20
    /* 6E930 8007E130 19004110 */  beq        $v0, $at, .L8007E198
    /* 6E934 8007E134 00000000 */   nop
    /* 6E938 8007E138 01004830 */  andi       $t0, $v0, 0x1
    /* 6E93C 8007E13C FEFF0A24 */  addiu      $t2, $zero, -0x2
    /* 6E940 8007E140 24504A00 */  and        $t2, $v0, $t2
    /* 6E944 8007E144 1F000924 */  addiu      $t1, $zero, 0x1F
    /* 6E948 8007E148 22482A01 */  sub        $t1, $t1, $t2 /* handwritten instruction */
    /* 6E94C 8007E14C 43480900 */  sra        $t1, $t1, 1
    /* 6E950 8007E150 E8FF4B21 */  addi       $t3, $t2, -0x18 /* handwritten instruction */
    /* 6E954 8007E154 03006005 */  bltz       $t3, .L8007E164
    /* 6E958 8007E158 00000000 */   nop
    /* 6E95C 8007E15C 04606401 */  sllv       $t4, $a0, $t3
    /* 6E960 8007E160 03000010 */  b          .L8007E170
  .L8007E164:
    /* 6E964 8007E164 18000B24 */   addiu     $t3, $zero, 0x18
    /* 6E968 8007E168 22586A01 */  sub        $t3, $t3, $t2 /* handwritten instruction */
    /* 6E96C 8007E16C 07606401 */  srav       $t4, $a0, $t3
  .L8007E170:
    /* 6E970 8007E170 C0FF8C21 */  addi       $t4, $t4, -0x40 /* handwritten instruction */
    /* 6E974 8007E174 40600C00 */  sll        $t4, $t4, 1
    /* 6E978 8007E178 0A800D3C */  lui        $t5, %hi(D_8009C7A8)
    /* 6E97C 8007E17C 2168AC01 */  addu       $t5, $t5, $t4
    /* 6E980 8007E180 A8C7AD85 */  lh         $t5, %lo(D_8009C7A8)($t5)
    /* 6E984 8007E184 00000000 */  nop
    /* 6E988 8007E188 04682D01 */  sllv       $t5, $t5, $t1
    /* 6E98C 8007E18C 02130D00 */  srl        $v0, $t5, 12
    /* 6E990 8007E190 0800E003 */  jr         $ra
    /* 6E994 8007E194 00000000 */   nop
  .L8007E198:
    /* 6E998 8007E198 0800E003 */  jr         $ra
    /* 6E99C 8007E19C 00000224 */   addiu     $v0, $zero, 0x0
endlabel func_8007E11C
    /* 6E9A0 8007E1A0 00000000 */  nop
    /* 6E9A4 8007E1A4 00000000 */  nop
    /* 6E9A8 8007E1A8 00000000 */  nop

/* Handwritten function */
nonmatching func_8007E1AC, 0x28C

glabel func_8007E1AC
    /* 6E9AC 8007E1AC 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6E9B0 8007E1B0 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6E9B4 8007E1B4 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6E9B8 8007E1B8 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6E9BC 8007E1BC 00000000 */  nop
    /* 6E9C0 8007E1C0 3D00984B */  gpf        1
    /* 6E9C4 8007E1C4 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6E9C8 8007E1C8 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6E9CC 8007E1CC 0000A9C8 */  lwc2       $9, 0x0($a1)
    /* 6E9D0 8007E1D0 0400AAC8 */  lwc2       $10, 0x4($a1)
    /* 6E9D4 8007E1D4 0800ABC8 */  lwc2       $11, 0x8($a1)
    /* 6E9D8 8007E1D8 00000000 */  nop
    /* 6E9DC 8007E1DC 3E00A84B */  gpl        1
    /* 6E9E0 8007E1E0 1000A88F */  lw         $t0, 0x10($sp)
    /* 6E9E4 8007E1E4 00000000 */  nop
    /* 6E9E8 8007E1E8 000009E9 */  swc2       $9, 0x0($t0)
    /* 6E9EC 8007E1EC 04000AE9 */  swc2       $10, 0x4($t0)
    /* 6E9F0 8007E1F0 08000BE9 */  swc2       $11, 0x8($t0)
    /* 6E9F4 8007E1F4 0800E003 */  jr         $ra
    /* 6E9F8 8007E1F8 00000000 */   nop
    /* 6E9FC 8007E1FC 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EA00 8007E200 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6EA04 8007E204 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6EA08 8007E208 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6EA0C 8007E20C 00000000 */  nop
    /* 6EA10 8007E210 3D00904B */  gpf        0
    /* 6EA14 8007E214 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EA18 8007E218 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EA1C 8007E21C 0000A9C8 */  lwc2       $9, 0x0($a1)
    /* 6EA20 8007E220 0400AAC8 */  lwc2       $10, 0x4($a1)
    /* 6EA24 8007E224 0800ABC8 */  lwc2       $11, 0x8($a1)
    /* 6EA28 8007E228 00000000 */  nop
    /* 6EA2C 8007E22C 3E00A04B */  gpl        0
    /* 6EA30 8007E230 1000A88F */  lw         $t0, 0x10($sp)
    /* 6EA34 8007E234 00000000 */  nop
    /* 6EA38 8007E238 000009E9 */  swc2       $9, 0x0($t0)
    /* 6EA3C 8007E23C 04000AE9 */  swc2       $10, 0x4($t0)
    /* 6EA40 8007E240 08000BE9 */  swc2       $11, 0x8($t0)
    /* 6EA44 8007E244 0800E003 */  jr         $ra
    /* 6EA48 8007E248 00000000 */   nop
    /* 6EA4C 8007E24C 0000888C */  lw         $t0, 0x0($a0)
    /* 6EA50 8007E250 04008A8C */  lw         $t2, 0x4($a0)
    /* 6EA54 8007E254 034C0800 */  sra        $t1, $t0, 16
    /* 6EA58 8007E258 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EA5C 8007E25C FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EA60 8007E260 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EA64 8007E264 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EA68 8007E268 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EA6C 8007E26C 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EA70 8007E270 00000000 */  nop
    /* 6EA74 8007E274 3D00984B */  gpf        1
    /* 6EA78 8007E278 0000A88C */  lw         $t0, 0x0($a1)
    /* 6EA7C 8007E27C 0400AA8C */  lw         $t2, 0x4($a1)
    /* 6EA80 8007E280 034C0800 */  sra        $t1, $t0, 16
    /* 6EA84 8007E284 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EA88 8007E288 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EA8C 8007E28C 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EA90 8007E290 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EA94 8007E294 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EA98 8007E298 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EA9C 8007E29C 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EAA0 8007E2A0 00000000 */  nop
    /* 6EAA4 8007E2A4 3E00A84B */  gpl        1
    /* 6EAA8 8007E2A8 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6EAAC 8007E2AC 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6EAB0 8007E2B0 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EAB4 8007E2B4 004C0900 */  sll        $t1, $t1, 16
    /* 6EAB8 8007E2B8 25400901 */  or         $t0, $t0, $t1
    /* 6EABC 8007E2BC 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EAC0 8007E2C0 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 6EAC4 8007E2C4 0000A8AD */  sw         $t0, 0x0($t5)
    /* 6EAC8 8007E2C8 0400AAAD */  sw         $t2, 0x4($t5)
    /* 6EACC 8007E2CC 0800E003 */  jr         $ra
    /* 6EAD0 8007E2D0 00000000 */   nop
    /* 6EAD4 8007E2D4 0000888C */  lw         $t0, 0x0($a0)
    /* 6EAD8 8007E2D8 04008A8C */  lw         $t2, 0x4($a0)
    /* 6EADC 8007E2DC 034C0800 */  sra        $t1, $t0, 16
    /* 6EAE0 8007E2E0 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EAE4 8007E2E4 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EAE8 8007E2E8 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EAEC 8007E2EC 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EAF0 8007E2F0 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EAF4 8007E2F4 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EAF8 8007E2F8 00000000 */  nop
    /* 6EAFC 8007E2FC 3D00904B */  gpf        0
    /* 6EB00 8007E300 0000A88C */  lw         $t0, 0x0($a1)
    /* 6EB04 8007E304 0400AA8C */  lw         $t2, 0x4($a1)
    /* 6EB08 8007E308 034C0800 */  sra        $t1, $t0, 16
    /* 6EB0C 8007E30C FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EB10 8007E310 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EB14 8007E314 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EB18 8007E318 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EB1C 8007E31C 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EB20 8007E320 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EB24 8007E324 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EB28 8007E328 00000000 */  nop
    /* 6EB2C 8007E32C 3E00A04B */  gpl        0
    /* 6EB30 8007E330 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6EB34 8007E334 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6EB38 8007E338 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EB3C 8007E33C 004C0900 */  sll        $t1, $t1, 16
    /* 6EB40 8007E340 25400901 */  or         $t0, $t0, $t1
    /* 6EB44 8007E344 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EB48 8007E348 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 6EB4C 8007E34C 0000A8AD */  sw         $t0, 0x0($t5)
    /* 6EB50 8007E350 0400AAAD */  sw         $t2, 0x4($t5)
    /* 6EB54 8007E354 0800E003 */  jr         $ra
    /* 6EB58 8007E358 00000000 */   nop
    /* 6EB5C 8007E35C 00008890 */  lbu        $t0, 0x0($a0)
    /* 6EB60 8007E360 01008990 */  lbu        $t1, 0x1($a0)
    /* 6EB64 8007E364 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EB68 8007E368 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EB6C 8007E36C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EB70 8007E370 00000000 */  nop
    /* 6EB74 8007E374 3D00904B */  gpf        0
    /* 6EB78 8007E378 0000A890 */  lbu        $t0, 0x0($a1)
    /* 6EB7C 8007E37C 0100A990 */  lbu        $t1, 0x1($a1)
    /* 6EB80 8007E380 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EB84 8007E384 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EB88 8007E388 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EB8C 8007E38C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EB90 8007E390 0C000B24 */  addiu      $t3, $zero, 0xC
    /* 6EB94 8007E394 3E00A04B */  gpl        0
    /* 6EB98 8007E398 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EB9C 8007E39C 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6EBA0 8007E3A0 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6EBA4 8007E3A4 07406801 */  srav       $t0, $t0, $t3
    /* 6EBA8 8007E3A8 07486901 */  srav       $t1, $t1, $t3
    /* 6EBAC 8007E3AC 0000A8A1 */  sb         $t0, 0x0($t5)
    /* 6EBB0 8007E3B0 0100A9A1 */  sb         $t1, 0x1($t5)
    /* 6EBB4 8007E3B4 0800E003 */  jr         $ra
    /* 6EBB8 8007E3B8 00000000 */   nop
    /* 6EBBC 8007E3BC 00008890 */  lbu        $t0, 0x0($a0)
    /* 6EBC0 8007E3C0 01008990 */  lbu        $t1, 0x1($a0)
    /* 6EBC4 8007E3C4 02008A90 */  lbu        $t2, 0x2($a0)
    /* 6EBC8 8007E3C8 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EBCC 8007E3CC 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EBD0 8007E3D0 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EBD4 8007E3D4 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EBD8 8007E3D8 00000000 */  nop
    /* 6EBDC 8007E3DC 3D00904B */  gpf        0
    /* 6EBE0 8007E3E0 0000A890 */  lbu        $t0, 0x0($a1)
    /* 6EBE4 8007E3E4 0100A990 */  lbu        $t1, 0x1($a1)
    /* 6EBE8 8007E3E8 0200AA90 */  lbu        $t2, 0x2($a1)
    /* 6EBEC 8007E3EC 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EBF0 8007E3F0 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EBF4 8007E3F4 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EBF8 8007E3F8 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EBFC 8007E3FC 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EC00 8007E400 0C000B24 */  addiu      $t3, $zero, 0xC
    /* 6EC04 8007E404 3E00A04B */  gpl        0
    /* 6EC08 8007E408 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EC0C 8007E40C 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6EC10 8007E410 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6EC14 8007E414 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 6EC18 8007E418 07406801 */  srav       $t0, $t0, $t3
    /* 6EC1C 8007E41C 07486901 */  srav       $t1, $t1, $t3
    /* 6EC20 8007E420 07506A01 */  srav       $t2, $t2, $t3
    /* 6EC24 8007E424 0000A8A1 */  sb         $t0, 0x0($t5)
    /* 6EC28 8007E428 0100A9A1 */  sb         $t1, 0x1($t5)
    /* 6EC2C 8007E42C 0200AAA1 */  sb         $t2, 0x2($t5)
    /* 6EC30 8007E430 0800E003 */  jr         $ra
    /* 6EC34 8007E434 00000000 */   nop
endlabel func_8007E1AC
    /* 6EC38 8007E438 00000000 */  nop

/* Handwritten function */
nonmatching func_8007E43C, 0x94

glabel func_8007E43C
    /* 6EC3C 8007E43C 00F08448 */  mtc2       $a0, $30 /* handwritten instruction */
    /* 6EC40 8007E440 00000000 */  nop
    /* 6EC44 8007E444 00000000 */  nop
    /* 6EC48 8007E448 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EC4C 8007E44C 20000124 */  addiu      $at, $zero, 0x20
    /* 6EC50 8007E450 1D004110 */  beq        $v0, $at, .L8007E4C8
    /* 6EC54 8007E454 00000000 */   nop
    /* 6EC58 8007E458 01004830 */  andi       $t0, $v0, 0x1
    /* 6EC5C 8007E45C FEFF0A24 */  addiu      $t2, $zero, -0x2
    /* 6EC60 8007E460 24504A00 */  and        $t2, $v0, $t2
    /* 6EC64 8007E464 13000924 */  addiu      $t1, $zero, 0x13
    /* 6EC68 8007E468 22482A01 */  sub        $t1, $t1, $t2 /* handwritten instruction */
    /* 6EC6C 8007E46C 43480900 */  sra        $t1, $t1, 1
    /* 6EC70 8007E470 E8FF4B21 */  addi       $t3, $t2, -0x18 /* handwritten instruction */
    /* 6EC74 8007E474 03006005 */  bltz       $t3, .L8007E484
    /* 6EC78 8007E478 00000000 */   nop
    /* 6EC7C 8007E47C 04606401 */  sllv       $t4, $a0, $t3
    /* 6EC80 8007E480 03000010 */  b          .L8007E490
  .L8007E484:
    /* 6EC84 8007E484 18000B24 */   addiu     $t3, $zero, 0x18
    /* 6EC88 8007E488 22586A01 */  sub        $t3, $t3, $t2 /* handwritten instruction */
    /* 6EC8C 8007E48C 07606401 */  srav       $t4, $a0, $t3
  .L8007E490:
    /* 6EC90 8007E490 C0FF8C21 */  addi       $t4, $t4, -0x40 /* handwritten instruction */
    /* 6EC94 8007E494 40600C00 */  sll        $t4, $t4, 1
    /* 6EC98 8007E498 0A800D3C */  lui        $t5, %hi(D_8009C7A8)
    /* 6EC9C 8007E49C 2168AC01 */  addu       $t5, $t5, $t4
    /* 6ECA0 8007E4A0 A8C7AD85 */  lh         $t5, %lo(D_8009C7A8)($t5)
    /* 6ECA4 8007E4A4 00000000 */  nop
    /* 6ECA8 8007E4A8 04002005 */  bltz       $t1, .L8007E4BC
    /* 6ECAC 8007E4AC 00000000 */   nop
    /* 6ECB0 8007E4B0 04102D01 */  sllv       $v0, $t5, $t1
    /* 6ECB4 8007E4B4 0800E003 */  jr         $ra
    /* 6ECB8 8007E4B8 00000000 */   nop
  .L8007E4BC:
    /* 6ECBC 8007E4BC 22480900 */  neg        $t1, $t1 /* handwritten instruction */
    /* 6ECC0 8007E4C0 0800E003 */  jr         $ra
    /* 6ECC4 8007E4C4 06102D01 */   srlv      $v0, $t5, $t1
  .L8007E4C8:
    /* 6ECC8 8007E4C8 0800E003 */  jr         $ra
    /* 6ECCC 8007E4CC 00000224 */   addiu     $v0, $zero, 0x0
endlabel func_8007E43C
    /* 6ECD0 8007E4D0 00000000 */  nop
    /* 6ECD4 8007E4D4 00000000 */  nop
    /* 6ECD8 8007E4D8 00000000 */  nop

/* Handwritten function */
nonmatching func_8007E4DC, 0x10C

glabel func_8007E4DC
    /* 6ECDC 8007E4DC 0000888C */  lw         $t0, 0x0($a0)
    /* 6ECE0 8007E4E0 0400898C */  lw         $t1, 0x4($a0)
    /* 6ECE4 8007E4E4 08008A8C */  lw         $t2, 0x8($a0)
    /* 6ECE8 8007E4E8 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6ECEC 8007E4EC 10008C8C */  lw         $t4, 0x10($a0)
    /* 6ECF0 8007E4F0 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6ECF4 8007E4F4 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6ECF8 8007E4F8 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6ECFC 8007E4FC 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6ED00 8007E500 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6ED04 8007E504 0000A894 */  lhu        $t0, 0x0($a1)
    /* 6ED08 8007E508 0400A98C */  lw         $t1, 0x4($a1)
    /* 6ED0C 8007E50C 0C00AA8C */  lw         $t2, 0xC($a1)
    /* 6ED10 8007E510 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6ED14 8007E514 24482101 */  and        $t1, $t1, $at
    /* 6ED18 8007E518 25400901 */  or         $t0, $t0, $t1
    /* 6ED1C 8007E51C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6ED20 8007E520 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6ED24 8007E524 00000000 */  nop
    /* 6ED28 8007E528 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6ED2C 8007E52C 0200A894 */  lhu        $t0, 0x2($a1)
    /* 6ED30 8007E530 0800A98C */  lw         $t1, 0x8($a1)
    /* 6ED34 8007E534 0E00AA84 */  lh         $t2, 0xE($a1)
    /* 6ED38 8007E538 004C0900 */  sll        $t1, $t1, 16
    /* 6ED3C 8007E53C 25400901 */  or         $t0, $t0, $t1
    /* 6ED40 8007E540 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 6ED44 8007E544 00500C48 */  mfc2       $t4, $10 /* handwritten instruction */
    /* 6ED48 8007E548 00580D48 */  mfc2       $t5, $11 /* handwritten instruction */
    /* 6ED4C 8007E54C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6ED50 8007E550 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6ED54 8007E554 00000000 */  nop
    /* 6ED58 8007E558 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6ED5C 8007E55C 0400A894 */  lhu        $t0, 0x4($a1)
    /* 6ED60 8007E560 0800A98C */  lw         $t1, 0x8($a1)
    /* 6ED64 8007E564 1000AA8C */  lw         $t2, 0x10($a1)
    /* 6ED68 8007E568 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6ED6C 8007E56C 24482101 */  and        $t1, $t1, $at
    /* 6ED70 8007E570 25400901 */  or         $t0, $t0, $t1
    /* 6ED74 8007E574 00480E48 */  mfc2       $t6, $9 /* handwritten instruction */
    /* 6ED78 8007E578 00500F48 */  mfc2       $t7, $10 /* handwritten instruction */
    /* 6ED7C 8007E57C 00581848 */  mfc2       $t8, $11 /* handwritten instruction */
    /* 6ED80 8007E580 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6ED84 8007E584 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6ED88 8007E588 00000000 */  nop
    /* 6ED8C 8007E58C 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6ED90 8007E590 FFFF6B31 */  andi       $t3, $t3, 0xFFFF
    /* 6ED94 8007E594 00740E00 */  sll        $t6, $t6, 16
    /* 6ED98 8007E598 2570CB01 */  or         $t6, $t6, $t3
    /* 6ED9C 8007E59C 0000CEAC */  sw         $t6, 0x0($a2)
    /* 6EDA0 8007E5A0 FFFFAD31 */  andi       $t5, $t5, 0xFFFF
    /* 6EDA4 8007E5A4 00C41800 */  sll        $t8, $t8, 16
    /* 6EDA8 8007E5A8 25C00D03 */  or         $t8, $t8, $t5
    /* 6EDAC 8007E5AC 0C00D8AC */  sw         $t8, 0xC($a2)
    /* 6EDB0 8007E5B0 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6EDB4 8007E5B4 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6EDB8 8007E5B8 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EDBC 8007E5BC 00640C00 */  sll        $t4, $t4, 16
    /* 6EDC0 8007E5C0 25400C01 */  or         $t0, $t0, $t4
    /* 6EDC4 8007E5C4 0400C8AC */  sw         $t0, 0x4($a2)
    /* 6EDC8 8007E5C8 FFFFEF31 */  andi       $t7, $t7, 0xFFFF
    /* 6EDCC 8007E5CC 004C0900 */  sll        $t1, $t1, 16
    /* 6EDD0 8007E5D0 25482F01 */  or         $t1, $t1, $t7
    /* 6EDD4 8007E5D4 0800C9AC */  sw         $t1, 0x8($a2)
    /* 6EDD8 8007E5D8 1000CBE8 */  swc2       $11, 0x10($a2)
    /* 6EDDC 8007E5DC 2110C000 */  addu       $v0, $a2, $zero
    /* 6EDE0 8007E5E0 0800E003 */  jr         $ra
    /* 6EDE4 8007E5E4 00000000 */   nop
endlabel func_8007E4DC
    /* 6EDE8 8007E5E8 00000000 */  nop

/* Handwritten function */
nonmatching func_8007E5EC, 0x160

glabel func_8007E5EC
    /* 6EDEC 8007E5EC 0000888C */  lw         $t0, 0x0($a0)
    /* 6EDF0 8007E5F0 0400898C */  lw         $t1, 0x4($a0)
    /* 6EDF4 8007E5F4 08008A8C */  lw         $t2, 0x8($a0)
    /* 6EDF8 8007E5F8 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6EDFC 8007E5FC 10008C8C */  lw         $t4, 0x10($a0)
    /* 6EE00 8007E600 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6EE04 8007E604 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6EE08 8007E608 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6EE0C 8007E60C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6EE10 8007E610 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6EE14 8007E614 0000A894 */  lhu        $t0, 0x0($a1)
    /* 6EE18 8007E618 0400A98C */  lw         $t1, 0x4($a1)
    /* 6EE1C 8007E61C 0C00AA8C */  lw         $t2, 0xC($a1)
    /* 6EE20 8007E620 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6EE24 8007E624 24482101 */  and        $t1, $t1, $at
    /* 6EE28 8007E628 25400901 */  or         $t0, $t0, $t1
    /* 6EE2C 8007E62C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6EE30 8007E630 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6EE34 8007E634 00000000 */  nop
    /* 6EE38 8007E638 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6EE3C 8007E63C 0200A894 */  lhu        $t0, 0x2($a1)
    /* 6EE40 8007E640 0800A98C */  lw         $t1, 0x8($a1)
    /* 6EE44 8007E644 0E00AA84 */  lh         $t2, 0xE($a1)
    /* 6EE48 8007E648 004C0900 */  sll        $t1, $t1, 16
    /* 6EE4C 8007E64C 25400901 */  or         $t0, $t0, $t1
    /* 6EE50 8007E650 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 6EE54 8007E654 00500C48 */  mfc2       $t4, $10 /* handwritten instruction */
    /* 6EE58 8007E658 00580D48 */  mfc2       $t5, $11 /* handwritten instruction */
    /* 6EE5C 8007E65C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6EE60 8007E660 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6EE64 8007E664 00000000 */  nop
    /* 6EE68 8007E668 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6EE6C 8007E66C 0400A894 */  lhu        $t0, 0x4($a1)
    /* 6EE70 8007E670 0800A98C */  lw         $t1, 0x8($a1)
    /* 6EE74 8007E674 1000AA8C */  lw         $t2, 0x10($a1)
    /* 6EE78 8007E678 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6EE7C 8007E67C 24482101 */  and        $t1, $t1, $at
    /* 6EE80 8007E680 25400901 */  or         $t0, $t0, $t1
    /* 6EE84 8007E684 00480E48 */  mfc2       $t6, $9 /* handwritten instruction */
    /* 6EE88 8007E688 00500F48 */  mfc2       $t7, $10 /* handwritten instruction */
    /* 6EE8C 8007E68C 00581848 */  mfc2       $t8, $11 /* handwritten instruction */
    /* 6EE90 8007E690 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6EE94 8007E694 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6EE98 8007E698 00000000 */  nop
    /* 6EE9C 8007E69C 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6EEA0 8007E6A0 FFFF6B31 */  andi       $t3, $t3, 0xFFFF
    /* 6EEA4 8007E6A4 00740E00 */  sll        $t6, $t6, 16
    /* 6EEA8 8007E6A8 2570CB01 */  or         $t6, $t6, $t3
    /* 6EEAC 8007E6AC 0000CEAC */  sw         $t6, 0x0($a2)
    /* 6EEB0 8007E6B0 FFFFAD31 */  andi       $t5, $t5, 0xFFFF
    /* 6EEB4 8007E6B4 00C41800 */  sll        $t8, $t8, 16
    /* 6EEB8 8007E6B8 25C00D03 */  or         $t8, $t8, $t5
    /* 6EEBC 8007E6BC 0C00D8AC */  sw         $t8, 0xC($a2)
    /* 6EEC0 8007E6C0 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6EEC4 8007E6C4 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6EEC8 8007E6C8 1000CBE8 */  swc2       $11, 0x10($a2)
    /* 6EECC 8007E6CC 1400AD94 */  lhu        $t5, 0x14($a1)
    /* 6EED0 8007E6D0 1800AE8C */  lw         $t6, 0x18($a1)
    /* 6EED4 8007E6D4 1C00AA8C */  lw         $t2, 0x1C($a1)
    /* 6EED8 8007E6D8 00740E00 */  sll        $t6, $t6, 16
    /* 6EEDC 8007E6DC 2568AE01 */  or         $t5, $t5, $t6
    /* 6EEE0 8007E6E0 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 6EEE4 8007E6E4 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6EEE8 8007E6E8 00000000 */  nop
    /* 6EEEC 8007E6EC 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6EEF0 8007E6F0 00640C00 */  sll        $t4, $t4, 16
    /* 6EEF4 8007E6F4 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EEF8 8007E6F8 25400C01 */  or         $t0, $t0, $t4
    /* 6EEFC 8007E6FC 0400C8AC */  sw         $t0, 0x4($a2)
    /* 6EF00 8007E700 FFFFEF31 */  andi       $t7, $t7, 0xFFFF
    /* 6EF04 8007E704 004C0900 */  sll        $t1, $t1, 16
    /* 6EF08 8007E708 25482F01 */  or         $t1, $t1, $t7
    /* 6EF0C 8007E70C 0800C9AC */  sw         $t1, 0x8($a2)
    /* 6EF10 8007E710 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6EF14 8007E714 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6EF18 8007E718 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 6EF1C 8007E71C 14008B8C */  lw         $t3, 0x14($a0)
    /* 6EF20 8007E720 18008C8C */  lw         $t4, 0x18($a0)
    /* 6EF24 8007E724 1C008D8C */  lw         $t5, 0x1C($a0)
    /* 6EF28 8007E728 20400B01 */  add        $t0, $t0, $t3 /* handwritten instruction */
    /* 6EF2C 8007E72C 20482C01 */  add        $t1, $t1, $t4 /* handwritten instruction */
    /* 6EF30 8007E730 20504D01 */  add        $t2, $t2, $t5 /* handwritten instruction */
    /* 6EF34 8007E734 1400C8AC */  sw         $t0, 0x14($a2)
    /* 6EF38 8007E738 1800C9AC */  sw         $t1, 0x18($a2)
    /* 6EF3C 8007E73C 1C00CAAC */  sw         $t2, 0x1C($a2)
    /* 6EF40 8007E740 2110C000 */  addu       $v0, $a2, $zero
    /* 6EF44 8007E744 0800E003 */  jr         $ra
    /* 6EF48 8007E748 00000000 */   nop
endlabel func_8007E5EC

/* Handwritten function */
nonmatching func_8007E74C, 0x160

glabel func_8007E74C
    /* 6EF4C 8007E74C 0000888C */  lw         $t0, 0x0($a0)
    /* 6EF50 8007E750 0400898C */  lw         $t1, 0x4($a0)
    /* 6EF54 8007E754 08008A8C */  lw         $t2, 0x8($a0)
    /* 6EF58 8007E758 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6EF5C 8007E75C 10008C8C */  lw         $t4, 0x10($a0)
    /* 6EF60 8007E760 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6EF64 8007E764 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6EF68 8007E768 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6EF6C 8007E76C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6EF70 8007E770 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6EF74 8007E774 0000A88C */  lw         $t0, 0x0($a1)
    /* 6EF78 8007E778 0400A98C */  lw         $t1, 0x4($a1)
    /* 6EF7C 8007E77C 0800AA8C */  lw         $t2, 0x8($a1)
    /* 6EF80 8007E780 08000105 */  bgez       $t0, .L8007E7A4
    /* 6EF84 8007E784 C35B0800 */   sra       $t3, $t0, 15
    /* 6EF88 8007E788 23400800 */  negu       $t0, $t0
    /* 6EF8C 8007E78C C35B0800 */  sra        $t3, $t0, 15
    /* 6EF90 8007E790 FF7F0831 */  andi       $t0, $t0, 0x7FFF
    /* 6EF94 8007E794 23580B00 */  negu       $t3, $t3
    /* 6EF98 8007E798 03000010 */  b          .L8007E7A8
    /* 6EF9C 8007E79C 23400800 */   negu      $t0, $t0
    /* 6EFA0 8007E7A0 C35B0800 */  sra        $t3, $t0, 15
  .L8007E7A4:
    /* 6EFA4 8007E7A4 FF7F0831 */  andi       $t0, $t0, 0x7FFF
  .L8007E7A8:
    /* 6EFA8 8007E7A8 08002105 */  bgez       $t1, .L8007E7CC
    /* 6EFAC 8007E7AC C3630900 */   sra       $t4, $t1, 15
    /* 6EFB0 8007E7B0 23480900 */  negu       $t1, $t1
    /* 6EFB4 8007E7B4 C3630900 */  sra        $t4, $t1, 15
    /* 6EFB8 8007E7B8 FF7F2931 */  andi       $t1, $t1, 0x7FFF
    /* 6EFBC 8007E7BC 23600C00 */  negu       $t4, $t4
    /* 6EFC0 8007E7C0 03000010 */  b          .L8007E7D0
    /* 6EFC4 8007E7C4 23480900 */   negu      $t1, $t1
    /* 6EFC8 8007E7C8 C3630900 */  sra        $t4, $t1, 15
  .L8007E7CC:
    /* 6EFCC 8007E7CC FF7F2931 */  andi       $t1, $t1, 0x7FFF
  .L8007E7D0:
    /* 6EFD0 8007E7D0 08004105 */  bgez       $t2, .L8007E7F4
    /* 6EFD4 8007E7D4 C36B0A00 */   sra       $t5, $t2, 15
    /* 6EFD8 8007E7D8 23500A00 */  negu       $t2, $t2
    /* 6EFDC 8007E7DC C36B0A00 */  sra        $t5, $t2, 15
    /* 6EFE0 8007E7E0 FF7F4A31 */  andi       $t2, $t2, 0x7FFF
    /* 6EFE4 8007E7E4 23680D00 */  negu       $t5, $t5
    /* 6EFE8 8007E7E8 03000010 */  b          .L8007E7F8
    /* 6EFEC 8007E7EC 23500A00 */   negu      $t2, $t2
    /* 6EFF0 8007E7F0 C36B0A00 */  sra        $t5, $t2, 15
  .L8007E7F4:
    /* 6EFF4 8007E7F4 FF7F4A31 */  andi       $t2, $t2, 0x7FFF
  .L8007E7F8:
    /* 6EFF8 8007E7F8 00488B48 */  mtc2       $t3, $9 /* handwritten instruction */
    /* 6EFFC 8007E7FC 00508C48 */  mtc2       $t4, $10 /* handwritten instruction */
    /* 6F000 8007E800 00588D48 */  mtc2       $t5, $11 /* handwritten instruction */
    /* 6F004 8007E804 00000000 */  nop
    /* 6F008 8007E808 12E0414A */  mvmva      0, 0, 3, 3, 0
    /* 6F00C 8007E80C 00C80B48 */  mfc2       $t3, $25 /* handwritten instruction */
    /* 6F010 8007E810 00D00C48 */  mfc2       $t4, $26 /* handwritten instruction */
    /* 6F014 8007E814 00D80D48 */  mfc2       $t5, $27 /* handwritten instruction */
    /* 6F018 8007E818 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6F01C 8007E81C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6F020 8007E820 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6F024 8007E824 00000000 */  nop
    /* 6F028 8007E828 12E0494A */  mvmva      1, 0, 3, 3, 0
    /* 6F02C 8007E82C 05006105 */  bgez       $t3, .L8007E844
    /* 6F030 8007E830 00000000 */   nop
    /* 6F034 8007E834 23580B00 */  negu       $t3, $t3
    /* 6F038 8007E838 C0580B00 */  sll        $t3, $t3, 3
    /* 6F03C 8007E83C 02000010 */  b          .L8007E848
    /* 6F040 8007E840 23580B00 */   negu      $t3, $t3
  .L8007E844:
    /* 6F044 8007E844 C0580B00 */  sll        $t3, $t3, 3
  .L8007E848:
    /* 6F048 8007E848 05008105 */  bgez       $t4, .L8007E860
    /* 6F04C 8007E84C 00000000 */   nop
    /* 6F050 8007E850 23600C00 */  negu       $t4, $t4
    /* 6F054 8007E854 C0600C00 */  sll        $t4, $t4, 3
    /* 6F058 8007E858 02000010 */  b          .L8007E864
    /* 6F05C 8007E85C 23600C00 */   negu      $t4, $t4
  .L8007E860:
    /* 6F060 8007E860 C0600C00 */  sll        $t4, $t4, 3
  .L8007E864:
    /* 6F064 8007E864 0500A105 */  bgez       $t5, .L8007E87C
    /* 6F068 8007E868 00000000 */   nop
    /* 6F06C 8007E86C 23680D00 */  negu       $t5, $t5
    /* 6F070 8007E870 C0680D00 */  sll        $t5, $t5, 3
    /* 6F074 8007E874 02000010 */  b          .L8007E880
    /* 6F078 8007E878 23680D00 */   negu      $t5, $t5
  .L8007E87C:
    /* 6F07C 8007E87C C0680D00 */  sll        $t5, $t5, 3
  .L8007E880:
    /* 6F080 8007E880 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6F084 8007E884 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6F088 8007E888 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 6F08C 8007E88C 21400B01 */  addu       $t0, $t0, $t3
    /* 6F090 8007E890 21482C01 */  addu       $t1, $t1, $t4
    /* 6F094 8007E894 21504D01 */  addu       $t2, $t2, $t5
    /* 6F098 8007E898 0000C8AC */  sw         $t0, 0x0($a2)
    /* 6F09C 8007E89C 0400C9AC */  sw         $t1, 0x4($a2)
    /* 6F0A0 8007E8A0 0800CAAC */  sw         $t2, 0x8($a2)
    /* 6F0A4 8007E8A4 0800E003 */  jr         $ra
    /* 6F0A8 8007E8A8 2110C000 */   addu      $v0, $a2, $zero
endlabel func_8007E74C

/* Handwritten function */
nonmatching func_8007E8AC, 0x30

glabel func_8007E8AC
    /* 6F0AC 8007E8AC 0000888C */  lw         $t0, 0x0($a0)
    /* 6F0B0 8007E8B0 0400898C */  lw         $t1, 0x4($a0)
    /* 6F0B4 8007E8B4 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F0B8 8007E8B8 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 6F0BC 8007E8BC 00000000 */  nop
    /* 6F0C0 8007E8C0 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F0C4 8007E8C4 0000A9E8 */  swc2       $9, 0x0($a1)
    /* 6F0C8 8007E8C8 0400AAE8 */  swc2       $10, 0x4($a1)
    /* 6F0CC 8007E8CC 0800ABE8 */  swc2       $11, 0x8($a1)
    /* 6F0D0 8007E8D0 2110C000 */  addu       $v0, $a2, $zero
    /* 6F0D4 8007E8D4 0800E003 */  jr         $ra
    /* 6F0D8 8007E8D8 00000000 */   nop
endlabel func_8007E8AC

nonmatching func_8007E8DC, 0x124

glabel func_8007E8DC
    /* 6F0DC 8007E8DC 0000888C */  lw         $t0, 0x0($a0)
    /* 6F0E0 8007E8E0 0000AB8C */  lw         $t3, 0x0($a1)
    /* 6F0E4 8007E8E4 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F0E8 8007E8E8 004C0900 */  sll        $t1, $t1, 16
    /* 6F0EC 8007E8EC 034C0900 */  sra        $t1, $t1, 16
    /* 6F0F0 8007E8F0 19002B01 */  multu      $t1, $t3
    /* 6F0F4 8007E8F4 03540800 */  sra        $t2, $t0, 16
    /* 6F0F8 8007E8F8 0400AC8C */  lw         $t4, 0x4($a1)
    /* 6F0FC 8007E8FC 0800AD8C */  lw         $t5, 0x8($a1)
    /* 6F100 8007E900 0400888C */  lw         $t0, 0x4($a0)
    /* 6F104 8007E904 21108000 */  addu       $v0, $a0, $zero
    /* 6F108 8007E908 12480000 */  mflo       $t1
    /* 6F10C 8007E90C 034B0900 */  sra        $t1, $t1, 12
    /* 6F110 8007E910 FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F114 8007E914 19004B01 */  multu      $t2, $t3
    /* 6F118 8007E918 12500000 */  mflo       $t2
    /* 6F11C 8007E91C 03530A00 */  sra        $t2, $t2, 12
    /* 6F120 8007E920 00540A00 */  sll        $t2, $t2, 16
    /* 6F124 8007E924 25482A01 */  or         $t1, $t1, $t2
    /* 6F128 8007E928 000089AC */  sw         $t1, 0x0($a0)
    /* 6F12C 8007E92C FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F130 8007E930 004C0900 */  sll        $t1, $t1, 16
    /* 6F134 8007E934 034C0900 */  sra        $t1, $t1, 16
    /* 6F138 8007E938 19002B01 */  multu      $t1, $t3
    /* 6F13C 8007E93C 03540800 */  sra        $t2, $t0, 16
    /* 6F140 8007E940 0800888C */  lw         $t0, 0x8($a0)
    /* 6F144 8007E944 12480000 */  mflo       $t1
    /* 6F148 8007E948 034B0900 */  sra        $t1, $t1, 12
    /* 6F14C 8007E94C FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F150 8007E950 19004C01 */  multu      $t2, $t4
    /* 6F154 8007E954 12500000 */  mflo       $t2
    /* 6F158 8007E958 03530A00 */  sra        $t2, $t2, 12
    /* 6F15C 8007E95C 00540A00 */  sll        $t2, $t2, 16
    /* 6F160 8007E960 25482A01 */  or         $t1, $t1, $t2
    /* 6F164 8007E964 040089AC */  sw         $t1, 0x4($a0)
    /* 6F168 8007E968 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F16C 8007E96C 004C0900 */  sll        $t1, $t1, 16
    /* 6F170 8007E970 034C0900 */  sra        $t1, $t1, 16
    /* 6F174 8007E974 19002C01 */  multu      $t1, $t4
    /* 6F178 8007E978 03540800 */  sra        $t2, $t0, 16
    /* 6F17C 8007E97C 0C00888C */  lw         $t0, 0xC($a0)
    /* 6F180 8007E980 12480000 */  mflo       $t1
    /* 6F184 8007E984 034B0900 */  sra        $t1, $t1, 12
    /* 6F188 8007E988 FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F18C 8007E98C 19004C01 */  multu      $t2, $t4
    /* 6F190 8007E990 12500000 */  mflo       $t2
    /* 6F194 8007E994 03530A00 */  sra        $t2, $t2, 12
    /* 6F198 8007E998 00540A00 */  sll        $t2, $t2, 16
    /* 6F19C 8007E99C 25482A01 */  or         $t1, $t1, $t2
    /* 6F1A0 8007E9A0 080089AC */  sw         $t1, 0x8($a0)
    /* 6F1A4 8007E9A4 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F1A8 8007E9A8 004C0900 */  sll        $t1, $t1, 16
    /* 6F1AC 8007E9AC 034C0900 */  sra        $t1, $t1, 16
    /* 6F1B0 8007E9B0 19002D01 */  multu      $t1, $t5
    /* 6F1B4 8007E9B4 03540800 */  sra        $t2, $t0, 16
    /* 6F1B8 8007E9B8 1000888C */  lw         $t0, 0x10($a0)
    /* 6F1BC 8007E9BC 12480000 */  mflo       $t1
    /* 6F1C0 8007E9C0 034B0900 */  sra        $t1, $t1, 12
    /* 6F1C4 8007E9C4 FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F1C8 8007E9C8 19004D01 */  multu      $t2, $t5
    /* 6F1CC 8007E9CC 12500000 */  mflo       $t2
    /* 6F1D0 8007E9D0 03530A00 */  sra        $t2, $t2, 12
    /* 6F1D4 8007E9D4 00540A00 */  sll        $t2, $t2, 16
    /* 6F1D8 8007E9D8 25482A01 */  or         $t1, $t1, $t2
    /* 6F1DC 8007E9DC 0C0089AC */  sw         $t1, 0xC($a0)
    /* 6F1E0 8007E9E0 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F1E4 8007E9E4 004C0900 */  sll        $t1, $t1, 16
    /* 6F1E8 8007E9E8 034C0900 */  sra        $t1, $t1, 16
    /* 6F1EC 8007E9EC 19002D01 */  multu      $t1, $t5
    /* 6F1F0 8007E9F0 12480000 */  mflo       $t1
    /* 6F1F4 8007E9F4 034B0900 */  sra        $t1, $t1, 12
    /* 6F1F8 8007E9F8 0800E003 */  jr         $ra
    /* 6F1FC 8007E9FC 100089AC */   sw        $t1, 0x10($a0)
endlabel func_8007E8DC
    /* 6F200 8007EA00 00000000 */  nop
    /* 6F204 8007EA04 00000000 */  nop
    /* 6F208 8007EA08 00000000 */  nop

/* Handwritten function */
nonmatching func_8007EA0C, 0x138

glabel func_8007EA0C
    /* 6F20C 8007EA0C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F210 8007EA10 0400898C */  lw         $t1, 0x4($a0)
    /* 6F214 8007EA14 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F218 8007EA18 08000105 */  bgez       $t0, .L8007EA3C
    /* 6F21C 8007EA1C C35B0800 */   sra       $t3, $t0, 15
    /* 6F220 8007EA20 23400800 */  negu       $t0, $t0
    /* 6F224 8007EA24 C35B0800 */  sra        $t3, $t0, 15
    /* 6F228 8007EA28 FF7F0831 */  andi       $t0, $t0, 0x7FFF
    /* 6F22C 8007EA2C 23580B00 */  negu       $t3, $t3
    /* 6F230 8007EA30 03000010 */  b          .L8007EA40
    /* 6F234 8007EA34 23400800 */   negu      $t0, $t0
    /* 6F238 8007EA38 C35B0800 */  sra        $t3, $t0, 15
  .L8007EA3C:
    /* 6F23C 8007EA3C FF7F0831 */  andi       $t0, $t0, 0x7FFF
  .L8007EA40:
    /* 6F240 8007EA40 08002105 */  bgez       $t1, .L8007EA64
    /* 6F244 8007EA44 C3630900 */   sra       $t4, $t1, 15
    /* 6F248 8007EA48 23480900 */  negu       $t1, $t1
    /* 6F24C 8007EA4C C3630900 */  sra        $t4, $t1, 15
    /* 6F250 8007EA50 FF7F2931 */  andi       $t1, $t1, 0x7FFF
    /* 6F254 8007EA54 23600C00 */  negu       $t4, $t4
    /* 6F258 8007EA58 03000010 */  b          .L8007EA68
    /* 6F25C 8007EA5C 23480900 */   negu      $t1, $t1
    /* 6F260 8007EA60 C3630900 */  sra        $t4, $t1, 15
  .L8007EA64:
    /* 6F264 8007EA64 FF7F2931 */  andi       $t1, $t1, 0x7FFF
  .L8007EA68:
    /* 6F268 8007EA68 08004105 */  bgez       $t2, .L8007EA8C
    /* 6F26C 8007EA6C C36B0A00 */   sra       $t5, $t2, 15
    /* 6F270 8007EA70 23500A00 */  negu       $t2, $t2
    /* 6F274 8007EA74 C36B0A00 */  sra        $t5, $t2, 15
    /* 6F278 8007EA78 FF7F4A31 */  andi       $t2, $t2, 0x7FFF
    /* 6F27C 8007EA7C 23680D00 */  negu       $t5, $t5
    /* 6F280 8007EA80 03000010 */  b          .L8007EA90
    /* 6F284 8007EA84 23500A00 */   negu      $t2, $t2
    /* 6F288 8007EA88 C36B0A00 */  sra        $t5, $t2, 15
  .L8007EA8C:
    /* 6F28C 8007EA8C FF7F4A31 */  andi       $t2, $t2, 0x7FFF
  .L8007EA90:
    /* 6F290 8007EA90 00488B48 */  mtc2       $t3, $9 /* handwritten instruction */
    /* 6F294 8007EA94 00508C48 */  mtc2       $t4, $10 /* handwritten instruction */
    /* 6F298 8007EA98 00588D48 */  mtc2       $t5, $11 /* handwritten instruction */
    /* 6F29C 8007EA9C 00000000 */  nop
    /* 6F2A0 8007EAA0 12E0414A */  mvmva      0, 0, 3, 3, 0
    /* 6F2A4 8007EAA4 00C80B48 */  mfc2       $t3, $25 /* handwritten instruction */
    /* 6F2A8 8007EAA8 00D00C48 */  mfc2       $t4, $26 /* handwritten instruction */
    /* 6F2AC 8007EAAC 00D80D48 */  mfc2       $t5, $27 /* handwritten instruction */
    /* 6F2B0 8007EAB0 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6F2B4 8007EAB4 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6F2B8 8007EAB8 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6F2BC 8007EABC 00000000 */  nop
    /* 6F2C0 8007EAC0 12E0494A */  mvmva      1, 0, 3, 3, 0
    /* 6F2C4 8007EAC4 05006105 */  bgez       $t3, .L8007EADC
    /* 6F2C8 8007EAC8 00000000 */   nop
    /* 6F2CC 8007EACC 23580B00 */  negu       $t3, $t3
    /* 6F2D0 8007EAD0 C0580B00 */  sll        $t3, $t3, 3
    /* 6F2D4 8007EAD4 02000010 */  b          .L8007EAE0
    /* 6F2D8 8007EAD8 23580B00 */   negu      $t3, $t3
  .L8007EADC:
    /* 6F2DC 8007EADC C0580B00 */  sll        $t3, $t3, 3
  .L8007EAE0:
    /* 6F2E0 8007EAE0 05008105 */  bgez       $t4, .L8007EAF8
    /* 6F2E4 8007EAE4 00000000 */   nop
    /* 6F2E8 8007EAE8 23600C00 */  negu       $t4, $t4
    /* 6F2EC 8007EAEC C0600C00 */  sll        $t4, $t4, 3
    /* 6F2F0 8007EAF0 02000010 */  b          .L8007EAFC
    /* 6F2F4 8007EAF4 23600C00 */   negu      $t4, $t4
  .L8007EAF8:
    /* 6F2F8 8007EAF8 C0600C00 */  sll        $t4, $t4, 3
  .L8007EAFC:
    /* 6F2FC 8007EAFC 0500A105 */  bgez       $t5, .L8007EB14
    /* 6F300 8007EB00 00000000 */   nop
    /* 6F304 8007EB04 23680D00 */  negu       $t5, $t5
    /* 6F308 8007EB08 C0680D00 */  sll        $t5, $t5, 3
    /* 6F30C 8007EB0C 02000010 */  b          .L8007EB18
    /* 6F310 8007EB10 23680D00 */   negu      $t5, $t5
  .L8007EB14:
    /* 6F314 8007EB14 C0680D00 */  sll        $t5, $t5, 3
  .L8007EB18:
    /* 6F318 8007EB18 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6F31C 8007EB1C 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6F320 8007EB20 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 6F324 8007EB24 21400B01 */  addu       $t0, $t0, $t3
    /* 6F328 8007EB28 21482C01 */  addu       $t1, $t1, $t4
    /* 6F32C 8007EB2C 21504D01 */  addu       $t2, $t2, $t5
    /* 6F330 8007EB30 0000A8AC */  sw         $t0, 0x0($a1)
    /* 6F334 8007EB34 0400A9AC */  sw         $t1, 0x4($a1)
    /* 6F338 8007EB38 0800AAAC */  sw         $t2, 0x8($a1)
    /* 6F33C 8007EB3C 0800E003 */  jr         $ra
    /* 6F340 8007EB40 2110A000 */   addu      $v0, $a1, $zero
endlabel func_8007EA0C
    /* 6F344 8007EB44 00000000 */  nop
    /* 6F348 8007EB48 00000000 */  nop

/* Handwritten function */
nonmatching func_8007EB4C, 0x10C

glabel func_8007EB4C
    /* 6F34C 8007EB4C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F350 8007EB50 0400898C */  lw         $t1, 0x4($a0)
    /* 6F354 8007EB54 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F358 8007EB58 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6F35C 8007EB5C 10008C8C */  lw         $t4, 0x10($a0)
    /* 6F360 8007EB60 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F364 8007EB64 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6F368 8007EB68 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6F36C 8007EB6C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6F370 8007EB70 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6F374 8007EB74 0000A894 */  lhu        $t0, 0x0($a1)
    /* 6F378 8007EB78 0400A98C */  lw         $t1, 0x4($a1)
    /* 6F37C 8007EB7C 0C00AA8C */  lw         $t2, 0xC($a1)
    /* 6F380 8007EB80 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6F384 8007EB84 24482101 */  and        $t1, $t1, $at
    /* 6F388 8007EB88 25400901 */  or         $t0, $t0, $t1
    /* 6F38C 8007EB8C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F390 8007EB90 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F394 8007EB94 00000000 */  nop
    /* 6F398 8007EB98 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F39C 8007EB9C 0200A894 */  lhu        $t0, 0x2($a1)
    /* 6F3A0 8007EBA0 0800A98C */  lw         $t1, 0x8($a1)
    /* 6F3A4 8007EBA4 0E00AA84 */  lh         $t2, 0xE($a1)
    /* 6F3A8 8007EBA8 004C0900 */  sll        $t1, $t1, 16
    /* 6F3AC 8007EBAC 25400901 */  or         $t0, $t0, $t1
    /* 6F3B0 8007EBB0 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 6F3B4 8007EBB4 00500C48 */  mfc2       $t4, $10 /* handwritten instruction */
    /* 6F3B8 8007EBB8 00580D48 */  mfc2       $t5, $11 /* handwritten instruction */
    /* 6F3BC 8007EBBC 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F3C0 8007EBC0 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F3C4 8007EBC4 00000000 */  nop
    /* 6F3C8 8007EBC8 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F3CC 8007EBCC 0400A894 */  lhu        $t0, 0x4($a1)
    /* 6F3D0 8007EBD0 0800A98C */  lw         $t1, 0x8($a1)
    /* 6F3D4 8007EBD4 1000AA8C */  lw         $t2, 0x10($a1)
    /* 6F3D8 8007EBD8 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6F3DC 8007EBDC 24482101 */  and        $t1, $t1, $at
    /* 6F3E0 8007EBE0 25400901 */  or         $t0, $t0, $t1
    /* 6F3E4 8007EBE4 00480E48 */  mfc2       $t6, $9 /* handwritten instruction */
    /* 6F3E8 8007EBE8 00500F48 */  mfc2       $t7, $10 /* handwritten instruction */
    /* 6F3EC 8007EBEC 00581848 */  mfc2       $t8, $11 /* handwritten instruction */
    /* 6F3F0 8007EBF0 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F3F4 8007EBF4 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F3F8 8007EBF8 00000000 */  nop
    /* 6F3FC 8007EBFC 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F400 8007EC00 FFFF6B31 */  andi       $t3, $t3, 0xFFFF
    /* 6F404 8007EC04 00740E00 */  sll        $t6, $t6, 16
    /* 6F408 8007EC08 2570CB01 */  or         $t6, $t6, $t3
    /* 6F40C 8007EC0C 00008EAC */  sw         $t6, 0x0($a0)
    /* 6F410 8007EC10 FFFFAD31 */  andi       $t5, $t5, 0xFFFF
    /* 6F414 8007EC14 00C41800 */  sll        $t8, $t8, 16
    /* 6F418 8007EC18 25C00D03 */  or         $t8, $t8, $t5
    /* 6F41C 8007EC1C 0C0098AC */  sw         $t8, 0xC($a0)
    /* 6F420 8007EC20 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6F424 8007EC24 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6F428 8007EC28 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6F42C 8007EC2C 00640C00 */  sll        $t4, $t4, 16
    /* 6F430 8007EC30 25400C01 */  or         $t0, $t0, $t4
    /* 6F434 8007EC34 040088AC */  sw         $t0, 0x4($a0)
    /* 6F438 8007EC38 FFFFEF31 */  andi       $t7, $t7, 0xFFFF
    /* 6F43C 8007EC3C 004C0900 */  sll        $t1, $t1, 16
    /* 6F440 8007EC40 25482F01 */  or         $t1, $t1, $t7
    /* 6F444 8007EC44 080089AC */  sw         $t1, 0x8($a0)
    /* 6F448 8007EC48 10008BE8 */  swc2       $11, 0x10($a0)
    /* 6F44C 8007EC4C 21108000 */  addu       $v0, $a0, $zero
    /* 6F450 8007EC50 0800E003 */  jr         $ra
    /* 6F454 8007EC54 00000000 */   nop
endlabel func_8007EB4C
    /* 6F458 8007EC58 00000000 */  nop

/* Handwritten function */
nonmatching func_8007EC5C, 0x10C

glabel func_8007EC5C
    /* 6F45C 8007EC5C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F460 8007EC60 0400898C */  lw         $t1, 0x4($a0)
    /* 6F464 8007EC64 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F468 8007EC68 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6F46C 8007EC6C 10008C8C */  lw         $t4, 0x10($a0)
    /* 6F470 8007EC70 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F474 8007EC74 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6F478 8007EC78 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6F47C 8007EC7C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6F480 8007EC80 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6F484 8007EC84 0000A894 */  lhu        $t0, 0x0($a1)
    /* 6F488 8007EC88 0400A98C */  lw         $t1, 0x4($a1)
    /* 6F48C 8007EC8C 0C00AA8C */  lw         $t2, 0xC($a1)
    /* 6F490 8007EC90 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6F494 8007EC94 24482101 */  and        $t1, $t1, $at
    /* 6F498 8007EC98 25400901 */  or         $t0, $t0, $t1
    /* 6F49C 8007EC9C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F4A0 8007ECA0 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F4A4 8007ECA4 00000000 */  nop
    /* 6F4A8 8007ECA8 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F4AC 8007ECAC 0200A894 */  lhu        $t0, 0x2($a1)
    /* 6F4B0 8007ECB0 0800A98C */  lw         $t1, 0x8($a1)
    /* 6F4B4 8007ECB4 0E00AA84 */  lh         $t2, 0xE($a1)
    /* 6F4B8 8007ECB8 004C0900 */  sll        $t1, $t1, 16
    /* 6F4BC 8007ECBC 25400901 */  or         $t0, $t0, $t1
    /* 6F4C0 8007ECC0 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 6F4C4 8007ECC4 00500C48 */  mfc2       $t4, $10 /* handwritten instruction */
    /* 6F4C8 8007ECC8 00580D48 */  mfc2       $t5, $11 /* handwritten instruction */
    /* 6F4CC 8007ECCC 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F4D0 8007ECD0 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F4D4 8007ECD4 00000000 */  nop
    /* 6F4D8 8007ECD8 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F4DC 8007ECDC 0400A894 */  lhu        $t0, 0x4($a1)
    /* 6F4E0 8007ECE0 0800A98C */  lw         $t1, 0x8($a1)
    /* 6F4E4 8007ECE4 1000AA8C */  lw         $t2, 0x10($a1)
    /* 6F4E8 8007ECE8 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6F4EC 8007ECEC 24482101 */  and        $t1, $t1, $at
    /* 6F4F0 8007ECF0 25400901 */  or         $t0, $t0, $t1
    /* 6F4F4 8007ECF4 00480E48 */  mfc2       $t6, $9 /* handwritten instruction */
    /* 6F4F8 8007ECF8 00500F48 */  mfc2       $t7, $10 /* handwritten instruction */
    /* 6F4FC 8007ECFC 00581848 */  mfc2       $t8, $11 /* handwritten instruction */
    /* 6F500 8007ED00 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F504 8007ED04 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F508 8007ED08 00000000 */  nop
    /* 6F50C 8007ED0C 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F510 8007ED10 FFFF6B31 */  andi       $t3, $t3, 0xFFFF
    /* 6F514 8007ED14 00740E00 */  sll        $t6, $t6, 16
    /* 6F518 8007ED18 2570CB01 */  or         $t6, $t6, $t3
    /* 6F51C 8007ED1C 0000AEAC */  sw         $t6, 0x0($a1)
    /* 6F520 8007ED20 FFFFAD31 */  andi       $t5, $t5, 0xFFFF
    /* 6F524 8007ED24 00C41800 */  sll        $t8, $t8, 16
    /* 6F528 8007ED28 25C00D03 */  or         $t8, $t8, $t5
    /* 6F52C 8007ED2C 0C00B8AC */  sw         $t8, 0xC($a1)
    /* 6F530 8007ED30 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6F534 8007ED34 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6F538 8007ED38 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6F53C 8007ED3C 00640C00 */  sll        $t4, $t4, 16
    /* 6F540 8007ED40 25400C01 */  or         $t0, $t0, $t4
    /* 6F544 8007ED44 0400A8AC */  sw         $t0, 0x4($a1)
    /* 6F548 8007ED48 FFFFEF31 */  andi       $t7, $t7, 0xFFFF
    /* 6F54C 8007ED4C 004C0900 */  sll        $t1, $t1, 16
    /* 6F550 8007ED50 25482F01 */  or         $t1, $t1, $t7
    /* 6F554 8007ED54 0800A9AC */  sw         $t1, 0x8($a1)
    /* 6F558 8007ED58 1000ABE8 */  swc2       $11, 0x10($a1)
    /* 6F55C 8007ED5C 2110A000 */  addu       $v0, $a1, $zero
    /* 6F560 8007ED60 0800E003 */  jr         $ra
    /* 6F564 8007ED64 00000000 */   nop
endlabel func_8007EC5C
    /* 6F568 8007ED68 00000000 */  nop

/* Handwritten function */
nonmatching func_8007ED6C, 0x50

glabel func_8007ED6C
    /* 6F56C 8007ED6C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F570 8007ED70 0400898C */  lw         $t1, 0x4($a0)
    /* 6F574 8007ED74 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F578 8007ED78 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6F57C 8007ED7C 10008C8C */  lw         $t4, 0x10($a0)
    /* 6F580 8007ED80 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F584 8007ED84 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6F588 8007ED88 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6F58C 8007ED8C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6F590 8007ED90 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6F594 8007ED94 0000A0C8 */  lwc2       $0, 0x0($a1)
    /* 6F598 8007ED98 0400A1C8 */  lwc2       $1, 0x4($a1)
    /* 6F59C 8007ED9C 00000000 */  nop
    /* 6F5A0 8007EDA0 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F5A4 8007EDA4 0000D9E8 */  swc2       $25, 0x0($a2)
    /* 6F5A8 8007EDA8 0400DAE8 */  swc2       $26, 0x4($a2) /* handwritten instruction */
    /* 6F5AC 8007EDAC 0800DBE8 */  swc2       $27, 0x8($a2) /* handwritten instruction */
    /* 6F5B0 8007EDB0 2110C000 */  addu       $v0, $a2, $zero
    /* 6F5B4 8007EDB4 0800E003 */  jr         $ra
    /* 6F5B8 8007EDB8 00000000 */   nop
endlabel func_8007ED6C

nonmatching func_8007EDBC, 0x124

glabel func_8007EDBC
    /* 6F5BC 8007EDBC 0000888C */  lw         $t0, 0x0($a0)
    /* 6F5C0 8007EDC0 0000AB8C */  lw         $t3, 0x0($a1)
    /* 6F5C4 8007EDC4 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F5C8 8007EDC8 004C0900 */  sll        $t1, $t1, 16
    /* 6F5CC 8007EDCC 034C0900 */  sra        $t1, $t1, 16
    /* 6F5D0 8007EDD0 19002B01 */  multu      $t1, $t3
    /* 6F5D4 8007EDD4 0400AC8C */  lw         $t4, 0x4($a1)
    /* 6F5D8 8007EDD8 03540800 */  sra        $t2, $t0, 16
    /* 6F5DC 8007EDDC 0800AD8C */  lw         $t5, 0x8($a1)
    /* 6F5E0 8007EDE0 0400888C */  lw         $t0, 0x4($a0)
    /* 6F5E4 8007EDE4 21108000 */  addu       $v0, $a0, $zero
    /* 6F5E8 8007EDE8 12480000 */  mflo       $t1
    /* 6F5EC 8007EDEC 034B0900 */  sra        $t1, $t1, 12
    /* 6F5F0 8007EDF0 FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F5F4 8007EDF4 19004C01 */  multu      $t2, $t4
    /* 6F5F8 8007EDF8 12500000 */  mflo       $t2
    /* 6F5FC 8007EDFC 03530A00 */  sra        $t2, $t2, 12
    /* 6F600 8007EE00 00540A00 */  sll        $t2, $t2, 16
    /* 6F604 8007EE04 25482A01 */  or         $t1, $t1, $t2
    /* 6F608 8007EE08 000089AC */  sw         $t1, 0x0($a0)
    /* 6F60C 8007EE0C FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F610 8007EE10 004C0900 */  sll        $t1, $t1, 16
    /* 6F614 8007EE14 034C0900 */  sra        $t1, $t1, 16
    /* 6F618 8007EE18 19002D01 */  multu      $t1, $t5
    /* 6F61C 8007EE1C 03540800 */  sra        $t2, $t0, 16
    /* 6F620 8007EE20 0800888C */  lw         $t0, 0x8($a0)
    /* 6F624 8007EE24 12480000 */  mflo       $t1
    /* 6F628 8007EE28 034B0900 */  sra        $t1, $t1, 12
    /* 6F62C 8007EE2C FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F630 8007EE30 19004B01 */  multu      $t2, $t3
    /* 6F634 8007EE34 12500000 */  mflo       $t2
    /* 6F638 8007EE38 03530A00 */  sra        $t2, $t2, 12
    /* 6F63C 8007EE3C 00540A00 */  sll        $t2, $t2, 16
    /* 6F640 8007EE40 25482A01 */  or         $t1, $t1, $t2
    /* 6F644 8007EE44 040089AC */  sw         $t1, 0x4($a0)
    /* 6F648 8007EE48 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F64C 8007EE4C 004C0900 */  sll        $t1, $t1, 16
    /* 6F650 8007EE50 034C0900 */  sra        $t1, $t1, 16
    /* 6F654 8007EE54 19002C01 */  multu      $t1, $t4
    /* 6F658 8007EE58 03540800 */  sra        $t2, $t0, 16
    /* 6F65C 8007EE5C 0C00888C */  lw         $t0, 0xC($a0)
    /* 6F660 8007EE60 12480000 */  mflo       $t1
    /* 6F664 8007EE64 034B0900 */  sra        $t1, $t1, 12
    /* 6F668 8007EE68 FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F66C 8007EE6C 19004D01 */  multu      $t2, $t5
    /* 6F670 8007EE70 12500000 */  mflo       $t2
    /* 6F674 8007EE74 03530A00 */  sra        $t2, $t2, 12
    /* 6F678 8007EE78 00540A00 */  sll        $t2, $t2, 16
    /* 6F67C 8007EE7C 25482A01 */  or         $t1, $t1, $t2
    /* 6F680 8007EE80 080089AC */  sw         $t1, 0x8($a0)
    /* 6F684 8007EE84 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F688 8007EE88 004C0900 */  sll        $t1, $t1, 16
    /* 6F68C 8007EE8C 034C0900 */  sra        $t1, $t1, 16
    /* 6F690 8007EE90 19002B01 */  multu      $t1, $t3
    /* 6F694 8007EE94 03540800 */  sra        $t2, $t0, 16
    /* 6F698 8007EE98 1000888C */  lw         $t0, 0x10($a0)
    /* 6F69C 8007EE9C 12480000 */  mflo       $t1
    /* 6F6A0 8007EEA0 034B0900 */  sra        $t1, $t1, 12
    /* 6F6A4 8007EEA4 FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F6A8 8007EEA8 19004C01 */  multu      $t2, $t4
    /* 6F6AC 8007EEAC 12500000 */  mflo       $t2
    /* 6F6B0 8007EEB0 03530A00 */  sra        $t2, $t2, 12
    /* 6F6B4 8007EEB4 00540A00 */  sll        $t2, $t2, 16
    /* 6F6B8 8007EEB8 25482A01 */  or         $t1, $t1, $t2
    /* 6F6BC 8007EEBC 0C0089AC */  sw         $t1, 0xC($a0)
    /* 6F6C0 8007EEC0 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F6C4 8007EEC4 004C0900 */  sll        $t1, $t1, 16
    /* 6F6C8 8007EEC8 034C0900 */  sra        $t1, $t1, 16
    /* 6F6CC 8007EECC 19002D01 */  multu      $t1, $t5
    /* 6F6D0 8007EED0 12480000 */  mflo       $t1
    /* 6F6D4 8007EED4 034B0900 */  sra        $t1, $t1, 12
    /* 6F6D8 8007EED8 0800E003 */  jr         $ra
    /* 6F6DC 8007EEDC 100089AC */   sw        $t1, 0x10($a0)
endlabel func_8007EDBC
    /* 6F6E0 8007EEE0 00000000 */  nop
    /* 6F6E4 8007EEE4 00000000 */  nop
    /* 6F6E8 8007EEE8 00000000 */  nop

/* Handwritten function */
nonmatching func_8007EEEC, 0x30

glabel func_8007EEEC
    /* 6F6EC 8007EEEC 0000888C */  lw         $t0, 0x0($a0)
    /* 6F6F0 8007EEF0 0400898C */  lw         $t1, 0x4($a0)
    /* 6F6F4 8007EEF4 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F6F8 8007EEF8 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6F6FC 8007EEFC 10008C8C */  lw         $t4, 0x10($a0)
    /* 6F700 8007EF00 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F704 8007EF04 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6F708 8007EF08 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6F70C 8007EF0C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6F710 8007EF10 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6F714 8007EF14 0800E003 */  jr         $ra
    /* 6F718 8007EF18 00000000 */   nop
endlabel func_8007EEEC

/* Handwritten function */
nonmatching func_8007EF1C, 0x30

glabel func_8007EF1C
    /* 6F71C 8007EF1C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F720 8007EF20 0400898C */  lw         $t1, 0x4($a0)
    /* 6F724 8007EF24 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F728 8007EF28 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6F72C 8007EF2C 10008C8C */  lw         $t4, 0x10($a0)
    /* 6F730 8007EF30 0080C848 */  ctc2       $t0, $16 /* handwritten instruction */
    /* 6F734 8007EF34 0088C948 */  ctc2       $t1, $17 /* handwritten instruction */
    /* 6F738 8007EF38 0090CA48 */  ctc2       $t2, $18 /* handwritten instruction */
    /* 6F73C 8007EF3C 0098CB48 */  ctc2       $t3, $19 /* handwritten instruction */
    /* 6F740 8007EF40 00A0CC48 */  ctc2       $t4, $20 /* handwritten instruction */
    /* 6F744 8007EF44 0800E003 */  jr         $ra
    /* 6F748 8007EF48 00000000 */   nop
endlabel func_8007EF1C

/* Handwritten function */
nonmatching func_8007EF4C, 0x20

glabel func_8007EF4C
    /* 6F74C 8007EF4C 1400888C */  lw         $t0, 0x14($a0)
    /* 6F750 8007EF50 1800898C */  lw         $t1, 0x18($a0)
    /* 6F754 8007EF54 1C008A8C */  lw         $t2, 0x1C($a0)
    /* 6F758 8007EF58 0028C848 */  ctc2       $t0, $5 /* handwritten instruction */
    /* 6F75C 8007EF5C 0030C948 */  ctc2       $t1, $6 /* handwritten instruction */
    /* 6F760 8007EF60 0038CA48 */  ctc2       $t2, $7 /* handwritten instruction */
    /* 6F764 8007EF64 0800E003 */  jr         $ra
    /* 6F768 8007EF68 00000000 */   nop
endlabel func_8007EF4C

nonmatching func_8007EF6C, 0x14

glabel func_8007EF6C
    /* 6F76C 8007EF6C 000091E8 */  swc2       $17, 0x0($a0)
    /* 6F770 8007EF70 0000B2E8 */  swc2       $18, 0x0($a1)
    /* 6F774 8007EF74 0000D3E8 */  swc2       $19, 0x0($a2)
    /* 6F778 8007EF78 0800E003 */  jr         $ra
    /* 6F77C 8007EF7C 00000000 */   nop
endlabel func_8007EF6C
    /* 6F780 8007EF80 00000000 */  nop
    /* 6F784 8007EF84 00000000 */  nop
    /* 6F788 8007EF88 00000000 */  nop

/* Handwritten function */
nonmatching func_8007EF8C, 0xC

glabel func_8007EF8C
    /* 6F78C 8007EF8C 00D04248 */  cfc2       $v0, $26 /* handwritten instruction */
    /* 6F790 8007EF90 0800E003 */  jr         $ra
    /* 6F794 8007EF94 00000000 */   nop
endlabel func_8007EF8C
    /* 6F798 8007EF98 00000000 */  nop

/* Handwritten function */
nonmatching func_8007EF9C, 0x20

glabel func_8007EF9C
    /* 6F79C 8007EF9C 00210400 */  sll        $a0, $a0, 4
    /* 6F7A0 8007EFA0 00290500 */  sll        $a1, $a1, 4
    /* 6F7A4 8007EFA4 00310600 */  sll        $a2, $a2, 4
    /* 6F7A8 8007EFA8 0068C448 */  ctc2       $a0, $13 /* handwritten instruction */
    /* 6F7AC 8007EFAC 0070C548 */  ctc2       $a1, $14 /* handwritten instruction */
    /* 6F7B0 8007EFB0 0078C648 */  ctc2       $a2, $15 /* handwritten instruction */
    /* 6F7B4 8007EFB4 0800E003 */  jr         $ra
    /* 6F7B8 8007EFB8 00000000 */   nop
endlabel func_8007EF9C

/* Handwritten function */
nonmatching func_8007EFBC, 0x20

glabel func_8007EFBC
    /* 6F7BC 8007EFBC 00210400 */  sll        $a0, $a0, 4
    /* 6F7C0 8007EFC0 00290500 */  sll        $a1, $a1, 4
    /* 6F7C4 8007EFC4 00310600 */  sll        $a2, $a2, 4
    /* 6F7C8 8007EFC8 00A8C448 */  ctc2       $a0, $21 /* handwritten instruction */
    /* 6F7CC 8007EFCC 00B0C548 */  ctc2       $a1, $22 /* handwritten instruction */
    /* 6F7D0 8007EFD0 00B8C648 */  ctc2       $a2, $23 /* handwritten instruction */
    /* 6F7D4 8007EFD4 0800E003 */  jr         $ra
    /* 6F7D8 8007EFD8 00000000 */   nop
endlabel func_8007EFBC

/* Handwritten function */
nonmatching func_8007EFDC, 0x18

glabel func_8007EFDC
    /* 6F7DC 8007EFDC 00240400 */  sll        $a0, $a0, 16
    /* 6F7E0 8007EFE0 002C0500 */  sll        $a1, $a1, 16
    /* 6F7E4 8007EFE4 00C0C448 */  ctc2       $a0, $24 /* handwritten instruction */
    /* 6F7E8 8007EFE8 00C8C548 */  ctc2       $a1, $25 /* handwritten instruction */
    /* 6F7EC 8007EFEC 0800E003 */  jr         $ra
    /* 6F7F0 8007EFF0 00000000 */   nop
endlabel func_8007EFDC
    /* 6F7F4 8007EFF4 00000000 */  nop
    /* 6F7F8 8007EFF8 00000000 */  nop

/* Handwritten function */
nonmatching func_8007EFFC, 0xC0

glabel func_8007EFFC
    /* 6F7FC 8007EFFC 00D0C448 */  ctc2       $a0, $26 /* handwritten instruction */
    /* 6F800 8007F000 0800E003 */  jr         $ra
    /* 6F804 8007F004 00000000 */   nop
    /* 6F808 8007F008 00000000 */  nop
    /* 6F80C 8007F00C 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6F810 8007F010 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6F814 8007F014 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6F818 8007F018 00000000 */  nop
    /* 6F81C 8007F01C 12A44D4A */  mvmva      1, 2, 3, 1, 1
    /* 6F820 8007F020 0000A9E8 */  swc2       $9, 0x0($a1)
    /* 6F824 8007F024 0400AAE8 */  swc2       $10, 0x4($a1)
    /* 6F828 8007F028 0800ABE8 */  swc2       $11, 0x8($a1)
    /* 6F82C 8007F02C 0800E003 */  jr         $ra
    /* 6F830 8007F030 00000000 */   nop
    /* 6F834 8007F034 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6F838 8007F038 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6F83C 8007F03C 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6F840 8007F040 0000A6C8 */  lwc2       $6, 0x0($a1)
    /* 6F844 8007F044 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6F848 8007F048 00000000 */  nop
    /* 6F84C 8007F04C 2900684A */  dpcl
    /* 6F850 8007F050 0000F6E8 */  swc2       $22, 0x0($a3)
    /* 6F854 8007F054 0800E003 */  jr         $ra
    /* 6F858 8007F058 00000000 */   nop
    /* 6F85C 8007F05C 000094C8 */  lwc2       $20, 0x0($a0)
    /* 6F860 8007F060 0000B5C8 */  lwc2       $21, 0x0($a1)
    /* 6F864 8007F064 0000D6C8 */  lwc2       $22, 0x0($a2)
    /* 6F868 8007F068 0000C6C8 */  lwc2       $6, 0x0($a2)
    /* 6F86C 8007F06C 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6F870 8007F070 00000000 */  nop
    /* 6F874 8007F074 2A00F84A */  dpct
    /* 6F878 8007F078 1000A88F */  lw         $t0, 0x10($sp)
    /* 6F87C 8007F07C 1400A98F */  lw         $t1, 0x14($sp)
    /* 6F880 8007F080 1800AA8F */  lw         $t2, 0x18($sp)
    /* 6F884 8007F084 000014E9 */  swc2       $20, 0x0($t0)
    /* 6F888 8007F088 000035E9 */  swc2       $21, 0x0($t1)
    /* 6F88C 8007F08C 000056E9 */  swc2       $22, 0x0($t2)
    /* 6F890 8007F090 0800E003 */  jr         $ra
    /* 6F894 8007F094 00000000 */   nop
    /* 6F898 8007F098 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6F89C 8007F09C 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6F8A0 8007F0A0 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6F8A4 8007F0A4 00408548 */  mtc2       $a1, $8 /* handwritten instruction */
    /* 6F8A8 8007F0A8 00000000 */  nop
    /* 6F8AC 8007F0AC 1100984A */  intpl
    /* 6F8B0 8007F0B0 0000D6E8 */  swc2       $22, 0x0($a2)
    /* 6F8B4 8007F0B4 0800E003 */  jr         $ra
    /* 6F8B8 8007F0B8 00000000 */   nop
endlabel func_8007EFFC

/* Handwritten function */
nonmatching func_8007F0BC, 0x15C

glabel func_8007F0BC
    /* 6F8BC 8007F0BC 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6F8C0 8007F0C0 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6F8C4 8007F0C4 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6F8C8 8007F0C8 00000000 */  nop
    /* 6F8CC 8007F0CC 2804A84A */  sqr        1
    /* 6F8D0 8007F0D0 0000B9E8 */  swc2       $25, 0x0($a1)
    /* 6F8D4 8007F0D4 0400BAE8 */  swc2       $26, 0x4($a1) /* handwritten instruction */
    /* 6F8D8 8007F0D8 0800BBE8 */  swc2       $27, 0x8($a1) /* handwritten instruction */
    /* 6F8DC 8007F0DC 0800E003 */  jr         $ra
    /* 6F8E0 8007F0E0 2110A000 */   addu      $v0, $a1, $zero
    /* 6F8E4 8007F0E4 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6F8E8 8007F0E8 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6F8EC 8007F0EC 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6F8F0 8007F0F0 00000000 */  nop
    /* 6F8F4 8007F0F4 2804A04A */  sqr        0
    /* 6F8F8 8007F0F8 0000B9E8 */  swc2       $25, 0x0($a1)
    /* 6F8FC 8007F0FC 0400BAE8 */  swc2       $26, 0x4($a1) /* handwritten instruction */
    /* 6F900 8007F100 0800BBE8 */  swc2       $27, 0x8($a1) /* handwritten instruction */
    /* 6F904 8007F104 0800E003 */  jr         $ra
    /* 6F908 8007F108 2110A000 */   addu      $v0, $a1, $zero
    /* 6F90C 8007F10C 00888448 */  mtc2       $a0, $17 /* handwritten instruction */
    /* 6F910 8007F110 00908548 */  mtc2       $a1, $18 /* handwritten instruction */
    /* 6F914 8007F114 00988648 */  mtc2       $a2, $19 /* handwritten instruction */
    /* 6F918 8007F118 00000000 */  nop
    /* 6F91C 8007F11C 2D00584B */  avsz3
    /* 6F920 8007F120 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 6F924 8007F124 0800E003 */  jr         $ra
    /* 6F928 8007F128 00000000 */   nop
    /* 6F92C 8007F12C 00808448 */  mtc2       $a0, $16 /* handwritten instruction */
    /* 6F930 8007F130 00888548 */  mtc2       $a1, $17 /* handwritten instruction */
    /* 6F934 8007F134 00908648 */  mtc2       $a2, $18 /* handwritten instruction */
    /* 6F938 8007F138 00988748 */  mtc2       $a3, $19 /* handwritten instruction */
    /* 6F93C 8007F13C 00000000 */  nop
    /* 6F940 8007F140 2E00684B */  avsz4
    /* 6F944 8007F144 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 6F948 8007F148 0800E003 */  jr         $ra
    /* 6F94C 8007F14C 00000000 */   nop
    /* 6F950 8007F150 00004D48 */  cfc2       $t5, $0 /* handwritten instruction */
    /* 6F954 8007F154 00104E48 */  cfc2       $t6, $2 /* handwritten instruction */
    /* 6F958 8007F158 00204F48 */  cfc2       $t7, $4 /* handwritten instruction */
    /* 6F95C 8007F15C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F960 8007F160 0400898C */  lw         $t1, 0x4($a0)
    /* 6F964 8007F164 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F968 8007F168 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F96C 8007F16C 0010C948 */  ctc2       $t1, $2 /* handwritten instruction */
    /* 6F970 8007F170 0020CA48 */  ctc2       $t2, $4 /* handwritten instruction */
    /* 6F974 8007F174 0800ABC8 */  lwc2       $11, 0x8($a1)
    /* 6F978 8007F178 0000A9C8 */  lwc2       $9, 0x0($a1)
    /* 6F97C 8007F17C 0400AAC8 */  lwc2       $10, 0x4($a1)
    /* 6F980 8007F180 00000000 */  nop
    /* 6F984 8007F184 0C00784B */  op         1
    /* 6F988 8007F188 0000D9E8 */  swc2       $25, 0x0($a2)
    /* 6F98C 8007F18C 0400DAE8 */  swc2       $26, 0x4($a2) /* handwritten instruction */
    /* 6F990 8007F190 0800DBE8 */  swc2       $27, 0x8($a2) /* handwritten instruction */
    /* 6F994 8007F194 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 6F998 8007F198 0010CE48 */  ctc2       $t6, $2 /* handwritten instruction */
    /* 6F99C 8007F19C 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 6F9A0 8007F1A0 0800E003 */  jr         $ra
    /* 6F9A4 8007F1A4 00000000 */   nop
    /* 6F9A8 8007F1A8 00004D48 */  cfc2       $t5, $0 /* handwritten instruction */
    /* 6F9AC 8007F1AC 00104E48 */  cfc2       $t6, $2 /* handwritten instruction */
    /* 6F9B0 8007F1B0 00204F48 */  cfc2       $t7, $4 /* handwritten instruction */
    /* 6F9B4 8007F1B4 0000888C */  lw         $t0, 0x0($a0)
    /* 6F9B8 8007F1B8 0400898C */  lw         $t1, 0x4($a0)
    /* 6F9BC 8007F1BC 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F9C0 8007F1C0 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F9C4 8007F1C4 0010C948 */  ctc2       $t1, $2 /* handwritten instruction */
    /* 6F9C8 8007F1C8 0020CA48 */  ctc2       $t2, $4 /* handwritten instruction */
    /* 6F9CC 8007F1CC 0800ABC8 */  lwc2       $11, 0x8($a1)
    /* 6F9D0 8007F1D0 0000A9C8 */  lwc2       $9, 0x0($a1)
    /* 6F9D4 8007F1D4 0400AAC8 */  lwc2       $10, 0x4($a1)
    /* 6F9D8 8007F1D8 00000000 */  nop
    /* 6F9DC 8007F1DC 0C00704B */  op         0
    /* 6F9E0 8007F1E0 0000D9E8 */  swc2       $25, 0x0($a2)
    /* 6F9E4 8007F1E4 0400DAE8 */  swc2       $26, 0x4($a2) /* handwritten instruction */
    /* 6F9E8 8007F1E8 0800DBE8 */  swc2       $27, 0x8($a2) /* handwritten instruction */
    /* 6F9EC 8007F1EC 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 6F9F0 8007F1F0 0010CE48 */  ctc2       $t6, $2 /* handwritten instruction */
    /* 6F9F4 8007F1F4 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 6F9F8 8007F1F8 0800E003 */  jr         $ra
    /* 6F9FC 8007F1FC 00000000 */   nop
    /* 6FA00 8007F200 00F08448 */  mtc2       $a0, $30 /* handwritten instruction */
    /* 6FA04 8007F204 00000000 */  nop
    /* 6FA08 8007F208 00000000 */  nop
    /* 6FA0C 8007F20C 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6FA10 8007F210 0800E003 */  jr         $ra
    /* 6FA14 8007F214 00000000 */   nop
endlabel func_8007F0BC
    /* 6FA18 8007F218 00000000 */  nop

/* Handwritten function */
nonmatching func_8007F21C, 0x2C

glabel func_8007F21C
    /* 6FA1C 8007F21C 000080C8 */  lwc2       $0, 0x0($a0)
    /* 6FA20 8007F220 040081C8 */  lwc2       $1, 0x4($a0)
    /* 6FA24 8007F224 00000000 */  nop
    /* 6FA28 8007F228 0100184A */  rtps
    /* 6FA2C 8007F22C 0000AEE8 */  swc2       $14, 0x0($a1)
    /* 6FA30 8007F230 0000C8E8 */  swc2       $8, 0x0($a2)
    /* 6FA34 8007F234 00F84348 */  cfc2       $v1, $31 /* handwritten instruction */
    /* 6FA38 8007F238 00980248 */  mfc2       $v0, $19 /* handwritten instruction */
    /* 6FA3C 8007F23C 0000E3AC */  sw         $v1, 0x0($a3)
    /* 6FA40 8007F240 0800E003 */  jr         $ra
    /* 6FA44 8007F244 83100200 */   sra       $v0, $v0, 2
endlabel func_8007F21C
    /* 6FA48 8007F248 00000000 */  nop

/* Handwritten function */
nonmatching func_8007F24C, 0x54

glabel func_8007F24C
    /* 6FA4C 8007F24C 000080C8 */  lwc2       $0, 0x0($a0)
    /* 6FA50 8007F250 040081C8 */  lwc2       $1, 0x4($a0)
    /* 6FA54 8007F254 0000A2C8 */  lwc2       $2, 0x0($a1)
    /* 6FA58 8007F258 0400A3C8 */  lwc2       $3, 0x4($a1)
    /* 6FA5C 8007F25C 0000C4C8 */  lwc2       $4, 0x0($a2)
    /* 6FA60 8007F260 0400C5C8 */  lwc2       $5, 0x4($a2)
    /* 6FA64 8007F264 00000000 */  nop
    /* 6FA68 8007F268 3000284A */  rtpt
    /* 6FA6C 8007F26C 1000A88F */  lw         $t0, 0x10($sp)
    /* 6FA70 8007F270 1400A98F */  lw         $t1, 0x14($sp)
    /* 6FA74 8007F274 1800AA8F */  lw         $t2, 0x18($sp)
    /* 6FA78 8007F278 1C00AB8F */  lw         $t3, 0x1C($sp)
    /* 6FA7C 8007F27C 0000ECE8 */  swc2       $12, 0x0($a3)
    /* 6FA80 8007F280 00000DE9 */  swc2       $13, 0x0($t0)
    /* 6FA84 8007F284 00002EE9 */  swc2       $14, 0x0($t1)
    /* 6FA88 8007F288 000048E9 */  swc2       $8, 0x0($t2)
    /* 6FA8C 8007F28C 00F84348 */  cfc2       $v1, $31 /* handwritten instruction */
    /* 6FA90 8007F290 00980248 */  mfc2       $v0, $19 /* handwritten instruction */
    /* 6FA94 8007F294 000063AD */  sw         $v1, 0x0($t3)
    /* 6FA98 8007F298 0800E003 */  jr         $ra
    /* 6FA9C 8007F29C 83100200 */   sra       $v0, $v0, 2
endlabel func_8007F24C
    /* 6FAA0 8007F2A0 00000000 */  nop
    /* 6FAA4 8007F2A4 00000000 */  nop
    /* 6FAA8 8007F2A8 00000000 */  nop

/* Handwritten function */
nonmatching func_8007F2AC, 0x28

glabel func_8007F2AC
    /* 6FAAC 8007F2AC 000080C8 */  lwc2       $0, 0x0($a0)
    /* 6FAB0 8007F2B0 040081C8 */  lwc2       $1, 0x4($a0)
    /* 6FAB4 8007F2B4 00000000 */  nop
    /* 6FAB8 8007F2B8 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 6FABC 8007F2BC 0000B9E8 */  swc2       $25, 0x0($a1)
    /* 6FAC0 8007F2C0 0400BAE8 */  swc2       $26, 0x4($a1) /* handwritten instruction */
    /* 6FAC4 8007F2C4 0800BBE8 */  swc2       $27, 0x8($a1) /* handwritten instruction */
    /* 6FAC8 8007F2C8 00F84248 */  cfc2       $v0, $31 /* handwritten instruction */
    /* 6FACC 8007F2CC 0800E003 */  jr         $ra
    /* 6FAD0 8007F2D0 0000C2AC */   sw        $v0, 0x0($a2)
endlabel func_8007F2AC
    /* 6FAD4 8007F2D4 00000000 */  nop
    /* 6FAD8 8007F2D8 00000000 */  nop

/* Handwritten function */
nonmatching func_8007F2DC, 0x78

glabel func_8007F2DC
    /* 6FADC 8007F2DC 000080C8 */  lwc2       $0, 0x0($a0)
    /* 6FAE0 8007F2E0 040081C8 */  lwc2       $1, 0x4($a0)
    /* 6FAE4 8007F2E4 0000A2C8 */  lwc2       $2, 0x0($a1)
    /* 6FAE8 8007F2E8 0400A3C8 */  lwc2       $3, 0x4($a1)
    /* 6FAEC 8007F2EC 0000C4C8 */  lwc2       $4, 0x0($a2)
    /* 6FAF0 8007F2F0 0400C5C8 */  lwc2       $5, 0x4($a2)
    /* 6FAF4 8007F2F4 00000000 */  nop
    /* 6FAF8 8007F2F8 3000284A */  rtpt
    /* 6FAFC 8007F2FC 1000A88F */  lw         $t0, 0x10($sp)
    /* 6FB00 8007F300 1400A98F */  lw         $t1, 0x14($sp)
    /* 6FB04 8007F304 1800AA8F */  lw         $t2, 0x18($sp)
    /* 6FB08 8007F308 00000CE9 */  swc2       $12, 0x0($t0)
    /* 6FB0C 8007F30C 00002DE9 */  swc2       $13, 0x0($t1)
    /* 6FB10 8007F310 00004EE9 */  swc2       $14, 0x0($t2)
    /* 6FB14 8007F314 00F84348 */  cfc2       $v1, $31 /* handwritten instruction */
    /* 6FB18 8007F318 0000E0C8 */  lwc2       $0, 0x0($a3)
    /* 6FB1C 8007F31C 0400E1C8 */  lwc2       $1, 0x4($a3)
    /* 6FB20 8007F320 00000000 */  nop
    /* 6FB24 8007F324 0100184A */  rtps
    /* 6FB28 8007F328 1C00A88F */  lw         $t0, 0x1C($sp)
    /* 6FB2C 8007F32C 2000A98F */  lw         $t1, 0x20($sp)
    /* 6FB30 8007F330 2400AA8F */  lw         $t2, 0x24($sp)
    /* 6FB34 8007F334 00000EE9 */  swc2       $14, 0x0($t0)
    /* 6FB38 8007F338 000028E9 */  swc2       $8, 0x0($t1)
    /* 6FB3C 8007F33C 00F84848 */  cfc2       $t0, $31 /* handwritten instruction */
    /* 6FB40 8007F340 00980248 */  mfc2       $v0, $19 /* handwritten instruction */
    /* 6FB44 8007F344 25400301 */  or         $t0, $t0, $v1
    /* 6FB48 8007F348 000048AD */  sw         $t0, 0x0($t2)
    /* 6FB4C 8007F34C 0800E003 */  jr         $ra
    /* 6FB50 8007F350 83100200 */   sra       $v0, $v0, 2
endlabel func_8007F2DC
    /* 6FB54 8007F354 00000000 */  nop
    /* 6FB58 8007F358 00000000 */  nop

nonmatching func_8007F35C, 0x28C

glabel func_8007F35C
    /* 6FB5C 8007F35C 00008F84 */  lh         $t7, 0x0($a0)
    /* 6FB60 8007F360 2110A000 */  addu       $v0, $a1, $zero
    /* 6FB64 8007F364 0E00E105 */  bgez       $t7, .L8007F3A0
    /* 6FB68 8007F368 FF0FF931 */   andi      $t9, $t7, 0xFFF
    /* 6FB6C 8007F36C 23780F00 */  negu       $t7, $t7
    /* 6FB70 8007F370 0100E105 */  bgez       $t7, .L8007F378
    /* 6FB74 8007F374 FF0FEF31 */   andi      $t7, $t7, 0xFFF
  .L8007F378:
    /* 6FB78 8007F378 80C00F00 */  sll        $t8, $t7, 2
    /* 6FB7C 8007F37C 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FB80 8007F380 21C83803 */  addu       $t9, $t9, $t8
    /* 6FB84 8007F384 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FB88 8007F388 00000000 */  nop
    /* 6FB8C 8007F38C 00C41900 */  sll        $t8, $t9, 16
    /* 6FB90 8007F390 03C41800 */  sra        $t8, $t8, 16
    /* 6FB94 8007F394 23581800 */  negu       $t3, $t8
    /* 6FB98 8007F398 F0FC0108 */  j          .L8007F3C0
    /* 6FB9C 8007F39C 03441900 */   sra       $t0, $t9, 16
  .L8007F3A0:
    /* 6FBA0 8007F3A0 80C01900 */  sll        $t8, $t9, 2
    /* 6FBA4 8007F3A4 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FBA8 8007F3A8 21C83803 */  addu       $t9, $t9, $t8
    /* 6FBAC 8007F3AC 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FBB0 8007F3B0 00000000 */  nop
    /* 6FBB4 8007F3B4 00C41900 */  sll        $t8, $t9, 16
    /* 6FBB8 8007F3B8 035C1800 */  sra        $t3, $t8, 16
    /* 6FBBC 8007F3BC 03441900 */  sra        $t0, $t9, 16
  .L8007F3C0:
    /* 6FBC0 8007F3C0 02008F84 */  lh         $t7, 0x2($a0)
    /* 6FBC4 8007F3C4 00000000 */  nop
    /* 6FBC8 8007F3C8 0E00E105 */  bgez       $t7, .L8007F404
    /* 6FBCC 8007F3CC FF0FF931 */   andi      $t9, $t7, 0xFFF
    /* 6FBD0 8007F3D0 23780F00 */  negu       $t7, $t7
    /* 6FBD4 8007F3D4 0100E105 */  bgez       $t7, .L8007F3DC
    /* 6FBD8 8007F3D8 FF0FEF31 */   andi      $t7, $t7, 0xFFF
  .L8007F3DC:
    /* 6FBDC 8007F3DC 80C00F00 */  sll        $t8, $t7, 2
    /* 6FBE0 8007F3E0 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FBE4 8007F3E4 21C83803 */  addu       $t9, $t9, $t8
    /* 6FBE8 8007F3E8 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FBEC 8007F3EC 00000000 */  nop
    /* 6FBF0 8007F3F0 00641900 */  sll        $t4, $t9, 16
    /* 6FBF4 8007F3F4 03640C00 */  sra        $t4, $t4, 16
    /* 6FBF8 8007F3F8 23700C00 */  negu       $t6, $t4
    /* 6FBFC 8007F3FC 0AFD0108 */  j          .L8007F428
    /* 6FC00 8007F400 034C1900 */   sra       $t1, $t9, 16
  .L8007F404:
    /* 6FC04 8007F404 80C01900 */  sll        $t8, $t9, 2
    /* 6FC08 8007F408 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FC0C 8007F40C 21C83803 */  addu       $t9, $t9, $t8
    /* 6FC10 8007F410 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FC14 8007F414 00000000 */  nop
    /* 6FC18 8007F418 00741900 */  sll        $t6, $t9, 16
    /* 6FC1C 8007F41C 03740E00 */  sra        $t6, $t6, 16
    /* 6FC20 8007F420 23600E00 */  negu       $t4, $t6
    /* 6FC24 8007F424 034C1900 */  sra        $t1, $t9, 16
  .L8007F428:
    /* 6FC28 8007F428 19002B01 */  multu      $t1, $t3
    /* 6FC2C 8007F42C 04008F84 */  lh         $t7, 0x4($a0)
    /* 6FC30 8007F430 0400AEA4 */  sh         $t6, 0x4($a1)
    /* 6FC34 8007F434 12C00000 */  mflo       $t8
    /* 6FC38 8007F438 23C81800 */  negu       $t9, $t8
    /* 6FC3C 8007F43C 03731900 */  sra        $t6, $t9, 12
    /* 6FC40 8007F440 19002801 */  multu      $t1, $t0
    /* 6FC44 8007F444 0A00AEA4 */  sh         $t6, 0xA($a1)
    /* 6FC48 8007F448 1100E105 */  bgez       $t7, .L8007F490
    /* 6FC4C 8007F44C FF0FF931 */   andi      $t9, $t7, 0xFFF
    /* 6FC50 8007F450 12C00000 */  mflo       $t8
    /* 6FC54 8007F454 03731800 */  sra        $t6, $t8, 12
    /* 6FC58 8007F458 1000AEA4 */  sh         $t6, 0x10($a1)
    /* 6FC5C 8007F45C 23780F00 */  negu       $t7, $t7
    /* 6FC60 8007F460 0100E105 */  bgez       $t7, .L8007F468
    /* 6FC64 8007F464 FF0FEF31 */   andi      $t7, $t7, 0xFFF
  .L8007F468:
    /* 6FC68 8007F468 80C00F00 */  sll        $t8, $t7, 2
    /* 6FC6C 8007F46C 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FC70 8007F470 21C83803 */  addu       $t9, $t9, $t8
    /* 6FC74 8007F474 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FC78 8007F478 00000000 */  nop
    /* 6FC7C 8007F47C 00C41900 */  sll        $t8, $t9, 16
    /* 6FC80 8007F480 03C41800 */  sra        $t8, $t8, 16
    /* 6FC84 8007F484 23681800 */  negu       $t5, $t8
    /* 6FC88 8007F488 2FFD0108 */  j          .L8007F4BC
    /* 6FC8C 8007F48C 03541900 */   sra       $t2, $t9, 16
  .L8007F490:
    /* 6FC90 8007F490 12780000 */  mflo       $t7
    /* 6FC94 8007F494 03730F00 */  sra        $t6, $t7, 12
    /* 6FC98 8007F498 1000AEA4 */  sh         $t6, 0x10($a1)
    /* 6FC9C 8007F49C 80C01900 */  sll        $t8, $t9, 2
    /* 6FCA0 8007F4A0 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FCA4 8007F4A4 21C83803 */  addu       $t9, $t9, $t8
    /* 6FCA8 8007F4A8 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FCAC 8007F4AC 00000000 */  nop
    /* 6FCB0 8007F4B0 00C41900 */  sll        $t8, $t9, 16
    /* 6FCB4 8007F4B4 036C1800 */  sra        $t5, $t8, 16
    /* 6FCB8 8007F4B8 03541900 */  sra        $t2, $t9, 16
  .L8007F4BC:
    /* 6FCBC 8007F4BC 19004901 */  multu      $t2, $t1
    /* 6FCC0 8007F4C0 00000000 */  nop
    /* 6FCC4 8007F4C4 00000000 */  nop
    /* 6FCC8 8007F4C8 12780000 */  mflo       $t7
    /* 6FCCC 8007F4CC 03730F00 */  sra        $t6, $t7, 12
    /* 6FCD0 8007F4D0 0000AEA4 */  sh         $t6, 0x0($a1)
    /* 6FCD4 8007F4D4 1900A901 */  multu      $t5, $t1
    /* 6FCD8 8007F4D8 00000000 */  nop
    /* 6FCDC 8007F4DC 00000000 */  nop
    /* 6FCE0 8007F4E0 12780000 */  mflo       $t7
    /* 6FCE4 8007F4E4 23700F00 */  negu       $t6, $t7
    /* 6FCE8 8007F4E8 037B0E00 */  sra        $t7, $t6, 12
    /* 6FCEC 8007F4EC 19004C01 */  multu      $t2, $t4
    /* 6FCF0 8007F4F0 0200AFA4 */  sh         $t7, 0x2($a1)
    /* 6FCF4 8007F4F4 00000000 */  nop
    /* 6FCF8 8007F4F8 12780000 */  mflo       $t7
    /* 6FCFC 8007F4FC 03C30F00 */  sra        $t8, $t7, 12
    /* 6FD00 8007F500 00000000 */  nop
    /* 6FD04 8007F504 19000B03 */  multu      $t8, $t3
    /* 6FD08 8007F508 00000000 */  nop
    /* 6FD0C 8007F50C 00000000 */  nop
    /* 6FD10 8007F510 12780000 */  mflo       $t7
    /* 6FD14 8007F514 03730F00 */  sra        $t6, $t7, 12
    /* 6FD18 8007F518 00000000 */  nop
    /* 6FD1C 8007F51C 1900A801 */  multu      $t5, $t0
    /* 6FD20 8007F520 00000000 */  nop
    /* 6FD24 8007F524 00000000 */  nop
    /* 6FD28 8007F528 12780000 */  mflo       $t7
    /* 6FD2C 8007F52C 03CB0F00 */  sra        $t9, $t7, 12
    /* 6FD30 8007F530 23782E03 */  subu       $t7, $t9, $t6
    /* 6FD34 8007F534 19000803 */  multu      $t8, $t0
    /* 6FD38 8007F538 0600AFA4 */  sh         $t7, 0x6($a1)
    /* 6FD3C 8007F53C 00000000 */  nop
    /* 6FD40 8007F540 12700000 */  mflo       $t6
    /* 6FD44 8007F544 037B0E00 */  sra        $t7, $t6, 12
    /* 6FD48 8007F548 00000000 */  nop
    /* 6FD4C 8007F54C 1900AB01 */  multu      $t5, $t3
    /* 6FD50 8007F550 00000000 */  nop
    /* 6FD54 8007F554 00000000 */  nop
    /* 6FD58 8007F558 12700000 */  mflo       $t6
    /* 6FD5C 8007F55C 03CB0E00 */  sra        $t9, $t6, 12
    /* 6FD60 8007F560 21702F03 */  addu       $t6, $t9, $t7
    /* 6FD64 8007F564 1900AC01 */  multu      $t5, $t4
    /* 6FD68 8007F568 0C00AEA4 */  sh         $t6, 0xC($a1)
    /* 6FD6C 8007F56C 00000000 */  nop
    /* 6FD70 8007F570 12780000 */  mflo       $t7
    /* 6FD74 8007F574 03C30F00 */  sra        $t8, $t7, 12
    /* 6FD78 8007F578 00000000 */  nop
    /* 6FD7C 8007F57C 19000B03 */  multu      $t8, $t3
    /* 6FD80 8007F580 00000000 */  nop
    /* 6FD84 8007F584 00000000 */  nop
    /* 6FD88 8007F588 12780000 */  mflo       $t7
    /* 6FD8C 8007F58C 03730F00 */  sra        $t6, $t7, 12
    /* 6FD90 8007F590 00000000 */  nop
    /* 6FD94 8007F594 19004801 */  multu      $t2, $t0
    /* 6FD98 8007F598 00000000 */  nop
    /* 6FD9C 8007F59C 00000000 */  nop
    /* 6FDA0 8007F5A0 12780000 */  mflo       $t7
    /* 6FDA4 8007F5A4 03CB0F00 */  sra        $t9, $t7, 12
    /* 6FDA8 8007F5A8 21782E03 */  addu       $t7, $t9, $t6
    /* 6FDAC 8007F5AC 19000803 */  multu      $t8, $t0
    /* 6FDB0 8007F5B0 0800AFA4 */  sh         $t7, 0x8($a1)
    /* 6FDB4 8007F5B4 00000000 */  nop
    /* 6FDB8 8007F5B8 12700000 */  mflo       $t6
    /* 6FDBC 8007F5BC 037B0E00 */  sra        $t7, $t6, 12
    /* 6FDC0 8007F5C0 00000000 */  nop
    /* 6FDC4 8007F5C4 19004B01 */  multu      $t2, $t3
    /* 6FDC8 8007F5C8 00000000 */  nop
    /* 6FDCC 8007F5CC 00000000 */  nop
    /* 6FDD0 8007F5D0 12700000 */  mflo       $t6
    /* 6FDD4 8007F5D4 03CB0E00 */  sra        $t9, $t6, 12
    /* 6FDD8 8007F5D8 23702F03 */  subu       $t6, $t9, $t7
    /* 6FDDC 8007F5DC 0E00AEA4 */  sh         $t6, 0xE($a1)
    /* 6FDE0 8007F5E0 0800E003 */  jr         $ra
    /* 6FDE4 8007F5E4 00000000 */   nop
endlabel func_8007F35C
    /* 6FDE8 8007F5E8 00000000 */  nop

nonmatching func_8007F5EC, 0x28C

glabel func_8007F5EC
    /* 6FDEC 8007F5EC 00008F84 */  lh         $t7, 0x0($a0)
    /* 6FDF0 8007F5F0 2110A000 */  addu       $v0, $a1, $zero
    /* 6FDF4 8007F5F4 0E00E105 */  bgez       $t7, .L8007F630
    /* 6FDF8 8007F5F8 FF0FF931 */   andi      $t9, $t7, 0xFFF
    /* 6FDFC 8007F5FC 23780F00 */  negu       $t7, $t7
    /* 6FE00 8007F600 0100E105 */  bgez       $t7, .L8007F608
    /* 6FE04 8007F604 FF0FEF31 */   andi      $t7, $t7, 0xFFF
  .L8007F608:
    /* 6FE08 8007F608 80C00F00 */  sll        $t8, $t7, 2
    /* 6FE0C 8007F60C 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FE10 8007F610 21C83803 */  addu       $t9, $t9, $t8
    /* 6FE14 8007F614 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FE18 8007F618 00000000 */  nop
    /* 6FE1C 8007F61C 00741900 */  sll        $t6, $t9, 16
    /* 6FE20 8007F620 03740E00 */  sra        $t6, $t6, 16
    /* 6FE24 8007F624 23580E00 */  negu       $t3, $t6
    /* 6FE28 8007F628 94FD0108 */  j          .L8007F650
    /* 6FE2C 8007F62C 03441900 */   sra       $t0, $t9, 16
  .L8007F630:
    /* 6FE30 8007F630 80C01900 */  sll        $t8, $t9, 2
    /* 6FE34 8007F634 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FE38 8007F638 21C83803 */  addu       $t9, $t9, $t8
    /* 6FE3C 8007F63C 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FE40 8007F640 00000000 */  nop
    /* 6FE44 8007F644 00C41900 */  sll        $t8, $t9, 16
    /* 6FE48 8007F648 035C1800 */  sra        $t3, $t8, 16
    /* 6FE4C 8007F64C 03441900 */  sra        $t0, $t9, 16
  .L8007F650:
    /* 6FE50 8007F650 02008F84 */  lh         $t7, 0x2($a0)
    /* 6FE54 8007F654 00000000 */  nop
    /* 6FE58 8007F658 0E00E105 */  bgez       $t7, .L8007F694
    /* 6FE5C 8007F65C FF0FF931 */   andi      $t9, $t7, 0xFFF
    /* 6FE60 8007F660 23780F00 */  negu       $t7, $t7
    /* 6FE64 8007F664 0100E105 */  bgez       $t7, .L8007F66C
    /* 6FE68 8007F668 FF0FEF31 */   andi      $t7, $t7, 0xFFF
  .L8007F66C:
    /* 6FE6C 8007F66C 80C00F00 */  sll        $t8, $t7, 2
    /* 6FE70 8007F670 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FE74 8007F674 21C83803 */  addu       $t9, $t9, $t8
    /* 6FE78 8007F678 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FE7C 8007F67C 00000000 */  nop
    /* 6FE80 8007F680 00741900 */  sll        $t6, $t9, 16
    /* 6FE84 8007F684 03740E00 */  sra        $t6, $t6, 16
    /* 6FE88 8007F688 23600E00 */  negu       $t4, $t6
    /* 6FE8C 8007F68C AEFD0108 */  j          .L8007F6B8
    /* 6FE90 8007F690 034C1900 */   sra       $t1, $t9, 16
  .L8007F694:
    /* 6FE94 8007F694 80C01900 */  sll        $t8, $t9, 2
    /* 6FE98 8007F698 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FE9C 8007F69C 21C83803 */  addu       $t9, $t9, $t8
    /* 6FEA0 8007F6A0 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FEA4 8007F6A4 00000000 */  nop
    /* 6FEA8 8007F6A8 00741900 */  sll        $t6, $t9, 16
    /* 6FEAC 8007F6AC 03640E00 */  sra        $t4, $t6, 16
    /* 6FEB0 8007F6B0 23700C00 */  negu       $t6, $t4
    /* 6FEB4 8007F6B4 034C1900 */  sra        $t1, $t9, 16
  .L8007F6B8:
    /* 6FEB8 8007F6B8 19006901 */  multu      $t3, $t1
    /* 6FEBC 8007F6BC 04008F84 */  lh         $t7, 0x4($a0)
    /* 6FEC0 8007F6C0 0C00AEA4 */  sh         $t6, 0xC($a1)
    /* 6FEC4 8007F6C4 12C00000 */  mflo       $t8
    /* 6FEC8 8007F6C8 03731800 */  sra        $t6, $t8, 12
    /* 6FECC 8007F6CC 00000000 */  nop
    /* 6FED0 8007F6D0 19000901 */  multu      $t0, $t1
    /* 6FED4 8007F6D4 0E00AEA4 */  sh         $t6, 0xE($a1)
    /* 6FED8 8007F6D8 1100E105 */  bgez       $t7, .L8007F720
    /* 6FEDC 8007F6DC FF0FF931 */   andi      $t9, $t7, 0xFFF
    /* 6FEE0 8007F6E0 12C00000 */  mflo       $t8
    /* 6FEE4 8007F6E4 03731800 */  sra        $t6, $t8, 12
    /* 6FEE8 8007F6E8 1000AEA4 */  sh         $t6, 0x10($a1)
    /* 6FEEC 8007F6EC 23780F00 */  negu       $t7, $t7
    /* 6FEF0 8007F6F0 0100E105 */  bgez       $t7, .L8007F6F8
    /* 6FEF4 8007F6F4 FF0FEF31 */   andi      $t7, $t7, 0xFFF
  .L8007F6F8:
    /* 6FEF8 8007F6F8 80C00F00 */  sll        $t8, $t7, 2
    /* 6FEFC 8007F6FC 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FF00 8007F700 21C83803 */  addu       $t9, $t9, $t8
    /* 6FF04 8007F704 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FF08 8007F708 00000000 */  nop
    /* 6FF0C 8007F70C 00C41900 */  sll        $t8, $t9, 16
    /* 6FF10 8007F710 03C41800 */  sra        $t8, $t8, 16
    /* 6FF14 8007F714 23681800 */  negu       $t5, $t8
    /* 6FF18 8007F718 D3FD0108 */  j          .L8007F74C
    /* 6FF1C 8007F71C 03541900 */   sra       $t2, $t9, 16
  .L8007F720:
    /* 6FF20 8007F720 12780000 */  mflo       $t7
    /* 6FF24 8007F724 03730F00 */  sra        $t6, $t7, 12
    /* 6FF28 8007F728 1000AEA4 */  sh         $t6, 0x10($a1)
    /* 6FF2C 8007F72C 80C01900 */  sll        $t8, $t9, 2
    /* 6FF30 8007F730 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 6FF34 8007F734 21C83803 */  addu       $t9, $t9, $t8
    /* 6FF38 8007F738 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 6FF3C 8007F73C 00000000 */  nop
    /* 6FF40 8007F740 00C41900 */  sll        $t8, $t9, 16
    /* 6FF44 8007F744 036C1800 */  sra        $t5, $t8, 16
    /* 6FF48 8007F748 03541900 */  sra        $t2, $t9, 16
  .L8007F74C:
    /* 6FF4C 8007F74C 19002A01 */  multu      $t1, $t2
    /* 6FF50 8007F750 00000000 */  nop
    /* 6FF54 8007F754 00000000 */  nop
    /* 6FF58 8007F758 12780000 */  mflo       $t7
    /* 6FF5C 8007F75C 03730F00 */  sra        $t6, $t7, 12
    /* 6FF60 8007F760 0000AEA4 */  sh         $t6, 0x0($a1)
    /* 6FF64 8007F764 1900A901 */  multu      $t5, $t1
    /* 6FF68 8007F768 00000000 */  nop
    /* 6FF6C 8007F76C 00000000 */  nop
    /* 6FF70 8007F770 12780000 */  mflo       $t7
    /* 6FF74 8007F774 03730F00 */  sra        $t6, $t7, 12
    /* 6FF78 8007F778 00000000 */  nop
    /* 6FF7C 8007F77C 19006C01 */  multu      $t3, $t4
    /* 6FF80 8007F780 0600AEA4 */  sh         $t6, 0x6($a1)
    /* 6FF84 8007F784 00000000 */  nop
    /* 6FF88 8007F788 12780000 */  mflo       $t7
    /* 6FF8C 8007F78C 03C30F00 */  sra        $t8, $t7, 12
    /* 6FF90 8007F790 00000000 */  nop
    /* 6FF94 8007F794 19000A03 */  multu      $t8, $t2
    /* 6FF98 8007F798 00000000 */  nop
    /* 6FF9C 8007F79C 00000000 */  nop
    /* 6FFA0 8007F7A0 12780000 */  mflo       $t7
    /* 6FFA4 8007F7A4 03730F00 */  sra        $t6, $t7, 12
    /* 6FFA8 8007F7A8 00000000 */  nop
    /* 6FFAC 8007F7AC 1900A801 */  multu      $t5, $t0
    /* 6FFB0 8007F7B0 00000000 */  nop
    /* 6FFB4 8007F7B4 00000000 */  nop
    /* 6FFB8 8007F7B8 12780000 */  mflo       $t7
    /* 6FFBC 8007F7BC 03CB0F00 */  sra        $t9, $t7, 12
    /* 6FFC0 8007F7C0 2378D901 */  subu       $t7, $t6, $t9
    /* 6FFC4 8007F7C4 19000A01 */  multu      $t0, $t2
    /* 6FFC8 8007F7C8 0200AFA4 */  sh         $t7, 0x2($a1)
    /* 6FFCC 8007F7CC 00000000 */  nop
    /* 6FFD0 8007F7D0 12700000 */  mflo       $t6
    /* 6FFD4 8007F7D4 037B0E00 */  sra        $t7, $t6, 12
    /* 6FFD8 8007F7D8 00000000 */  nop
    /* 6FFDC 8007F7DC 19000D03 */  multu      $t8, $t5
    /* 6FFE0 8007F7E0 00000000 */  nop
    /* 6FFE4 8007F7E4 00000000 */  nop
    /* 6FFE8 8007F7E8 12700000 */  mflo       $t6
    /* 6FFEC 8007F7EC 03CB0E00 */  sra        $t9, $t6, 12
    /* 6FFF0 8007F7F0 21702F03 */  addu       $t6, $t9, $t7
    /* 6FFF4 8007F7F4 19008801 */  multu      $t4, $t0
    /* 6FFF8 8007F7F8 0800AEA4 */  sh         $t6, 0x8($a1)
    /* 6FFFC 8007F7FC 00000000 */  nop
    /* 70000 8007F800 12780000 */  mflo       $t7
    /* 70004 8007F804 03C30F00 */  sra        $t8, $t7, 12
    /* 70008 8007F808 00000000 */  nop
    /* 7000C 8007F80C 19000A03 */  multu      $t8, $t2
    /* 70010 8007F810 00000000 */  nop
    /* 70014 8007F814 00000000 */  nop
    /* 70018 8007F818 12780000 */  mflo       $t7
    /* 7001C 8007F81C 03730F00 */  sra        $t6, $t7, 12
    /* 70020 8007F820 00000000 */  nop
    /* 70024 8007F824 19006D01 */  multu      $t3, $t5
    /* 70028 8007F828 00000000 */  nop
    /* 7002C 8007F82C 00000000 */  nop
    /* 70030 8007F830 12780000 */  mflo       $t7
    /* 70034 8007F834 03CB0F00 */  sra        $t9, $t7, 12
    /* 70038 8007F838 2178D901 */  addu       $t7, $t6, $t9
    /* 7003C 8007F83C 19006A01 */  multu      $t3, $t2
    /* 70040 8007F840 0400AFA4 */  sh         $t7, 0x4($a1)
    /* 70044 8007F844 00000000 */  nop
    /* 70048 8007F848 12700000 */  mflo       $t6
    /* 7004C 8007F84C 037B0E00 */  sra        $t7, $t6, 12
    /* 70050 8007F850 00000000 */  nop
    /* 70054 8007F854 19000D03 */  multu      $t8, $t5
    /* 70058 8007F858 00000000 */  nop
    /* 7005C 8007F85C 00000000 */  nop
    /* 70060 8007F860 12700000 */  mflo       $t6
    /* 70064 8007F864 03CB0E00 */  sra        $t9, $t6, 12
    /* 70068 8007F868 23702F03 */  subu       $t6, $t9, $t7
    /* 7006C 8007F86C 0A00AEA4 */  sh         $t6, 0xA($a1)
    /* 70070 8007F870 0800E003 */  jr         $ra
    /* 70074 8007F874 00000000 */   nop
endlabel func_8007F5EC
    /* 70078 8007F878 00000000 */  nop

nonmatching func_8007F87C, 0x198

glabel func_8007F87C
    /* 7007C 8007F87C 21788000 */  addu       $t7, $a0, $zero
    /* 70080 8007F880 2110A000 */  addu       $v0, $a1, $zero
    /* 70084 8007F884 0E00E105 */  bgez       $t7, .L8007F8C0
    /* 70088 8007F888 FF0FF931 */   andi      $t9, $t7, 0xFFF
    /* 7008C 8007F88C 23780F00 */  negu       $t7, $t7
    /* 70090 8007F890 0100E105 */  bgez       $t7, .L8007F898
    /* 70094 8007F894 FF0FEF31 */   andi      $t7, $t7, 0xFFF
  .L8007F898:
    /* 70098 8007F898 80C00F00 */  sll        $t8, $t7, 2
    /* 7009C 8007F89C 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 700A0 8007F8A0 21C83803 */  addu       $t9, $t9, $t8
    /* 700A4 8007F8A4 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 700A8 8007F8A8 00000000 */  nop
    /* 700AC 8007F8AC 00741900 */  sll        $t6, $t9, 16
    /* 700B0 8007F8B0 03740E00 */  sra        $t6, $t6, 16
    /* 700B4 8007F8B4 23480E00 */  negu       $t1, $t6
    /* 700B8 8007F8B8 38FE0108 */  j          .L8007F8E0
    /* 700BC 8007F8BC 03441900 */   sra       $t0, $t9, 16
  .L8007F8C0:
    /* 700C0 8007F8C0 80C01900 */  sll        $t8, $t9, 2
    /* 700C4 8007F8C4 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 700C8 8007F8C8 21C83803 */  addu       $t9, $t9, $t8
    /* 700CC 8007F8CC 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 700D0 8007F8D0 00000000 */  nop
    /* 700D4 8007F8D4 00C41900 */  sll        $t8, $t9, 16
    /* 700D8 8007F8D8 034C1800 */  sra        $t1, $t8, 16
    /* 700DC 8007F8DC 03441900 */  sra        $t0, $t9, 16
  .L8007F8E0:
    /* 700E0 8007F8E0 0600AA84 */  lh         $t2, 0x6($a1)
    /* 700E4 8007F8E4 0C00AD84 */  lh         $t5, 0xC($a1)
    /* 700E8 8007F8E8 19000A01 */  multu      $t0, $t2
    /* 700EC 8007F8EC 0800AB84 */  lh         $t3, 0x8($a1)
    /* 700F0 8007F8F0 0E00AE84 */  lh         $t6, 0xE($a1)
    /* 700F4 8007F8F4 12C00000 */  mflo       $t8
    /* 700F8 8007F8F8 0A00AC84 */  lh         $t4, 0xA($a1)
    /* 700FC 8007F8FC 1000AF84 */  lh         $t7, 0x10($a1)
    /* 70100 8007F900 19002D01 */  multu      $t1, $t5
    /* 70104 8007F904 00000000 */  nop
    /* 70108 8007F908 00000000 */  nop
    /* 7010C 8007F90C 12C80000 */  mflo       $t9
    /* 70110 8007F910 23C81903 */  subu       $t9, $t8, $t9
    /* 70114 8007F914 03C31900 */  sra        $t8, $t9, 12
    /* 70118 8007F918 19000B01 */  multu      $t0, $t3
    /* 7011C 8007F91C 0600B8A4 */  sh         $t8, 0x6($a1)
    /* 70120 8007F920 00000000 */  nop
    /* 70124 8007F924 12C00000 */  mflo       $t8
    /* 70128 8007F928 00000000 */  nop
    /* 7012C 8007F92C 00000000 */  nop
    /* 70130 8007F930 19002E01 */  multu      $t1, $t6
    /* 70134 8007F934 00000000 */  nop
    /* 70138 8007F938 00000000 */  nop
    /* 7013C 8007F93C 12C80000 */  mflo       $t9
    /* 70140 8007F940 23C81903 */  subu       $t9, $t8, $t9
    /* 70144 8007F944 03C31900 */  sra        $t8, $t9, 12
    /* 70148 8007F948 19000C01 */  multu      $t0, $t4
    /* 7014C 8007F94C 0800B8A4 */  sh         $t8, 0x8($a1)
    /* 70150 8007F950 00000000 */  nop
    /* 70154 8007F954 12C00000 */  mflo       $t8
    /* 70158 8007F958 00000000 */  nop
    /* 7015C 8007F95C 00000000 */  nop
    /* 70160 8007F960 19002F01 */  multu      $t1, $t7
    /* 70164 8007F964 00000000 */  nop
    /* 70168 8007F968 00000000 */  nop
    /* 7016C 8007F96C 12C80000 */  mflo       $t9
    /* 70170 8007F970 23C81903 */  subu       $t9, $t8, $t9
    /* 70174 8007F974 03C31900 */  sra        $t8, $t9, 12
    /* 70178 8007F978 19002A01 */  multu      $t1, $t2
    /* 7017C 8007F97C 0A00B8A4 */  sh         $t8, 0xA($a1)
    /* 70180 8007F980 00000000 */  nop
    /* 70184 8007F984 12C00000 */  mflo       $t8
    /* 70188 8007F988 00000000 */  nop
    /* 7018C 8007F98C 00000000 */  nop
    /* 70190 8007F990 19000D01 */  multu      $t0, $t5
    /* 70194 8007F994 00000000 */  nop
    /* 70198 8007F998 00000000 */  nop
    /* 7019C 8007F99C 12C80000 */  mflo       $t9
    /* 701A0 8007F9A0 21C81903 */  addu       $t9, $t8, $t9
    /* 701A4 8007F9A4 03C31900 */  sra        $t8, $t9, 12
    /* 701A8 8007F9A8 19002B01 */  multu      $t1, $t3
    /* 701AC 8007F9AC 0C00B8A4 */  sh         $t8, 0xC($a1)
    /* 701B0 8007F9B0 00000000 */  nop
    /* 701B4 8007F9B4 12C00000 */  mflo       $t8
    /* 701B8 8007F9B8 00000000 */  nop
    /* 701BC 8007F9BC 00000000 */  nop
    /* 701C0 8007F9C0 19000E01 */  multu      $t0, $t6
    /* 701C4 8007F9C4 00000000 */  nop
    /* 701C8 8007F9C8 00000000 */  nop
    /* 701CC 8007F9CC 12C80000 */  mflo       $t9
    /* 701D0 8007F9D0 21C81903 */  addu       $t9, $t8, $t9
    /* 701D4 8007F9D4 03C31900 */  sra        $t8, $t9, 12
    /* 701D8 8007F9D8 19002C01 */  multu      $t1, $t4
    /* 701DC 8007F9DC 0E00B8A4 */  sh         $t8, 0xE($a1)
    /* 701E0 8007F9E0 00000000 */  nop
    /* 701E4 8007F9E4 12C00000 */  mflo       $t8
    /* 701E8 8007F9E8 00000000 */  nop
    /* 701EC 8007F9EC 00000000 */  nop
    /* 701F0 8007F9F0 19000F01 */  multu      $t0, $t7
    /* 701F4 8007F9F4 00000000 */  nop
    /* 701F8 8007F9F8 00000000 */  nop
    /* 701FC 8007F9FC 12C80000 */  mflo       $t9
    /* 70200 8007FA00 21C81903 */  addu       $t9, $t8, $t9
    /* 70204 8007FA04 03C31900 */  sra        $t8, $t9, 12
    /* 70208 8007FA08 1000B8A4 */  sh         $t8, 0x10($a1)
    /* 7020C 8007FA0C 0800E003 */  jr         $ra
    /* 70210 8007FA10 00000000 */   nop
endlabel func_8007F87C
    /* 70214 8007FA14 00000000 */  nop
    /* 70218 8007FA18 00000000 */  nop

nonmatching func_8007FA1C, 0x198

glabel func_8007FA1C
    /* 7021C 8007FA1C 21788000 */  addu       $t7, $a0, $zero
    /* 70220 8007FA20 2110A000 */  addu       $v0, $a1, $zero
    /* 70224 8007FA24 0D00E105 */  bgez       $t7, .L8007FA5C
    /* 70228 8007FA28 FF0FF931 */   andi      $t9, $t7, 0xFFF
    /* 7022C 8007FA2C 23780F00 */  negu       $t7, $t7
    /* 70230 8007FA30 0100E105 */  bgez       $t7, .L8007FA38
    /* 70234 8007FA34 FF0FEF31 */   andi      $t7, $t7, 0xFFF
  .L8007FA38:
    /* 70238 8007FA38 80C00F00 */  sll        $t8, $t7, 2
    /* 7023C 8007FA3C 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 70240 8007FA40 21C83803 */  addu       $t9, $t9, $t8
    /* 70244 8007FA44 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 70248 8007FA48 00000000 */  nop
    /* 7024C 8007FA4C 00741900 */  sll        $t6, $t9, 16
    /* 70250 8007FA50 034C0E00 */  sra        $t1, $t6, 16
    /* 70254 8007FA54 A0FE0108 */  j          .L8007FA80
    /* 70258 8007FA58 03441900 */   sra       $t0, $t9, 16
  .L8007FA5C:
    /* 7025C 8007FA5C 80C01900 */  sll        $t8, $t9, 2
    /* 70260 8007FA60 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 70264 8007FA64 21C83803 */  addu       $t9, $t9, $t8
    /* 70268 8007FA68 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 7026C 8007FA6C 00000000 */  nop
    /* 70270 8007FA70 00C41900 */  sll        $t8, $t9, 16
    /* 70274 8007FA74 037C1800 */  sra        $t7, $t8, 16
    /* 70278 8007FA78 23480F00 */  negu       $t1, $t7
    /* 7027C 8007FA7C 03441900 */  sra        $t0, $t9, 16
  .L8007FA80:
    /* 70280 8007FA80 0000AA84 */  lh         $t2, 0x0($a1)
    /* 70284 8007FA84 0C00AD84 */  lh         $t5, 0xC($a1)
    /* 70288 8007FA88 19000A01 */  multu      $t0, $t2
    /* 7028C 8007FA8C 0200AB84 */  lh         $t3, 0x2($a1)
    /* 70290 8007FA90 0E00AE84 */  lh         $t6, 0xE($a1)
    /* 70294 8007FA94 12C00000 */  mflo       $t8
    /* 70298 8007FA98 0400AC84 */  lh         $t4, 0x4($a1)
    /* 7029C 8007FA9C 1000AF84 */  lh         $t7, 0x10($a1)
    /* 702A0 8007FAA0 19002D01 */  multu      $t1, $t5
    /* 702A4 8007FAA4 00000000 */  nop
    /* 702A8 8007FAA8 00000000 */  nop
    /* 702AC 8007FAAC 12C80000 */  mflo       $t9
    /* 702B0 8007FAB0 23C81903 */  subu       $t9, $t8, $t9
    /* 702B4 8007FAB4 03C31900 */  sra        $t8, $t9, 12
    /* 702B8 8007FAB8 19000B01 */  multu      $t0, $t3
    /* 702BC 8007FABC 0000B8A4 */  sh         $t8, 0x0($a1)
    /* 702C0 8007FAC0 00000000 */  nop
    /* 702C4 8007FAC4 12C00000 */  mflo       $t8
    /* 702C8 8007FAC8 00000000 */  nop
    /* 702CC 8007FACC 00000000 */  nop
    /* 702D0 8007FAD0 19002E01 */  multu      $t1, $t6
    /* 702D4 8007FAD4 00000000 */  nop
    /* 702D8 8007FAD8 00000000 */  nop
    /* 702DC 8007FADC 12C80000 */  mflo       $t9
    /* 702E0 8007FAE0 23C81903 */  subu       $t9, $t8, $t9
    /* 702E4 8007FAE4 03C31900 */  sra        $t8, $t9, 12
    /* 702E8 8007FAE8 19000C01 */  multu      $t0, $t4
    /* 702EC 8007FAEC 0200B8A4 */  sh         $t8, 0x2($a1)
    /* 702F0 8007FAF0 00000000 */  nop
    /* 702F4 8007FAF4 12C00000 */  mflo       $t8
    /* 702F8 8007FAF8 00000000 */  nop
    /* 702FC 8007FAFC 00000000 */  nop
    /* 70300 8007FB00 19002F01 */  multu      $t1, $t7
    /* 70304 8007FB04 00000000 */  nop
    /* 70308 8007FB08 00000000 */  nop
    /* 7030C 8007FB0C 12C80000 */  mflo       $t9
    /* 70310 8007FB10 23C81903 */  subu       $t9, $t8, $t9
    /* 70314 8007FB14 03C31900 */  sra        $t8, $t9, 12
    /* 70318 8007FB18 19002A01 */  multu      $t1, $t2
    /* 7031C 8007FB1C 0400B8A4 */  sh         $t8, 0x4($a1)
    /* 70320 8007FB20 00000000 */  nop
    /* 70324 8007FB24 12C00000 */  mflo       $t8
    /* 70328 8007FB28 00000000 */  nop
    /* 7032C 8007FB2C 00000000 */  nop
    /* 70330 8007FB30 19000D01 */  multu      $t0, $t5
    /* 70334 8007FB34 00000000 */  nop
    /* 70338 8007FB38 00000000 */  nop
    /* 7033C 8007FB3C 12C80000 */  mflo       $t9
    /* 70340 8007FB40 21C81903 */  addu       $t9, $t8, $t9
    /* 70344 8007FB44 03C31900 */  sra        $t8, $t9, 12
    /* 70348 8007FB48 19002B01 */  multu      $t1, $t3
    /* 7034C 8007FB4C 0C00B8A4 */  sh         $t8, 0xC($a1)
    /* 70350 8007FB50 00000000 */  nop
    /* 70354 8007FB54 12C00000 */  mflo       $t8
    /* 70358 8007FB58 00000000 */  nop
    /* 7035C 8007FB5C 00000000 */  nop
    /* 70360 8007FB60 19000E01 */  multu      $t0, $t6
    /* 70364 8007FB64 00000000 */  nop
    /* 70368 8007FB68 00000000 */  nop
    /* 7036C 8007FB6C 12C80000 */  mflo       $t9
    /* 70370 8007FB70 21C81903 */  addu       $t9, $t8, $t9
    /* 70374 8007FB74 03C31900 */  sra        $t8, $t9, 12
    /* 70378 8007FB78 19002C01 */  multu      $t1, $t4
    /* 7037C 8007FB7C 0E00B8A4 */  sh         $t8, 0xE($a1)
    /* 70380 8007FB80 00000000 */  nop
    /* 70384 8007FB84 12C00000 */  mflo       $t8
    /* 70388 8007FB88 00000000 */  nop
    /* 7038C 8007FB8C 00000000 */  nop
    /* 70390 8007FB90 19000F01 */  multu      $t0, $t7
    /* 70394 8007FB94 00000000 */  nop
    /* 70398 8007FB98 00000000 */  nop
    /* 7039C 8007FB9C 12C80000 */  mflo       $t9
    /* 703A0 8007FBA0 21C81903 */  addu       $t9, $t8, $t9
    /* 703A4 8007FBA4 03C31900 */  sra        $t8, $t9, 12
    /* 703A8 8007FBA8 1000B8A4 */  sh         $t8, 0x10($a1)
    /* 703AC 8007FBAC 0800E003 */  jr         $ra
    /* 703B0 8007FBB0 00000000 */   nop
endlabel func_8007FA1C
    /* 703B4 8007FBB4 00000000 */  nop
    /* 703B8 8007FBB8 00000000 */  nop

nonmatching func_8007FBBC, 0x198

glabel func_8007FBBC
    /* 703BC 8007FBBC 21788000 */  addu       $t7, $a0, $zero
    /* 703C0 8007FBC0 2110A000 */  addu       $v0, $a1, $zero
    /* 703C4 8007FBC4 0E00E105 */  bgez       $t7, .L8007FC00
    /* 703C8 8007FBC8 FF0FF931 */   andi      $t9, $t7, 0xFFF
    /* 703CC 8007FBCC 23780F00 */  negu       $t7, $t7
    /* 703D0 8007FBD0 0100E105 */  bgez       $t7, .L8007FBD8
    /* 703D4 8007FBD4 FF0FEF31 */   andi      $t7, $t7, 0xFFF
  .L8007FBD8:
    /* 703D8 8007FBD8 80C00F00 */  sll        $t8, $t7, 2
    /* 703DC 8007FBDC 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 703E0 8007FBE0 21C83803 */  addu       $t9, $t9, $t8
    /* 703E4 8007FBE4 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 703E8 8007FBE8 00000000 */  nop
    /* 703EC 8007FBEC 00741900 */  sll        $t6, $t9, 16
    /* 703F0 8007FBF0 03740E00 */  sra        $t6, $t6, 16
    /* 703F4 8007FBF4 23480E00 */  negu       $t1, $t6
    /* 703F8 8007FBF8 08FF0108 */  j          .L8007FC20
    /* 703FC 8007FBFC 03441900 */   sra       $t0, $t9, 16
  .L8007FC00:
    /* 70400 8007FC00 80C01900 */  sll        $t8, $t9, 2
    /* 70404 8007FC04 0A80193C */  lui        $t9, %hi(D_8009C928)
    /* 70408 8007FC08 21C83803 */  addu       $t9, $t9, $t8
    /* 7040C 8007FC0C 28C9398F */  lw         $t9, %lo(D_8009C928)($t9)
    /* 70410 8007FC10 00000000 */  nop
    /* 70414 8007FC14 00C41900 */  sll        $t8, $t9, 16
    /* 70418 8007FC18 034C1800 */  sra        $t1, $t8, 16
    /* 7041C 8007FC1C 03441900 */  sra        $t0, $t9, 16
  .L8007FC20:
    /* 70420 8007FC20 0000AA84 */  lh         $t2, 0x0($a1)
    /* 70424 8007FC24 0600AD84 */  lh         $t5, 0x6($a1)
    /* 70428 8007FC28 19000A01 */  multu      $t0, $t2
    /* 7042C 8007FC2C 0200AB84 */  lh         $t3, 0x2($a1)
    /* 70430 8007FC30 0800AE84 */  lh         $t6, 0x8($a1)
    /* 70434 8007FC34 12C00000 */  mflo       $t8
    /* 70438 8007FC38 0400AC84 */  lh         $t4, 0x4($a1)
    /* 7043C 8007FC3C 0A00AF84 */  lh         $t7, 0xA($a1)
    /* 70440 8007FC40 19002D01 */  multu      $t1, $t5
    /* 70444 8007FC44 00000000 */  nop
    /* 70448 8007FC48 00000000 */  nop
    /* 7044C 8007FC4C 12C80000 */  mflo       $t9
    /* 70450 8007FC50 23C81903 */  subu       $t9, $t8, $t9
    /* 70454 8007FC54 03C31900 */  sra        $t8, $t9, 12
    /* 70458 8007FC58 19000B01 */  multu      $t0, $t3
    /* 7045C 8007FC5C 0000B8A4 */  sh         $t8, 0x0($a1)
    /* 70460 8007FC60 00000000 */  nop
    /* 70464 8007FC64 12C00000 */  mflo       $t8
    /* 70468 8007FC68 00000000 */  nop
    /* 7046C 8007FC6C 00000000 */  nop
    /* 70470 8007FC70 19002E01 */  multu      $t1, $t6
    /* 70474 8007FC74 00000000 */  nop
    /* 70478 8007FC78 00000000 */  nop
    /* 7047C 8007FC7C 12C80000 */  mflo       $t9
    /* 70480 8007FC80 23C81903 */  subu       $t9, $t8, $t9
    /* 70484 8007FC84 03C31900 */  sra        $t8, $t9, 12
    /* 70488 8007FC88 19000C01 */  multu      $t0, $t4
    /* 7048C 8007FC8C 0200B8A4 */  sh         $t8, 0x2($a1)
    /* 70490 8007FC90 00000000 */  nop
    /* 70494 8007FC94 12C00000 */  mflo       $t8
    /* 70498 8007FC98 00000000 */  nop
    /* 7049C 8007FC9C 00000000 */  nop
    /* 704A0 8007FCA0 19002F01 */  multu      $t1, $t7
    /* 704A4 8007FCA4 00000000 */  nop
    /* 704A8 8007FCA8 00000000 */  nop
    /* 704AC 8007FCAC 12C80000 */  mflo       $t9
    /* 704B0 8007FCB0 23C81903 */  subu       $t9, $t8, $t9
    /* 704B4 8007FCB4 03C31900 */  sra        $t8, $t9, 12
    /* 704B8 8007FCB8 19002A01 */  multu      $t1, $t2
    /* 704BC 8007FCBC 0400B8A4 */  sh         $t8, 0x4($a1)
    /* 704C0 8007FCC0 00000000 */  nop
    /* 704C4 8007FCC4 12C00000 */  mflo       $t8
    /* 704C8 8007FCC8 00000000 */  nop
    /* 704CC 8007FCCC 00000000 */  nop
    /* 704D0 8007FCD0 19000D01 */  multu      $t0, $t5
    /* 704D4 8007FCD4 00000000 */  nop
    /* 704D8 8007FCD8 00000000 */  nop
    /* 704DC 8007FCDC 12C80000 */  mflo       $t9
    /* 704E0 8007FCE0 21C81903 */  addu       $t9, $t8, $t9
    /* 704E4 8007FCE4 03C31900 */  sra        $t8, $t9, 12
    /* 704E8 8007FCE8 19002B01 */  multu      $t1, $t3
    /* 704EC 8007FCEC 0600B8A4 */  sh         $t8, 0x6($a1)
    /* 704F0 8007FCF0 00000000 */  nop
    /* 704F4 8007FCF4 12C00000 */  mflo       $t8
    /* 704F8 8007FCF8 00000000 */  nop
    /* 704FC 8007FCFC 00000000 */  nop
    /* 70500 8007FD00 19000E01 */  multu      $t0, $t6
    /* 70504 8007FD04 00000000 */  nop
    /* 70508 8007FD08 00000000 */  nop
    /* 7050C 8007FD0C 12C80000 */  mflo       $t9
    /* 70510 8007FD10 21C81903 */  addu       $t9, $t8, $t9
    /* 70514 8007FD14 03C31900 */  sra        $t8, $t9, 12
    /* 70518 8007FD18 19002C01 */  multu      $t1, $t4
    /* 7051C 8007FD1C 0800B8A4 */  sh         $t8, 0x8($a1)
    /* 70520 8007FD20 00000000 */  nop
    /* 70524 8007FD24 12C00000 */  mflo       $t8
    /* 70528 8007FD28 00000000 */  nop
    /* 7052C 8007FD2C 00000000 */  nop
    /* 70530 8007FD30 19000F01 */  multu      $t0, $t7
    /* 70534 8007FD34 00000000 */  nop
    /* 70538 8007FD38 00000000 */  nop
    /* 7053C 8007FD3C 12C80000 */  mflo       $t9
    /* 70540 8007FD40 21C81903 */  addu       $t9, $t8, $t9
    /* 70544 8007FD44 03C31900 */  sra        $t8, $t9, 12
    /* 70548 8007FD48 0A00B8A4 */  sh         $t8, 0xA($a1)
    /* 7054C 8007FD4C 0800E003 */  jr         $ra
    /* 70550 8007FD50 00000000 */   nop
endlabel func_8007FBBC
    /* 70554 8007FD54 00000000 */  nop
    /* 70558 8007FD58 00000000 */  nop

nonmatching func_8007FD5C, 0x180

glabel func_8007FD5C
    /* 7055C 8007FD5C 21300000 */  addu       $a2, $zero, $zero
    /* 70560 8007FD60 0300A104 */  bgez       $a1, .L8007FD70
    /* 70564 8007FD64 21380000 */   addu      $a3, $zero, $zero
    /* 70568 8007FD68 01000624 */  addiu      $a2, $zero, 0x1
    /* 7056C 8007FD6C 23280500 */  negu       $a1, $a1
  .L8007FD70:
    /* 70570 8007FD70 03008104 */  bgez       $a0, .L8007FD80
    /* 70574 8007FD74 00000000 */   nop
    /* 70578 8007FD78 01000724 */  addiu      $a3, $zero, 0x1
    /* 7057C 8007FD7C 23200400 */  negu       $a0, $a0
  .L8007FD80:
    /* 70580 8007FD80 0500A014 */  bnez       $a1, .L8007FD98
    /* 70584 8007FD84 2A108500 */   slt       $v0, $a0, $a1
    /* 70588 8007FD88 03008014 */  bnez       $a0, .L8007FD98
    /* 7058C 8007FD8C 00000000 */   nop
    /* 70590 8007FD90 B5FF0108 */  j          .L8007FED4
    /* 70594 8007FD94 21100000 */   addu      $v0, $zero, $zero
  .L8007FD98:
    /* 70598 8007FD98 24004010 */  beqz       $v0, .L8007FE2C
    /* 7059C 8007FD9C E07F023C */   lui       $v0, (0x7FE00000 >> 16)
    /* 705A0 8007FDA0 24108200 */  and        $v0, $a0, $v0
    /* 705A4 8007FDA4 0E004010 */  beqz       $v0, .L8007FDE0
    /* 705A8 8007FDA8 83120500 */   sra       $v0, $a1, 10
    /* 705AC 8007FDAC 1A008200 */  div        $zero, $a0, $v0
    /* 705B0 8007FDB0 02004014 */  bnez       $v0, .L8007FDBC
    /* 705B4 8007FDB4 00000000 */   nop
    /* 705B8 8007FDB8 0D000700 */  break      7
  .L8007FDBC:
    /* 705BC 8007FDBC FFFF0124 */  addiu      $at, $zero, -0x1
    /* 705C0 8007FDC0 04004114 */  bne        $v0, $at, .L8007FDD4
    /* 705C4 8007FDC4 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 705C8 8007FDC8 02008114 */  bne        $a0, $at, .L8007FDD4
    /* 705CC 8007FDCC 00000000 */   nop
    /* 705D0 8007FDD0 0D000600 */  break      6
  .L8007FDD4:
    /* 705D4 8007FDD4 12200000 */  mflo       $a0
    /* 705D8 8007FDD8 86FF0108 */  j          .L8007FE18
    /* 705DC 8007FDDC 40100400 */   sll       $v0, $a0, 1
  .L8007FDE0:
    /* 705E0 8007FDE0 80120400 */  sll        $v0, $a0, 10
    /* 705E4 8007FDE4 1A004500 */  div        $zero, $v0, $a1
    /* 705E8 8007FDE8 0200A014 */  bnez       $a1, .L8007FDF4
    /* 705EC 8007FDEC 00000000 */   nop
    /* 705F0 8007FDF0 0D000700 */  break      7
  .L8007FDF4:
    /* 705F4 8007FDF4 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 705F8 8007FDF8 0400A114 */  bne        $a1, $at, .L8007FE0C
    /* 705FC 8007FDFC 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 70600 8007FE00 02004114 */  bne        $v0, $at, .L8007FE0C
    /* 70604 8007FE04 00000000 */   nop
    /* 70608 8007FE08 0D000600 */  break      6
  .L8007FE0C:
    /* 7060C 8007FE0C 12200000 */  mflo       $a0
    /* 70610 8007FE10 00000000 */  nop
    /* 70614 8007FE14 40100400 */  sll        $v0, $a0, 1
  .L8007FE18:
    /* 70618 8007FE18 0A80013C */  lui        $at, %hi(D_800A0928)
    /* 7061C 8007FE1C 21082200 */  addu       $at, $at, $v0
    /* 70620 8007FE20 28092384 */  lh         $v1, %lo(D_800A0928)($at)
    /* 70624 8007FE24 AEFF0108 */  j          .L8007FEB8
    /* 70628 8007FE28 00000000 */   nop
  .L8007FE2C:
    /* 7062C 8007FE2C 2410A200 */  and        $v0, $a1, $v0
    /* 70630 8007FE30 0E004010 */  beqz       $v0, .L8007FE6C
    /* 70634 8007FE34 83120400 */   sra       $v0, $a0, 10
    /* 70638 8007FE38 1A00A200 */  div        $zero, $a1, $v0
    /* 7063C 8007FE3C 02004014 */  bnez       $v0, .L8007FE48
    /* 70640 8007FE40 00000000 */   nop
    /* 70644 8007FE44 0D000700 */  break      7
  .L8007FE48:
    /* 70648 8007FE48 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 7064C 8007FE4C 04004114 */  bne        $v0, $at, .L8007FE60
    /* 70650 8007FE50 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 70654 8007FE54 0200A114 */  bne        $a1, $at, .L8007FE60
    /* 70658 8007FE58 00000000 */   nop
    /* 7065C 8007FE5C 0D000600 */  break      6
  .L8007FE60:
    /* 70660 8007FE60 12200000 */  mflo       $a0
    /* 70664 8007FE64 A9FF0108 */  j          .L8007FEA4
    /* 70668 8007FE68 40100400 */   sll       $v0, $a0, 1
  .L8007FE6C:
    /* 7066C 8007FE6C 80120500 */  sll        $v0, $a1, 10
    /* 70670 8007FE70 1A004400 */  div        $zero, $v0, $a0
    /* 70674 8007FE74 02008014 */  bnez       $a0, .L8007FE80
    /* 70678 8007FE78 00000000 */   nop
    /* 7067C 8007FE7C 0D000700 */  break      7
  .L8007FE80:
    /* 70680 8007FE80 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 70684 8007FE84 04008114 */  bne        $a0, $at, .L8007FE98
    /* 70688 8007FE88 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 7068C 8007FE8C 02004114 */  bne        $v0, $at, .L8007FE98
    /* 70690 8007FE90 00000000 */   nop
    /* 70694 8007FE94 0D000600 */  break      6
  .L8007FE98:
    /* 70698 8007FE98 12200000 */  mflo       $a0
    /* 7069C 8007FE9C 00000000 */  nop
    /* 706A0 8007FEA0 40100400 */  sll        $v0, $a0, 1
  .L8007FEA4:
    /* 706A4 8007FEA4 0A80013C */  lui        $at, %hi(D_800A0928)
    /* 706A8 8007FEA8 21082200 */  addu       $at, $at, $v0
    /* 706AC 8007FEAC 28092384 */  lh         $v1, %lo(D_800A0928)($at)
    /* 706B0 8007FEB0 00040224 */  addiu      $v0, $zero, 0x400
    /* 706B4 8007FEB4 23184300 */  subu       $v1, $v0, $v1
  .L8007FEB8:
    /* 706B8 8007FEB8 0200C010 */  beqz       $a2, .L8007FEC4
    /* 706BC 8007FEBC 00080224 */   addiu     $v0, $zero, 0x800
    /* 706C0 8007FEC0 23184300 */  subu       $v1, $v0, $v1
  .L8007FEC4:
    /* 706C4 8007FEC4 0300E010 */  beqz       $a3, .L8007FED4
    /* 706C8 8007FEC8 21106000 */   addu      $v0, $v1, $zero
    /* 706CC 8007FECC 23180300 */  negu       $v1, $v1
    /* 706D0 8007FED0 21106000 */  addu       $v0, $v1, $zero
  .L8007FED4:
    /* 706D4 8007FED4 0800E003 */  jr         $ra
    /* 706D8 8007FED8 00000000 */   nop
endlabel func_8007FD5C

/* Handwritten function */
nonmatching func_8007FEDC, 0x9C

glabel func_8007FEDC
    /* 706DC 8007FEDC 0A80013C */  lui        $at, %hi(D_800A3658)
    /* 706E0 8007FEE0 58363FAC */  sw         $ra, %lo(D_800A3658)($at)
    /* 706E4 8007FEE4 6EE2010C */  jal        EnterCriticalSection
    /* 706E8 8007FEE8 00000000 */   nop
    /* 706EC 8007FEEC B0000A24 */  addiu      $t2, $zero, 0xB0
    /* 706F0 8007FEF0 09F84001 */  jalr       $t2
    /* 706F4 8007FEF4 56000924 */   addiu     $t1, $zero, 0x56
    /* 706F8 8007FEF8 08800A3C */  lui        $t2, %hi(D_8007FF44)
    /* 706FC 8007FEFC 0880093C */  lui        $t1, %hi(func_8007FF7C)
    /* 70700 8007FF00 1800428C */  lw         $v0, 0x18($v0)
    /* 70704 8007FF04 44FF4A25 */  addiu      $t2, $t2, %lo(D_8007FF44)
    /* 70708 8007FF08 7CFF2925 */  addiu      $t1, $t1, %lo(func_8007FF7C)
  .L8007FF0C:
    /* 7070C 8007FF0C 0000438D */  lw         $v1, 0x0($t2)
    /* 70710 8007FF10 04004A25 */  addiu      $t2, $t2, 0x4
    /* 70714 8007FF14 04004224 */  addiu      $v0, $v0, 0x4
    /* 70718 8007FF18 FCFF4915 */  bne        $t2, $t1, .L8007FF0C
    /* 7071C 8007FF1C FCFF43AC */   sw        $v1, -0x4($v0)
    /* 70720 8007FF20 FCE3010C */  jal        func_80078FF0
    /* 70724 8007FF24 00000000 */   nop
    /* 70728 8007FF28 72E2010C */  jal        ExitCriticalSection
    /* 7072C 8007FF2C 00000000 */   nop
    /* 70730 8007FF30 0A801F3C */  lui        $ra, %hi(D_800A3658)
    /* 70734 8007FF34 5836FF8F */  lw         $ra, %lo(D_800A3658)($ra)
    /* 70738 8007FF38 00000000 */  nop
    /* 7073C 8007FF3C 0800E003 */  jr         $ra
    /* 70740 8007FF40 00000000 */   nop
  alabel D_8007FF44
    /* 70744 8007FF44 00000000 */  nop
    /* 70748 8007FF48 00000000 */  nop
    /* 7074C 8007FF4C 00011A24 */  addiu      $k0, $zero, 0x100 /* handwritten instruction */
    /* 70750 8007FF50 08005A8F */  lw         $k0, 0x8($k0) /* handwritten instruction */
    /* 70754 8007FF54 00000000 */  nop
    /* 70758 8007FF58 00005A8F */  lw         $k0, 0x0($k0) /* handwritten instruction */
    /* 7075C 8007FF5C 00000000 */  nop
    /* 70760 8007FF60 08005A23 */  addi       $k0, $k0, 0x8 /* handwritten instruction */
    /* 70764 8007FF64 040041AF */  sw         $at, 0x4($k0) /* handwritten instruction */
    /* 70768 8007FF68 080042AF */  sw         $v0, 0x8($k0) /* handwritten instruction */
    /* 7076C 8007FF6C 0C0043AF */  sw         $v1, 0xC($k0) /* handwritten instruction */
    /* 70770 8007FF70 7C005FAF */  sw         $ra, 0x7C($k0) /* handwritten instruction */
    /* 70774 8007FF74 00680240 */  mfc0       $v0, $13 /* handwritten instruction */
endlabel func_8007FEDC
    /* 70778 8007FF78 00000000 */  nop

nonmatching func_8007FF7C, 0x110

glabel func_8007FF7C
    /* 7077C 8007FF7C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70780 8007FF80 1000B0AF */  sw         $s0, 0x10($sp)
    /* 70784 8007FF84 04001024 */  addiu      $s0, $zero, 0x4
    /* 70788 8007FF88 1400BFAF */  sw         $ra, 0x14($sp)
  .L8007FF8C:
    /* 7078C 8007FF8C 3700020C */  jal        func_800800DC
    /* 70790 8007FF90 01000424 */   addiu     $a0, $zero, 0x1
    /* 70794 8007FF94 01000324 */  addiu      $v1, $zero, 0x1
    /* 70798 8007FF98 11004314 */  bne        $v0, $v1, .L8007FFE0
    /* 7079C 8007FF9C FFFF1026 */   addiu     $s0, $s0, -0x1
    /* 707A0 8007FFA0 0880043C */  lui        $a0, %hi(D_80080014)
    /* 707A4 8007FFA4 14008424 */  addiu      $a0, $a0, %lo(D_80080014)
    /* 707A8 8007FFA8 8A00020C */  jal        func_80080228
    /* 707AC 8007FFAC 00000000 */   nop
    /* 707B0 8007FFB0 0880043C */  lui        $a0, %hi(D_8008003C)
    /* 707B4 8007FFB4 3C008424 */  addiu      $a0, $a0, %lo(D_8008003C)
    /* 707B8 8007FFB8 9000020C */  jal        func_80080240
    /* 707BC 8007FFBC 00000000 */   nop
    /* 707C0 8007FFC0 0880043C */  lui        $a0, %hi(D_80080064)
    /* 707C4 8007FFC4 64008424 */  addiu      $a0, $a0, %lo(D_80080064)
    /* 707C8 8007FFC8 270A020C */  jal        sys_SetVsyncMode
    /* 707CC 8007FFCC 00000000 */   nop
    /* 707D0 8007FFD0 2D0A020C */  jal        sys_SetTimer
    /* 707D4 8007FFD4 21200000 */   addu      $a0, $zero, $zero
    /* 707D8 8007FFD8 00000208 */  j          .L80080000
    /* 707DC 8007FFDC 01000224 */   addiu     $v0, $zero, 0x1
  .L8007FFE0:
    /* 707E0 8007FFE0 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 707E4 8007FFE4 E9FF0216 */  bne        $s0, $v0, .L8007FF8C
    /* 707E8 8007FFE8 00000000 */   nop
    /* 707EC 8007FFEC 0180043C */  lui        $a0, %hi(D_8001605C)
    /* 707F0 8007FFF0 5C608424 */  addiu      $a0, $a0, %lo(D_8001605C)
    /* 707F4 8007FFF4 82E4010C */  jal        debug_printf
    /* 707F8 8007FFF8 00000000 */   nop
    /* 707FC 8007FFFC 21100000 */  addu       $v0, $zero, $zero
  .L80080000:
    /* 70800 80080000 1400BF8F */  lw         $ra, 0x14($sp)
    /* 70804 80080004 1000B08F */  lw         $s0, 0x10($sp)
    /* 70808 80080008 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7080C 8008000C 0800E003 */  jr         $ra
    /* 70810 80080010 00000000 */   nop
  alabel D_80080014
    /* 70814 80080014 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70818 80080018 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7081C 8008001C 00F0043C */  lui        $a0, (0xF0000003 >> 16)
    /* 70820 80080020 03008434 */  ori        $a0, $a0, (0xF0000003 & 0xFFFF)
    /* 70824 80080024 2300020C */  jal        func_8008008C
    /* 70828 80080028 20000524 */   addiu     $a1, $zero, 0x20
    /* 7082C 8008002C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70830 80080030 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 70834 80080034 0800E003 */  jr         $ra
    /* 70838 80080038 00000000 */   nop
  alabel D_8008003C
    /* 7083C 8008003C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70840 80080040 1000BFAF */  sw         $ra, 0x10($sp)
    /* 70844 80080044 00F0043C */  lui        $a0, (0xF0000003 >> 16)
    /* 70848 80080048 03008434 */  ori        $a0, $a0, (0xF0000003 & 0xFFFF)
    /* 7084C 8008004C 2300020C */  jal        func_8008008C
    /* 70850 80080050 40000524 */   addiu     $a1, $zero, 0x40
    /* 70854 80080054 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70858 80080058 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7085C 8008005C 0800E003 */  jr         $ra
    /* 70860 80080060 00000000 */   nop
  alabel D_80080064
    /* 70864 80080064 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 70868 80080068 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7086C 8008006C 00F0043C */  lui        $a0, (0xF0000003 >> 16)
    /* 70870 80080070 03008434 */  ori        $a0, $a0, (0xF0000003 & 0xFFFF)
    /* 70874 80080074 2300020C */  jal        func_8008008C
    /* 70878 80080078 40000524 */   addiu     $a1, $zero, 0x40
    /* 7087C 8008007C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 70880 80080080 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 70884 80080084 0800E003 */  jr         $ra
    /* 70888 80080088 00000000 */   nop
endlabel func_8007FF7C

