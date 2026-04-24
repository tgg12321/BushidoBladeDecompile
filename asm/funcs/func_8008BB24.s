glabel func_8008BB24
    /* 7C324 8008BB24 F0FFBD27 */  addiu      $sp, $sp, -0x10
    /* 7C328 8008BB28 FFFF8430 */  andi       $a0, $a0, 0xFFFF
    /* 7C32C 8008BB2C C0210400 */  sll        $a0, $a0, 7
    /* 7C330 8008BB30 FFFFA530 */  andi       $a1, $a1, 0xFFFF
    /* 7C334 8008BB34 21208500 */  addu       $a0, $a0, $a1
    /* 7C338 8008BB38 FFFFC630 */  andi       $a2, $a2, 0xFFFF
    /* 7C33C 8008BB3C C0310600 */  sll        $a2, $a2, 7
    /* 7C340 8008BB40 FFFFE730 */  andi       $a3, $a3, 0xFFFF
    /* 7C344 8008BB44 2130C700 */  addu       $a2, $a2, $a3
    /* 7C348 8008BB48 2330C400 */  subu       $a2, $a2, $a0
    /* 7C34C 8008BB4C 0200C104 */  bgez       $a2, .L8008BB58
    /* 7C350 8008BB50 2128C000 */   addu      $a1, $a2, $zero
    /* 7C354 8008BB54 23280600 */  negu       $a1, $a2
  .L8008BB58:
    /* 7C358 8008BB58 AA2A023C */  lui        $v0, (0x2AAAAAAB >> 16)
    /* 7C35C 8008BB5C ABAA4234 */  ori        $v0, $v0, (0x2AAAAAAB & 0xFFFF)
    /* 7C360 8008BB60 1800A200 */  mult       $a1, $v0
    /* 7C364 8008BB64 C3170500 */  sra        $v0, $a1, 31
    /* 7C368 8008BB68 10480000 */  mfhi       $t1
    /* 7C36C 8008BB6C 031A0900 */  sra        $v1, $t1, 8
    /* 7C370 8008BB70 23186200 */  subu       $v1, $v1, $v0
    /* 7C374 8008BB74 21206000 */  addu       $a0, $v1, $zero
    /* 7C378 8008BB78 40100400 */  sll        $v0, $a0, 1
    /* 7C37C 8008BB7C 21104400 */  addu       $v0, $v0, $a0
    /* 7C380 8008BB80 40120200 */  sll        $v0, $v0, 9
    /* 7C384 8008BB84 0400C004 */  bltz       $a2, .L8008BB98
    /* 7C388 8008BB88 2318A200 */   subu      $v1, $a1, $v0
    /* 7C38C 8008BB8C 00100224 */  addiu      $v0, $zero, 0x1000
    /* 7C390 8008BB90 EC2E0208 */  j          .L8008BBB0
    /* 7C394 8008BB94 04108200 */   sllv      $v0, $v0, $a0
  .L8008BB98:
    /* 7C398 8008BB98 03006010 */  beqz       $v1, .L8008BBA8
    /* 7C39C 8008BB9C 00060224 */   addiu     $v0, $zero, 0x600
    /* 7C3A0 8008BBA0 01008424 */  addiu      $a0, $a0, 0x1
    /* 7C3A4 8008BBA4 23184300 */  subu       $v1, $v0, $v1
  .L8008BBA8:
    /* 7C3A8 8008BBA8 00100224 */  addiu      $v0, $zero, 0x1000
    /* 7C3AC 8008BBAC 07108200 */  srav       $v0, $v0, $a0
  .L8008BBB0:
    /* 7C3B0 8008BBB0 FFFF4630 */  andi       $a2, $v0, 0xFFFF
    /* 7C3B4 8008BBB4 3B100424 */  addiu      $a0, $zero, 0x103B
    /* 7C3B8 8008BBB8 1800C400 */  mult       $a2, $a0
    /* 7C3BC 8008BBBC 003B0600 */  sll        $a3, $a2, 12
    /* 7C3C0 8008BBC0 21280000 */  addu       $a1, $zero, $zero
    /* 7C3C4 8008BBC4 02006104 */  bgez       $v1, .L8008BBD0
    /* 7C3C8 8008BBC8 21106000 */   addu      $v0, $v1, $zero
    /* 7C3CC 8008BBCC 23100200 */  negu       $v0, $v0
  .L8008BBD0:
    /* 7C3D0 8008BBD0 42190200 */  srl        $v1, $v0, 5
    /* 7C3D4 8008BBD4 1F004830 */  andi       $t0, $v0, 0x1F
    /* 7C3D8 8008BBD8 12480000 */  mflo       $t1
    /* 7C3DC 8008BBDC 10006010 */  beqz       $v1, .L8008BC20
    /* 7C3E0 8008BBE0 0800A9AF */   sw        $t1, 0x8($sp)
    /* 7C3E4 8008BBE4 1800C400 */  mult       $a2, $a0
  .L8008BBE8:
    /* 7C3E8 8008BBE8 80110400 */  sll        $v0, $a0, 6
    /* 7C3EC 8008BBEC 21104400 */  addu       $v0, $v0, $a0
    /* 7C3F0 8008BBF0 00110200 */  sll        $v0, $v0, 4
    /* 7C3F4 8008BBF4 23104400 */  subu       $v0, $v0, $a0
    /* 7C3F8 8008BBF8 80100200 */  sll        $v0, $v0, 2
    /* 7C3FC 8008BBFC 12380000 */  mflo       $a3
    /* 7C400 8008BC00 23204400 */  subu       $a0, $v0, $a0
    /* 7C404 8008BC04 02230400 */  srl        $a0, $a0, 12
    /* 7C408 8008BC08 1800C400 */  mult       $a2, $a0
    /* 7C40C 8008BC0C 0100A524 */  addiu      $a1, $a1, 0x1
    /* 7C410 8008BC10 2A10A300 */  slt        $v0, $a1, $v1
    /* 7C414 8008BC14 12480000 */  mflo       $t1
    /* 7C418 8008BC18 F3FF4014 */  bnez       $v0, .L8008BBE8
    /* 7C41C 8008BC1C 0800A9AF */   sw        $t1, 0x8($sp)
  .L8008BC20:
    /* 7C420 8008BC20 0800A98F */  lw         $t1, 0x8($sp)
    /* 7C424 8008BC24 00000000 */  nop
    /* 7C428 8008BC28 23102701 */  subu       $v0, $t1, $a3
    /* 7C42C 8008BC2C 42110200 */  srl        $v0, $v0, 5
    /* 7C430 8008BC30 18004800 */  mult       $v0, $t0
    /* 7C434 8008BC34 12480000 */  mflo       $t1
    /* 7C438 8008BC38 2110E900 */  addu       $v0, $a3, $t1
    /* 7C43C 8008BC3C 021B0200 */  srl        $v1, $v0, 12
    /* 7C440 8008BC40 0040622C */  sltiu      $v0, $v1, 0x4000
    /* 7C444 8008BC44 03004014 */  bnez       $v0, .L8008BC54
    /* 7C448 8008BC48 FFFF6230 */   andi      $v0, $v1, 0xFFFF
    /* 7C44C 8008BC4C FF3F0324 */  addiu      $v1, $zero, 0x3FFF
    /* 7C450 8008BC50 FFFF6230 */  andi       $v0, $v1, 0xFFFF
  .L8008BC54:
    /* 7C454 8008BC54 1000BD27 */  addiu      $sp, $sp, 0x10
    /* 7C458 8008BC58 0800E003 */  jr         $ra
    /* 7C45C 8008BC5C 00000000 */   nop
endlabel func_8008BB24
