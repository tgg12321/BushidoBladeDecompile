glabel func_8008C1E8
    /* 7C9E8 8008C1E8 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 7C9EC 8008C1EC 2000B4AF */  sw         $s4, 0x20($sp)
    /* 7C9F0 8008C1F0 21A0A000 */  addu       $s4, $a1, $zero
    /* 7C9F4 8008C1F4 0F80033C */  lui        $v1, %hi(D_800F1AEC)
    /* 7C9F8 8008C1F8 EC1A6324 */  addiu      $v1, $v1, %lo(D_800F1AEC)
    /* 7C9FC 8008C1FC 2800BFAF */  sw         $ra, 0x28($sp)
    /* 7CA00 8008C200 2400B5AF */  sw         $s5, 0x24($sp)
    /* 7CA04 8008C204 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 7CA08 8008C208 1800B2AF */  sw         $s2, 0x18($sp)
    /* 7CA0C 8008C20C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 7CA10 8008C210 1000B0AF */  sw         $s0, 0x10($sp)
    /* 7CA14 8008C214 0000628C */  lw         $v0, 0x0($v1)
    /* 7CA18 8008C218 00000000 */  nop
    /* 7CA1C 8008C21C 14004010 */  beqz       $v0, .L8008C270
    /* 7CA20 8008C220 21800000 */   addu      $s0, $zero, $zero
    /* 7CA24 8008C224 0F310208 */  j          .L8008C43C
    /* 7CA28 8008C228 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8008C22C:
    /* 7CA2C 8008C22C 00F0043C */  lui        $a0, (0xF000000B >> 16)
    /* 7CA30 8008C230 0B008434 */  ori        $a0, $a0, (0xF000000B & 0xFFFF)
    /* 7CA34 8008C234 2300020C */  jal        func_8008008C
    /* 7CA38 8008C238 00010524 */   addiu     $a1, $zero, 0x100
    /* 7CA3C 8008C23C 0A310208 */  j          .L8008C428
    /* 7CA40 8008C240 00000000 */   nop
  .L8008C244:
    /* 7CA44 8008C244 00F0043C */  lui        $a0, (0xF000000B >> 16)
    /* 7CA48 8008C248 0B008434 */  ori        $a0, $a0, (0xF000000B & 0xFFFF)
    /* 7CA4C 8008C24C 2300020C */  jal        func_8008008C
    /* 7CA50 8008C250 00010524 */   addiu     $a1, $zero, 0x100
    /* 7CA54 8008C254 0F80023C */  lui        $v0, %hi(D_800F1AF4)
    /* 7CA58 8008C258 F41A4224 */  addiu      $v0, $v0, %lo(D_800F1AF4)
    /* 7CA5C 8008C25C 0000428C */  lw         $v0, 0x0($v0)
    /* 7CA60 8008C260 00000000 */  nop
    /* 7CA64 8008C264 23108202 */  subu       $v0, $s4, $v0
    /* 7CA68 8008C268 0F310208 */  j          .L8008C43C
    /* 7CA6C 8008C26C FFFF4224 */   addiu     $v0, $v0, -0x1
  .L8008C270:
    /* 7CA70 8008C270 0F80023C */  lui        $v0, %hi(D_800F1AE2)
    /* 7CA74 8008C274 E21A4224 */  addiu      $v0, $v0, %lo(D_800F1AE2)
    /* 7CA78 8008C278 00004294 */  lhu        $v0, 0x0($v0)
    /* 7CA7C 8008C27C 00000000 */  nop
    /* 7CA80 8008C280 00034230 */  andi       $v0, $v0, 0x300
    /* 7CA84 8008C284 C2110200 */  srl        $v0, $v0, 7
    /* 7CA88 8008C288 0A80013C */  lui        $at, %hi(D_800A3074)
    /* 7CA8C 8008C28C 21082200 */  addu       $at, $at, $v0
    /* 7CA90 8008C290 74303584 */  lh         $s5, %lo(D_800A3074)($at)
    /* 7CA94 8008C294 0F80013C */  lui        $at, %hi(D_800F1AF4)
    /* 7CA98 8008C298 F41A34AC */  sw         $s4, %lo(D_800F1AF4)($at)
    /* 7CA9C 8008C29C 0F80013C */  lui        $at, %hi(D_800F1AF0)
    /* 7CAA0 8008C2A0 F01A24AC */  sw         $a0, %lo(D_800F1AF0)($at)
    /* 7CAA4 8008C2A4 0F80023C */  lui        $v0, %hi(D_800F1AF4)
    /* 7CAA8 8008C2A8 F41A428C */  lw         $v0, %lo(D_800F1AF4)($v0)
    /* 7CAAC 8008C2AC 00000000 */  nop
    /* 7CAB0 8008C2B0 5D004010 */  beqz       $v0, .L8008C428
    /* 7CAB4 8008C2B4 21880000 */   addu      $s1, $zero, $zero
    /* 7CAB8 8008C2B8 21986000 */  addu       $s3, $v1, $zero
  .L8008C2BC:
    /* 7CABC 8008C2BC 0A80023C */  lui        $v0, %hi(D_800A3044)
    /* 7CAC0 8008C2C0 4430428C */  lw         $v0, %lo(D_800A3044)($v0)
    /* 7CAC4 8008C2C4 00000000 */  nop
    /* 7CAC8 8008C2C8 04004294 */  lhu        $v0, 0x4($v0)
    /* 7CACC 8008C2CC 05000324 */  addiu      $v1, $zero, 0x5
    /* 7CAD0 8008C2D0 05004230 */  andi       $v0, $v0, 0x5
    /* 7CAD4 8008C2D4 14004310 */  beq        $v0, $v1, .L8008C328
    /* 7CAD8 8008C2D8 00000000 */   nop
    /* 7CADC 8008C2DC 05001224 */  addiu      $s2, $zero, 0x5
  .L8008C2E0:
    /* 7CAE0 8008C2E0 0F80023C */  lui        $v0, %hi(D_800F1AE8)
    /* 7CAE4 8008C2E4 E81A428C */  lw         $v0, %lo(D_800F1AE8)($v0)
    /* 7CAE8 8008C2E8 00000000 */  nop
    /* 7CAEC 8008C2EC 06004010 */  beqz       $v0, .L8008C308
    /* 7CAF0 8008C2F0 21280002 */   addu      $a1, $s0, $zero
    /* 7CAF4 8008C2F4 01001026 */  addiu      $s0, $s0, 0x1
    /* 7CAF8 8008C2F8 09F84000 */  jalr       $v0
    /* 7CAFC 8008C2FC 02000424 */   addiu     $a0, $zero, 0x2
    /* 7CB00 8008C300 CAFF4010 */  beqz       $v0, .L8008C22C
    /* 7CB04 8008C304 00000000 */   nop
  .L8008C308:
    /* 7CB08 8008C308 0A80023C */  lui        $v0, %hi(D_800A3044)
    /* 7CB0C 8008C30C 4430428C */  lw         $v0, %lo(D_800A3044)($v0)
    /* 7CB10 8008C310 00000000 */  nop
    /* 7CB14 8008C314 04004294 */  lhu        $v0, 0x4($v0)
    /* 7CB18 8008C318 00000000 */  nop
    /* 7CB1C 8008C31C 05004230 */  andi       $v0, $v0, 0x5
    /* 7CB20 8008C320 EFFF5214 */  bne        $v0, $s2, .L8008C2E0
    /* 7CB24 8008C324 00000000 */   nop
  .L8008C328:
    /* 7CB28 8008C328 09002016 */  bnez       $s1, .L8008C350
    /* 7CB2C 8008C32C 00000000 */   nop
    /* 7CB30 8008C330 0A80023C */  lui        $v0, %hi(D_800A3044)
    /* 7CB34 8008C334 4430428C */  lw         $v0, %lo(D_800A3044)($v0)
    /* 7CB38 8008C338 00000000 */  nop
    /* 7CB3C 8008C33C 04004294 */  lhu        $v0, 0x4($v0)
    /* 7CB40 8008C340 00000000 */  nop
    /* 7CB44 8008C344 80004230 */  andi       $v0, $v0, 0x80
    /* 7CB48 8008C348 0F80013C */  lui        $at, %hi(D_800F1AF8)
    /* 7CB4C 8008C34C F81A22AC */  sw         $v0, %lo(D_800F1AF8)($at)
  .L8008C350:
    /* 7CB50 8008C350 0F80023C */  lui        $v0, %hi(D_800F1AF0)
    /* 7CB54 8008C354 F01A428C */  lw         $v0, %lo(D_800F1AF0)($v0)
    /* 7CB58 8008C358 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7CB5C 8008C35C 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7CB60 8008C360 00004290 */  lbu        $v0, 0x0($v0)
    /* 7CB64 8008C364 00000000 */  nop
    /* 7CB68 8008C368 000062A0 */  sb         $v0, 0x0($v1)
    /* 7CB6C 8008C36C 0400628E */  lw         $v0, 0x4($s3)
    /* 7CB70 8008C370 00000000 */  nop
    /* 7CB74 8008C374 01004224 */  addiu      $v0, $v0, 0x1
    /* 7CB78 8008C378 040062AE */  sw         $v0, 0x4($s3)
    /* 7CB7C 8008C37C 0400628E */  lw         $v0, 0x4($s3)
    /* 7CB80 8008C380 0800628E */  lw         $v0, 0x8($s3)
    /* 7CB84 8008C384 01003126 */  addiu      $s1, $s1, 0x1
    /* 7CB88 8008C388 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 7CB8C 8008C38C 080062AE */  sw         $v0, 0x8($s3)
    /* 7CB90 8008C390 0800628E */  lw         $v0, 0x8($s3)
    /* 7CB94 8008C394 1E003516 */  bne        $s1, $s5, .L8008C410
    /* 7CB98 8008C398 00000000 */   nop
    /* 7CB9C 8008C39C 0A80023C */  lui        $v0, %hi(D_800A3044)
    /* 7CBA0 8008C3A0 4430428C */  lw         $v0, %lo(D_800A3044)($v0)
    /* 7CBA4 8008C3A4 00000000 */  nop
    /* 7CBA8 8008C3A8 04004294 */  lhu        $v0, 0x4($v0)
    /* 7CBAC 8008C3AC 0C00638E */  lw         $v1, 0xC($s3)
    /* 7CBB0 8008C3B0 80004230 */  andi       $v0, $v0, 0x80
    /* 7CBB4 8008C3B4 16004314 */  bne        $v0, $v1, .L8008C410
    /* 7CBB8 8008C3B8 21880000 */   addu      $s1, $zero, $zero
    /* 7CBBC 8008C3BC 0F80113C */  lui        $s1, %hi(D_800F1AF8)
    /* 7CBC0 8008C3C0 F81A3126 */  addiu      $s1, $s1, %lo(D_800F1AF8)
  .L8008C3C4:
    /* 7CBC4 8008C3C4 0F80023C */  lui        $v0, %hi(D_800F1AE8)
    /* 7CBC8 8008C3C8 E81A428C */  lw         $v0, %lo(D_800F1AE8)($v0)
    /* 7CBCC 8008C3CC 00000000 */  nop
    /* 7CBD0 8008C3D0 06004010 */  beqz       $v0, .L8008C3EC
    /* 7CBD4 8008C3D4 21280002 */   addu      $a1, $s0, $zero
    /* 7CBD8 8008C3D8 01001026 */  addiu      $s0, $s0, 0x1
    /* 7CBDC 8008C3DC 09F84000 */  jalr       $v0
    /* 7CBE0 8008C3E0 02000424 */   addiu     $a0, $zero, 0x2
    /* 7CBE4 8008C3E4 97FF4010 */  beqz       $v0, .L8008C244
    /* 7CBE8 8008C3E8 00000000 */   nop
  .L8008C3EC:
    /* 7CBEC 8008C3EC 0A80023C */  lui        $v0, %hi(D_800A3044)
    /* 7CBF0 8008C3F0 4430428C */  lw         $v0, %lo(D_800A3044)($v0)
    /* 7CBF4 8008C3F4 00000000 */  nop
    /* 7CBF8 8008C3F8 04004294 */  lhu        $v0, 0x4($v0)
    /* 7CBFC 8008C3FC 0000238E */  lw         $v1, 0x0($s1)
    /* 7CC00 8008C400 80004230 */  andi       $v0, $v0, 0x80
    /* 7CC04 8008C404 EFFF4310 */  beq        $v0, $v1, .L8008C3C4
    /* 7CC08 8008C408 00000000 */   nop
    /* 7CC0C 8008C40C 21880000 */  addu       $s1, $zero, $zero
  .L8008C410:
    /* 7CC10 8008C410 0F80023C */  lui        $v0, %hi(D_800F1AF4)
    /* 7CC14 8008C414 F41A4224 */  addiu      $v0, $v0, %lo(D_800F1AF4)
    /* 7CC18 8008C418 0000428C */  lw         $v0, 0x0($v0)
    /* 7CC1C 8008C41C 00000000 */  nop
    /* 7CC20 8008C420 A6FF4014 */  bnez       $v0, .L8008C2BC
    /* 7CC24 8008C424 00000000 */   nop
  .L8008C428:
    /* 7CC28 8008C428 0F80023C */  lui        $v0, %hi(D_800F1AF4)
    /* 7CC2C 8008C42C F41A4224 */  addiu      $v0, $v0, %lo(D_800F1AF4)
    /* 7CC30 8008C430 0000428C */  lw         $v0, 0x0($v0)
    /* 7CC34 8008C434 00000000 */  nop
    /* 7CC38 8008C438 23108202 */  subu       $v0, $s4, $v0
  .L8008C43C:
    /* 7CC3C 8008C43C 2800BF8F */  lw         $ra, 0x28($sp)
    /* 7CC40 8008C440 2400B58F */  lw         $s5, 0x24($sp)
    /* 7CC44 8008C444 2000B48F */  lw         $s4, 0x20($sp)
    /* 7CC48 8008C448 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 7CC4C 8008C44C 1800B28F */  lw         $s2, 0x18($sp)
    /* 7CC50 8008C450 1400B18F */  lw         $s1, 0x14($sp)
    /* 7CC54 8008C454 1000B08F */  lw         $s0, 0x10($sp)
    /* 7CC58 8008C458 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 7CC5C 8008C45C 0800E003 */  jr         $ra
    /* 7CC60 8008C460 00000000 */   nop
endlabel func_8008C1E8
