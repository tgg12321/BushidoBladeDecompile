glabel func_8005BA8C
    /* 4C28C 8005BA8C 80FFBD27 */  addiu      $sp, $sp, -0x80
    /* 4C290 8005BA90 6C00B5AF */  sw         $s5, 0x6C($sp)
    /* 4C294 8005BA94 21A8A000 */  addu       $s5, $a1, $zero
    /* 4C298 8005BA98 7400B7AF */  sw         $s7, 0x74($sp)
    /* 4C29C 8005BA9C 21B8C000 */  addu       $s7, $a2, $zero
    /* 4C2A0 8005BAA0 7800BEAF */  sw         $fp, 0x78($sp)
    /* 4C2A4 8005BAA4 21F0E000 */  addu       $fp, $a3, $zero
    /* 4C2A8 8005BAA8 6400B3AF */  sw         $s3, 0x64($sp)
    /* 4C2AC 8005BAAC 21988000 */  addu       $s3, $a0, $zero
    /* 4C2B0 8005BAB0 21200000 */  addu       $a0, $zero, $zero
    /* 4C2B4 8005BAB4 7C00BFAF */  sw         $ra, 0x7C($sp)
    /* 4C2B8 8005BAB8 7000B6AF */  sw         $s6, 0x70($sp)
    /* 4C2BC 8005BABC 6800B4AF */  sw         $s4, 0x68($sp)
    /* 4C2C0 8005BAC0 6000B2AF */  sw         $s2, 0x60($sp)
    /* 4C2C4 8005BAC4 5C00B1AF */  sw         $s1, 0x5C($sp)
    /* 4C2C8 8005BAC8 3416020C */  jal        func_800858D0
    /* 4C2CC 8005BACC 5800B0AF */   sw        $s0, 0x58($sp)
    /* 4C2D0 8005BAD0 0F80143C */  lui        $s4, %hi(D_800EFC38)
    /* 4C2D4 8005BAD4 38FC9426 */  addiu      $s4, $s4, %lo(D_800EFC38)
    /* 4C2D8 8005BAD8 0F80123C */  lui        $s2, %hi(D_800EFB38)
    /* 4C2DC 8005BADC 38FB5226 */  addiu      $s2, $s2, %lo(D_800EFB38)
    /* 4C2E0 8005BAE0 0A80103C */  lui        $s0, %hi(D_8009AD18)
    /* 4C2E4 8005BAE4 18AD1026 */  addiu      $s0, $s0, %lo(D_8009AD18)
    /* 4C2E8 8005BAE8 03001126 */  addiu      $s1, $s0, 0x3
  .L8005BAEC:
    /* 4C2EC 8005BAEC 00000492 */  lbu        $a0, 0x0($s0)
    /* 4C2F0 8005BAF0 D91F020C */  jal        func_80087F64
    /* 4C2F4 8005BAF4 00000000 */   nop
    /* 4C2F8 8005BAF8 00000292 */  lbu        $v0, 0x0($s0)
    /* 4C2FC 8005BAFC 00000000 */  nop
    /* 4C300 8005BB00 80100200 */  sll        $v0, $v0, 2
    /* 4C304 8005BB04 21105400 */  addu       $v0, $v0, $s4
    /* 4C308 8005BB08 000040AC */  sw         $zero, 0x0($v0)
    /* 4C30C 8005BB0C 00000292 */  lbu        $v0, 0x0($s0)
    /* 4C310 8005BB10 01001026 */  addiu      $s0, $s0, 0x1
    /* 4C314 8005BB14 80100200 */  sll        $v0, $v0, 2
    /* 4C318 8005BB18 21105200 */  addu       $v0, $v0, $s2
    /* 4C31C 8005BB1C 000040AC */  sw         $zero, 0x0($v0)
    /* 4C320 8005BB20 2A101102 */  slt        $v0, $s0, $s1
    /* 4C324 8005BB24 F1FF4014 */  bnez       $v0, .L8005BAEC
    /* 4C328 8005BB28 02000424 */   addiu     $a0, $zero, 0x2
    /* 4C32C 8005BB2C 03001224 */  addiu      $s2, $zero, 0x3
    /* 4C330 8005BB30 AADB000C */  jal        func_80036EA8
    /* 4C334 8005BB34 0900A526 */   addiu     $a1, $s5, 0x9
    /* 4C338 8005BB38 D0DB000C */  jal        func_80036F40
    /* 4C33C 8005BB3C 21804000 */   addu      $s0, $v0, $zero
    /* 4C340 8005BB40 21200002 */  addu       $a0, $s0, $zero
    /* 4C344 8005BB44 66DB000C */  jal        replay_camera_Init
    /* 4C348 8005BB48 21286002 */   addu      $a1, $s3, $zero
    /* 4C34C 8005BB4C CADB000C */  jal        func_80036F28
    /* 4C350 8005BB50 21200002 */   addu      $a0, $s0, $zero
    /* 4C354 8005BB54 D0DB000C */  jal        func_80036F40
    /* 4C358 8005BB58 21A84000 */   addu      $s5, $v0, $zero
    /* 4C35C 8005BB5C 3000648E */  lw         $a0, 0x30($s3)
    /* 4C360 8005BB60 03001624 */  addiu      $s6, $zero, 0x3
    /* 4C364 8005BB64 21209300 */  addu       $a0, $a0, $s3
    /* 4C368 8005BB68 0888010C */  jal        func_80062020
    /* 4C36C 8005BB6C 300064AE */   sw        $a0, 0x30($s3)
    /* 4C370 8005BB70 0200FE16 */  bne        $s7, $fp, .L8005BB7C
    /* 4C374 8005BB74 3800B3AF */   sw        $s3, 0x38($sp)
    /* 4C378 8005BB78 02001624 */  addiu      $s6, $zero, 0x2
  .L8005BB7C:
    /* 4C37C 8005BB7C 0000628E */  lw         $v0, 0x0($s3)
    /* 4C380 8005BB80 00000000 */  nop
    /* 4C384 8005BB84 1000A2AF */  sw         $v0, 0x10($sp)
    /* 4C388 8005BB88 0400628E */  lw         $v0, 0x4($s3)
    /* 4C38C 8005BB8C 00000000 */  nop
    /* 4C390 8005BB90 1400A2AF */  sw         $v0, 0x14($sp)
    /* 4C394 8005BB94 C0101700 */  sll        $v0, $s7, 3
    /* 4C398 8005BB98 21105300 */  addu       $v0, $v0, $s3
    /* 4C39C 8005BB9C 0800438C */  lw         $v1, 0x8($v0)
    /* 4C3A0 8005BBA0 00000000 */  nop
    /* 4C3A4 8005BBA4 1800A3AF */  sw         $v1, 0x18($sp)
    /* 4C3A8 8005BBA8 0C00428C */  lw         $v0, 0xC($v0)
    /* 4C3AC 8005BBAC FF00C432 */  andi       $a0, $s6, 0xFF
    /* 4C3B0 8005BBB0 09009214 */  bne        $a0, $s2, .L8005BBD8
    /* 4C3B4 8005BBB4 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 4C3B8 8005BBB8 C0101E00 */  sll        $v0, $fp, 3
    /* 4C3BC 8005BBBC 21105300 */  addu       $v0, $v0, $s3
    /* 4C3C0 8005BBC0 0800438C */  lw         $v1, 0x8($v0)
    /* 4C3C4 8005BBC4 00000000 */  nop
    /* 4C3C8 8005BBC8 2000A3AF */  sw         $v1, 0x20($sp)
    /* 4C3CC 8005BBCC 0C00428C */  lw         $v0, 0xC($v0)
    /* 4C3D0 8005BBD0 00000000 */  nop
    /* 4C3D4 8005BBD4 2400A2AF */  sw         $v0, 0x24($sp)
  .L8005BBD8:
    /* 4C3D8 8005BBD8 12008010 */  beqz       $a0, .L8005BC24
    /* 4C3DC 8005BBDC 21900000 */   addu      $s2, $zero, $zero
    /* 4C3E0 8005BBE0 21A08000 */  addu       $s4, $a0, $zero
    /* 4C3E4 8005BBE4 1000B027 */  addiu      $s0, $sp, 0x10
    /* 4C3E8 8005BBE8 21880002 */  addu       $s1, $s0, $zero
  .L8005BBEC:
    /* 4C3EC 8005BBEC 0000048E */  lw         $a0, 0x0($s0)
    /* 4C3F0 8005BBF0 21307502 */  addu       $a2, $s3, $s5
    /* 4C3F4 8005BBF4 21209300 */  addu       $a0, $a0, $s3
    /* 4C3F8 8005BBF8 000004AE */  sw         $a0, 0x0($s0)
    /* 4C3FC 8005BBFC 08001026 */  addiu      $s0, $s0, 0x8
    /* 4C400 8005BC00 0A80013C */  lui        $at, %hi(D_8009AD18)
    /* 4C404 8005BC04 21083200 */  addu       $at, $at, $s2
    /* 4C408 8005BC08 18AD2590 */  lbu        $a1, %lo(D_8009AD18)($at)
    /* 4C40C 8005BC0C AA70010C */  jal        func_8005C2A8
    /* 4C410 8005BC10 01005226 */   addiu     $s2, $s2, 0x1
    /* 4C414 8005BC14 180022AE */  sw         $v0, 0x18($s1)
    /* 4C418 8005BC18 2A105402 */  slt        $v0, $s2, $s4
    /* 4C41C 8005BC1C F3FF4014 */  bnez       $v0, .L8005BBEC
    /* 4C420 8005BC20 04003126 */   addiu     $s1, $s1, 0x4
  .L8005BC24:
    /* 4C424 8005BC24 2A00C012 */  beqz       $s6, .L8005BCD0
    /* 4C428 8005BC28 21900000 */   addu      $s2, $zero, $zero
    /* 4C42C 8005BC2C 1000B027 */  addiu      $s0, $sp, 0x10
    /* 4C430 8005BC30 21B80002 */  addu       $s7, $s0, $zero
    /* 4C434 8005BC34 21A00000 */  addu       $s4, $zero, $zero
    /* 4C438 8005BC38 21880002 */  addu       $s1, $s0, $zero
    /* 4C43C 8005BC3C 21A80000 */  addu       $s5, $zero, $zero
  .L8005BC40:
    /* 4C440 8005BC40 2130E002 */  addu       $a2, $s7, $zero
    /* 4C444 8005BC44 1800028E */  lw         $v0, 0x18($s0)
    /* 4C448 8005BC48 00000000 */  nop
    /* 4C44C 8005BC4C 10004010 */  beqz       $v0, .L8005BC90
    /* 4C450 8005BC50 21200000 */   addu      $a0, $zero, $zero
    /* 4C454 8005BC54 2138A002 */  addu       $a3, $s5, $zero
    /* 4C458 8005BC58 21286002 */  addu       $a1, $s3, $zero
  .L8005BC5C:
    /* 4C45C 8005BC5C 2118C000 */  addu       $v1, $a2, $zero
    /* 4C460 8005BC60 2110C700 */  addu       $v0, $a2, $a3
    /* 4C464 8005BC64 0000428C */  lw         $v0, 0x0($v0)
    /* 4C468 8005BC68 21188302 */  addu       $v1, $s4, $v1
    /* 4C46C 8005BC6C 21104400 */  addu       $v0, $v0, $a0
    /* 4C470 8005BC70 00004290 */  lbu        $v0, 0x0($v0)
    /* 4C474 8005BC74 01008424 */  addiu      $a0, $a0, 0x1
    /* 4C478 8005BC78 0000A2A0 */  sb         $v0, 0x0($a1)
    /* 4C47C 8005BC7C 1800628C */  lw         $v0, 0x18($v1)
    /* 4C480 8005BC80 00000000 */  nop
    /* 4C484 8005BC84 2B108200 */  sltu       $v0, $a0, $v0
    /* 4C488 8005BC88 F4FF4014 */  bnez       $v0, .L8005BC5C
    /* 4C48C 8005BC8C 0100A524 */   addiu     $a1, $a1, 0x1
  .L8005BC90:
    /* 4C490 8005BC90 04009426 */  addiu      $s4, $s4, 0x4
    /* 4C494 8005BC94 0800B526 */  addiu      $s5, $s5, 0x8
    /* 4C498 8005BC98 0A80013C */  lui        $at, %hi(D_8009AD18)
    /* 4C49C 8005BC9C 21083200 */  addu       $at, $at, $s2
    /* 4C4A0 8005BCA0 18AD2590 */  lbu        $a1, %lo(D_8009AD18)($at)
    /* 4C4A4 8005BCA4 0000248E */  lw         $a0, 0x0($s1)
    /* 4C4A8 8005BCA8 01005226 */  addiu      $s2, $s2, 0x1
    /* 4C4AC 8005BCAC 3071010C */  jal        func_8005C4C0
    /* 4C4B0 8005BCB0 23206402 */   subu      $a0, $s3, $a0
    /* 4C4B4 8005BCB4 000033AE */  sw         $s3, 0x0($s1)
    /* 4C4B8 8005BCB8 1800028E */  lw         $v0, 0x18($s0)
    /* 4C4BC 8005BCBC 04001026 */  addiu      $s0, $s0, 0x4
    /* 4C4C0 8005BCC0 21986202 */  addu       $s3, $s3, $v0
    /* 4C4C4 8005BCC4 2A105602 */  slt        $v0, $s2, $s6
    /* 4C4C8 8005BCC8 DDFF4014 */  bnez       $v0, .L8005BC40
    /* 4C4CC 8005BCCC 08003126 */   addiu     $s1, $s1, 0x8
  .L8005BCD0:
    /* 4C4D0 8005BCD0 FF00C332 */  andi       $v1, $s6, 0xFF
    /* 4C4D4 8005BCD4 02000224 */  addiu      $v0, $zero, 0x2
    /* 4C4D8 8005BCD8 05006214 */  bne        $v1, $v0, .L8005BCF0
    /* 4C4DC 8005BCDC 00000000 */   nop
    /* 4C4E0 8005BCE0 0F80023C */  lui        $v0, %hi(D_800EFC44)
    /* 4C4E4 8005BCE4 44FC428C */  lw         $v0, %lo(D_800EFC44)($v0)
    /* 4C4E8 8005BCE8 0F80013C */  lui        $at, %hi(D_800EFC50)
    /* 4C4EC 8005BCEC 50FC22AC */  sw         $v0, %lo(D_800EFC50)($at)
  .L8005BCF0:
    /* 4C4F0 8005BCF0 3800A88F */  lw         $t0, 0x38($sp)
    /* 4C4F4 8005BCF4 00000000 */  nop
    /* 4C4F8 8005BCF8 23106802 */  subu       $v0, $s3, $t0
    /* 4C4FC 8005BCFC 7C00BF8F */  lw         $ra, 0x7C($sp)
    /* 4C500 8005BD00 7800BE8F */  lw         $fp, 0x78($sp)
    /* 4C504 8005BD04 7400B78F */  lw         $s7, 0x74($sp)
    /* 4C508 8005BD08 7000B68F */  lw         $s6, 0x70($sp)
    /* 4C50C 8005BD0C 6C00B58F */  lw         $s5, 0x6C($sp)
    /* 4C510 8005BD10 6800B48F */  lw         $s4, 0x68($sp)
    /* 4C514 8005BD14 6400B38F */  lw         $s3, 0x64($sp)
    /* 4C518 8005BD18 6000B28F */  lw         $s2, 0x60($sp)
    /* 4C51C 8005BD1C 5C00B18F */  lw         $s1, 0x5C($sp)
    /* 4C520 8005BD20 5800B08F */  lw         $s0, 0x58($sp)
    /* 4C524 8005BD24 8000BD27 */  addiu      $sp, $sp, 0x80
    /* 4C528 8005BD28 0800E003 */  jr         $ra
    /* 4C52C 8005BD2C 00000000 */   nop
endlabel func_8005BA8C
