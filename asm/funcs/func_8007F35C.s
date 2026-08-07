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
