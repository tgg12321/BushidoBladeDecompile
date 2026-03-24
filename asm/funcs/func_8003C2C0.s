glabel func_8003C2C0
    /* 2CAC0 8003C2C0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2CAC4 8003C2C4 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2CAC8 8003C2C8 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2CACC 8003C2CC 0A80043C */  lui        $a0, %hi(D_800A38B4)
    /* 2CAD0 8003C2D0 B438848C */  lw         $a0, %lo(D_800A38B4)($a0)
    /* 2CAD4 8003C2D4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2CAD8 8003C2D8 01004224 */  addiu      $v0, $v0, 0x1
    /* 2CADC 8003C2DC 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2CAE0 8003C2E0 B83722AC */  sw         $v0, %lo(D_800A37B8)($at)
    /* 2CAE4 8003C2E4 277F010C */  jal        func_8005FC9C
    /* 2CAE8 8003C2E8 01000524 */   addiu     $a1, $zero, 0x1
    /* 2CAEC 8003C2EC 0A80033C */  lui        $v1, %hi(D_800A38B4)
    /* 2CAF0 8003C2F0 B438638C */  lw         $v1, %lo(D_800A38B4)($v1)
    /* 2CAF4 8003C2F4 80100200 */  sll        $v0, $v0, 2
    /* 2CAF8 8003C2F8 21186200 */  addu       $v1, $v1, $v0
    /* 2CAFC 8003C2FC 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 2CB00 8003C300 B43823AC */  sw         $v1, %lo(D_800A38B4)($at)
    /* 2CB04 8003C304 DA53010C */  jal        func_80054F68
    /* 2CB08 8003C308 00000000 */   nop
    /* 2CB0C 8003C30C 07004010 */  beqz       $v0, .L8003C32C
    /* 2CB10 8003C310 4000033C */   lui       $v1, (0x400040 >> 16)
    /* 2CB14 8003C314 1080023C */  lui        $v0, %hi(D_80102794)
    /* 2CB18 8003C318 9427428C */  lw         $v0, %lo(D_80102794)($v0)
    /* 2CB1C 8003C31C 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 2CB20 8003C320 24104300 */  and        $v0, $v0, $v1
    /* 2CB24 8003C324 2F004010 */  beqz       $v0, .L8003C3E4
    /* 2CB28 8003C328 00000000 */   nop
  .L8003C32C:
    /* 2CB2C 8003C32C 0A80023C */  lui        $v0, %hi(D_800A38A4)
    /* 2CB30 8003C330 A4384290 */  lbu        $v0, %lo(D_800A38A4)($v0)
    /* 2CB34 8003C334 00000000 */  nop
    /* 2CB38 8003C338 FAFF4224 */  addiu      $v0, $v0, -0x6
    /* 2CB3C 8003C33C 0200422C */  sltiu      $v0, $v0, 0x2
    /* 2CB40 8003C340 13004010 */  beqz       $v0, .L8003C390
    /* 2CB44 8003C344 00000000 */   nop
    /* 2CB48 8003C348 0A80023C */  lui        $v0, %hi(D_800A3781)
    /* 2CB4C 8003C34C 81374290 */  lbu        $v0, %lo(D_800A3781)($v0)
    /* 2CB50 8003C350 00000000 */  nop
    /* 2CB54 8003C354 0E004010 */  beqz       $v0, .L8003C390
    /* 2CB58 8003C358 00000000 */   nop
    /* 2CB5C 8003C35C 1080023C */  lui        $v0, %hi(D_80101ED2)
    /* 2CB60 8003C360 D21E4284 */  lh         $v0, %lo(D_80101ED2)($v0)
    /* 2CB64 8003C364 0980013C */  lui        $at, %hi(D_8008D9EC)
    /* 2CB68 8003C368 21082200 */  addu       $at, $at, $v0
    /* 2CB6C 8003C36C ECD92290 */  lbu        $v0, %lo(D_8008D9EC)($at)
    /* 2CB70 8003C370 00000000 */  nop
    /* 2CB74 8003C374 02004010 */  beqz       $v0, .L8003C380
    /* 2CB78 8003C378 08000324 */   addiu     $v1, $zero, 0x8
    /* 2CB7C 8003C37C 09000324 */  addiu      $v1, $zero, 0x9
  .L8003C380:
    /* 2CB80 8003C380 0A80013C */  lui        $at, %hi(D_800A38A4)
    /* 2CB84 8003C384 A43823A0 */  sb         $v1, %lo(D_800A38A4)($at)
    /* 2CB88 8003C388 F7F00008 */  j          .L8003C3DC
    /* 2CB8C 8003C38C 12000224 */   addiu     $v0, $zero, 0x12
  .L8003C390:
    /* 2CB90 8003C390 0A80033C */  lui        $v1, %hi(D_800A38A4)
    /* 2CB94 8003C394 A4386390 */  lbu        $v1, %lo(D_800A38A4)($v1)
    /* 2CB98 8003C398 00000000 */  nop
    /* 2CB9C 8003C39C FCFF6224 */  addiu      $v0, $v1, -0x4
    /* 2CBA0 8003C3A0 0200422C */  sltiu      $v0, $v0, 0x2
    /* 2CBA4 8003C3A4 0D004014 */  bnez       $v0, .L8003C3DC
    /* 2CBA8 8003C3A8 18000224 */   addiu     $v0, $zero, 0x18
    /* 2CBAC 8003C3AC FAFF6224 */  addiu      $v0, $v1, -0x6
    /* 2CBB0 8003C3B0 0200422C */  sltiu      $v0, $v0, 0x2
    /* 2CBB4 8003C3B4 09004014 */  bnez       $v0, .L8003C3DC
    /* 2CBB8 8003C3B8 1A000224 */   addiu     $v0, $zero, 0x1A
    /* 2CBBC 8003C3BC F8FF6224 */  addiu      $v0, $v1, -0x8
    /* 2CBC0 8003C3C0 0200422C */  sltiu      $v0, $v0, 0x2
    /* 2CBC4 8003C3C4 05004014 */  bnez       $v0, .L8003C3DC
    /* 2CBC8 8003C3C8 1A000224 */   addiu     $v0, $zero, 0x1A
    /* 2CBCC 8003C3CC 0200622C */  sltiu      $v0, $v1, 0x2
    /* 2CBD0 8003C3D0 02004014 */  bnez       $v0, .L8003C3DC
    /* 2CBD4 8003C3D4 0A000224 */   addiu     $v0, $zero, 0xA
    /* 2CBD8 8003C3D8 08000224 */  addiu      $v0, $zero, 0x8
  .L8003C3DC:
    /* 2CBDC 8003C3DC 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2CBE0 8003C3E0 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
  .L8003C3E4:
    /* 2CBE4 8003C3E4 0A80033C */  lui        $v1, %hi(D_800A3834)
    /* 2CBE8 8003C3E8 34386384 */  lh         $v1, %lo(D_800A3834)($v1)
    /* 2CBEC 8003C3EC 13000224 */  addiu      $v0, $zero, 0x13
    /* 2CBF0 8003C3F0 0A006210 */  beq        $v1, $v0, .L8003C41C
    /* 2CBF4 8003C3F4 00000000 */   nop
    /* 2CBF8 8003C3F8 B0DC000C */  jal        func_800372C0
    /* 2CBFC 8003C3FC 00000000 */   nop
    /* 2CC00 8003C400 0A80033C */  lui        $v1, %hi(D_800A3834)
    /* 2CC04 8003C404 34386384 */  lh         $v1, %lo(D_800A3834)($v1)
    /* 2CC08 8003C408 12000224 */  addiu      $v0, $zero, 0x12
    /* 2CC0C 8003C40C 03006210 */  beq        $v1, $v0, .L8003C41C
    /* 2CC10 8003C410 00000000 */   nop
    /* 2CC14 8003C414 3752010C */  jal        func_800548DC
    /* 2CC18 8003C418 00000000 */   nop
  .L8003C41C:
    /* 2CC1C 8003C41C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2CC20 8003C420 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2CC24 8003C424 0800E003 */  jr         $ra
    /* 2CC28 8003C428 00000000 */   nop
endlabel func_8003C2C0
