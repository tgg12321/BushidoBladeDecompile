glabel func_8005C2A8
    /* 4CAA8 8005C2A8 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 4CAAC 8005C2AC 1400B1AF */  sw         $s1, 0x14($sp)
    /* 4CAB0 8005C2B0 21888000 */  addu       $s1, $a0, $zero
    /* 4CAB4 8005C2B4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 4CAB8 8005C2B8 2180A000 */  addu       $s0, $a1, $zero
    /* 4CABC 8005C2BC 2800B6AF */  sw         $s6, 0x28($sp)
    /* 4CAC0 8005C2C0 21B0C000 */  addu       $s6, $a2, $zero
    /* 4CAC4 8005C2C4 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 4CAC8 8005C2C8 2400B5AF */  sw         $s5, 0x24($sp)
    /* 4CACC 8005C2CC 2000B4AF */  sw         $s4, 0x20($sp)
    /* 4CAD0 8005C2D0 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 4CAD4 8005C2D4 40DF010C */  jal        func_80077D00
    /* 4CAD8 8005C2D8 1800B2AF */   sw        $s2, 0x18($sp)
    /* 4CADC 8005C2DC 1400428C */  lw         $v0, 0x14($v0)
    /* 4CAE0 8005C2E0 03000324 */  addiu      $v1, $zero, 0x3
    /* 4CAE4 8005C2E4 0F004230 */  andi       $v0, $v0, 0xF
    /* 4CAE8 8005C2E8 06004314 */  bne        $v0, $v1, .L8005C304
    /* 4CAEC 8005C2EC 21A80002 */   addu      $s5, $s0, $zero
    /* 4CAF0 8005C2F0 00141000 */  sll        $v0, $s0, 16
    /* 4CAF4 8005C2F4 03140200 */  sra        $v0, $v0, 16
    /* 4CAF8 8005C2F8 05000324 */  addiu      $v1, $zero, 0x5
    /* 4CAFC 8005C2FC 65004310 */  beq        $v0, $v1, .L8005C494
    /* 4CB00 8005C300 21100000 */   addu      $v0, $zero, $zero
  .L8005C304:
    /* 4CB04 8005C304 3416020C */  jal        func_800858D0
    /* 4CB08 8005C308 21200000 */   addu      $a0, $zero, $zero
    /* 4CB0C 8005C30C 00141500 */  sll        $v0, $s5, 16
    /* 4CB10 8005C310 03840200 */  sra        $s0, $v0, 16
    /* 4CB14 8005C314 0F80143C */  lui        $s4, %hi(D_800EFC38)
    /* 4CB18 8005C318 38FC9426 */  addiu      $s4, $s4, %lo(D_800EFC38)
    /* 4CB1C 8005C31C 80981000 */  sll        $s3, $s0, 2
    /* 4CB20 8005C320 21907402 */  addu       $s2, $s3, $s4
    /* 4CB24 8005C324 0000428E */  lw         $v0, 0x0($s2)
    /* 4CB28 8005C328 00000000 */  nop
    /* 4CB2C 8005C32C 07004010 */  beqz       $v0, .L8005C34C
    /* 4CB30 8005C330 00000000 */   nop
    /* 4CB34 8005C334 D91F020C */  jal        func_80087F64
    /* 4CB38 8005C338 21200002 */   addu      $a0, $s0, $zero
    /* 4CB3C 8005C33C 000040AE */  sw         $zero, 0x0($s2)
    /* 4CB40 8005C340 0F80013C */  lui        $at, %hi(D_800EFB38)
    /* 4CB44 8005C344 21083300 */  addu       $at, $at, $s3
    /* 4CB48 8005C348 38FB20AC */  sw         $zero, %lo(D_800EFB38)($at)
  .L8005C34C:
    /* 4CB4C 8005C34C 19000012 */  beqz       $s0, .L8005C3B4
    /* 4CB50 8005C350 00000000 */   nop
    /* 4CB54 8005C354 21200000 */  addu       $a0, $zero, $zero
    /* 4CB58 8005C358 0F80023C */  lui        $v0, %hi(D_800EFB38)
    /* 4CB5C 8005C35C 38FB428C */  lw         $v0, %lo(D_800EFB38)($v0)
    /* 4CB60 8005C360 21288002 */  addu       $a1, $s4, $zero
    /* 4CB64 8005C364 380382AF */  sw         $v0, %gp_rel(D_800A3404)($gp)
    /* 4CB68 8005C368 00140400 */  sll        $v0, $a0, 16
  .L8005C36C:
    /* 4CB6C 8005C36C 83130200 */  sra        $v0, $v0, 14
    /* 4CB70 8005C370 21104500 */  addu       $v0, $v0, $a1
    /* 4CB74 8005C374 0000438C */  lw         $v1, 0x0($v0)
    /* 4CB78 8005C378 00000000 */  nop
    /* 4CB7C 8005C37C 07006010 */  beqz       $v1, .L8005C39C
    /* 4CB80 8005C380 01008224 */   addiu     $v0, $a0, 0x1
    /* 4CB84 8005C384 3803828F */  lw         $v0, %gp_rel(D_800A3404)($gp)
    /* 4CB88 8005C388 0C00638C */  lw         $v1, 0xC($v1)
    /* 4CB8C 8005C38C 00000000 */  nop
    /* 4CB90 8005C390 21104300 */  addu       $v0, $v0, $v1
    /* 4CB94 8005C394 380382AF */  sw         $v0, %gp_rel(D_800A3404)($gp)
    /* 4CB98 8005C398 01008224 */  addiu      $v0, $a0, 0x1
  .L8005C39C:
    /* 4CB9C 8005C39C 21204000 */  addu       $a0, $v0, $zero
    /* 4CBA0 8005C3A0 00140200 */  sll        $v0, $v0, 16
    /* 4CBA4 8005C3A4 03140200 */  sra        $v0, $v0, 16
    /* 4CBA8 8005C3A8 10004228 */  slti       $v0, $v0, 0x10
    /* 4CBAC 8005C3AC EFFF4014 */  bnez       $v0, .L8005C36C
    /* 4CBB0 8005C3B0 00140400 */   sll       $v0, $a0, 16
  .L8005C3B4:
    /* 4CBB4 8005C3B4 3803828F */  lw         $v0, %gp_rel(D_800A3404)($gp)
    /* 4CBB8 8005C3B8 4003838F */  lw         $v1, %gp_rel(D_800A340C)($gp)
    /* 4CBBC 8005C3BC 00000000 */  nop
    /* 4CBC0 8005C3C0 23104300 */  subu       $v0, $v0, $v1
    /* 4CBC4 8005C3C4 3C0382AF */  sw         $v0, %gp_rel(D_800A3408)($gp)
    /* 4CBC8 8005C3C8 00141500 */  sll        $v0, $s5, 16
    /* 4CBCC 8005C3CC 03940200 */  sra        $s2, $v0, 16
    /* 4CBD0 8005C3D0 03004012 */  beqz       $s2, .L8005C3E0
    /* 4CBD4 8005C3D4 21204002 */   addu      $a0, $s2, $zero
    /* 4CBD8 8005C3D8 1D70010C */  jal        func_8005C074
    /* 4CBDC 8005C3DC 2128C002 */   addu      $a1, $s6, $zero
  .L8005C3E0:
    /* 4CBE0 8005C3E0 21202002 */  addu       $a0, $s1, $zero
    /* 4CBE4 8005C3E4 21284002 */  addu       $a1, $s2, $zero
    /* 4CBE8 8005C3E8 0000228E */  lw         $v0, 0x0($s1)
    /* 4CBEC 8005C3EC 0800238E */  lw         $v1, 0x8($s1)
    /* 4CBF0 8005C3F0 21105100 */  addu       $v0, $v0, $s1
    /* 4CBF4 8005C3F4 000022AE */  sw         $v0, 0x0($s1)
    /* 4CBF8 8005C3F8 0400228E */  lw         $v0, 0x4($s1)
    /* 4CBFC 8005C3FC 21187100 */  addu       $v1, $v1, $s1
    /* 4CC00 8005C400 080023AE */  sw         $v1, 0x8($s1)
    /* 4CC04 8005C404 21105100 */  addu       $v0, $v0, $s1
    /* 4CC08 8005C408 6A71010C */  jal        func_8005C5A8
    /* 4CC0C 8005C40C 040022AE */   sw        $v0, 0x4($s1)
    /* 4CC10 8005C410 01000424 */  addiu      $a0, $zero, 0x1
    /* 4CC14 8005C414 6121020C */  jal        func_80088584
    /* 4CC18 8005C418 21804000 */   addu      $s0, $v0, $zero
    /* 4CC1C 8005C41C 00841000 */  sll        $s0, $s0, 16
    /* 4CC20 8005C420 03841000 */  sra        $s0, $s0, 16
    /* 4CC24 8005C424 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 4CC28 8005C428 07000216 */  bne        $s0, $v0, .L8005C448
    /* 4CC2C 8005C42C 80101000 */   sll       $v0, $s0, 2
    /* 4CC30 8005C430 0180043C */  lui        $a0, %hi(D_800158CC)
    /* 4CC34 8005C434 CC588424 */  addiu      $a0, $a0, %lo(D_800158CC)
    /* 4CC38 8005C438 82E4010C */  jal        debug_printf
    /* 4CC3C 8005C43C 21284002 */   addu      $a1, $s2, $zero
    /* 4CC40 8005C440 25710108 */  j          .L8005C494
    /* 4CC44 8005C444 21100000 */   addu      $v0, $zero, $zero
  .L8005C448:
    /* 4CC48 8005C448 0F80013C */  lui        $at, %hi(D_800EFC38)
    /* 4CC4C 8005C44C 21082200 */  addu       $at, $at, $v0
    /* 4CC50 8005C450 38FC31AC */  sw         $s1, %lo(D_800EFC38)($at)
    /* 4CC54 8005C454 3C03828F */  lw         $v0, %gp_rel(D_800A3408)($gp)
    /* 4CC58 8005C458 0C00258E */  lw         $a1, 0xC($s1)
    /* 4CC5C 8005C45C 4003838F */  lw         $v1, %gp_rel(D_800A340C)($gp)
    /* 4CC60 8005C460 21104500 */  addu       $v0, $v0, $a1
    /* 4CC64 8005C464 21186200 */  addu       $v1, $v1, $v0
    /* 4CC68 8005C468 3C0382AF */  sw         $v0, %gp_rel(D_800A3408)($gp)
    /* 4CC6C 8005C46C 380383AF */  sw         $v1, %gp_rel(D_800A3404)($gp)
    /* 4CC70 8005C470 7C16020C */  jal        func_800859F0
    /* 4CC74 8005C474 21204002 */   addu      $a0, $s2, $zero
    /* 4CC78 8005C478 80181200 */  sll        $v1, $s2, 2
    /* 4CC7C 8005C47C 0F80013C */  lui        $at, %hi(D_800EFB38)
    /* 4CC80 8005C480 21082300 */  addu       $at, $at, $v1
    /* 4CC84 8005C484 38FB22AC */  sw         $v0, %lo(D_800EFB38)($at)
    /* 4CC88 8005C488 0800228E */  lw         $v0, 0x8($s1)
    /* 4CC8C 8005C48C 00000000 */  nop
    /* 4CC90 8005C490 23105100 */  subu       $v0, $v0, $s1
  .L8005C494:
    /* 4CC94 8005C494 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 4CC98 8005C498 2800B68F */  lw         $s6, 0x28($sp)
    /* 4CC9C 8005C49C 2400B58F */  lw         $s5, 0x24($sp)
    /* 4CCA0 8005C4A0 2000B48F */  lw         $s4, 0x20($sp)
    /* 4CCA4 8005C4A4 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 4CCA8 8005C4A8 1800B28F */  lw         $s2, 0x18($sp)
    /* 4CCAC 8005C4AC 1400B18F */  lw         $s1, 0x14($sp)
    /* 4CCB0 8005C4B0 1000B08F */  lw         $s0, 0x10($sp)
    /* 4CCB4 8005C4B4 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 4CCB8 8005C4B8 0800E003 */  jr         $ra
    /* 4CCBC 8005C4BC 00000000 */   nop
endlabel func_8005C2A8
