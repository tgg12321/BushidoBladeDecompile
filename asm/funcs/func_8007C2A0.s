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
