glabel md_game_rob_data_init
    /* FAE4 8001F2E4 B8FFBD27 */  addiu      $sp, $sp, -0x48
    /* FAE8 8001F2E8 3000B0AF */  sw         $s0, 0x30($sp)
    /* FAEC 8001F2EC 21808000 */  addu       $s0, $a0, $zero
    /* FAF0 8001F2F0 3C00B3AF */  sw         $s3, 0x3C($sp)
    /* FAF4 8001F2F4 2198A000 */  addu       $s3, $a1, $zero
    /* FAF8 8001F2F8 4400BFAF */  sw         $ra, 0x44($sp)
    /* FAFC 8001F2FC 4000B4AF */  sw         $s4, 0x40($sp)
    /* FB00 8001F300 3800B2AF */  sw         $s2, 0x38($sp)
    /* FB04 8001F304 3400B1AF */  sw         $s1, 0x34($sp)
    /* FB08 8001F308 6C020286 */  lh         $v0, 0x26C($s0)
    /* FB0C 8001F30C 00000000 */  nop
    /* FB10 8001F310 05004014 */  bnez       $v0, .L8001F328
    /* FB14 8001F314 21A0C000 */   addu      $s4, $a2, $zero
    /* FB18 8001F318 CD9C000C */  jal        func_80027334
    /* FB1C 8001F31C 21206002 */   addu      $a0, $s3, $zero
    /* FB20 8001F320 CD9C000C */  jal        func_80027334
    /* FB24 8001F324 21208002 */   addu      $a0, $s4, $zero
  .L8001F328:
    /* FB28 8001F328 6A000396 */  lhu        $v1, 0x6A($s0)
    /* FB2C 8001F32C 15000224 */  addiu      $v0, $zero, 0x15
    /* FB30 8001F330 03006210 */  beq        $v1, $v0, .L8001F340
    /* FB34 8001F334 25000224 */   addiu     $v0, $zero, 0x25
    /* FB38 8001F338 56006214 */  bne        $v1, $v0, .L8001F494
    /* FB3C 8001F33C 21900000 */   addu      $s2, $zero, $zero
  .L8001F340:
    /* FB40 8001F340 0C000386 */  lh         $v1, 0xC($s0)
    /* FB44 8001F344 1F000224 */  addiu      $v0, $zero, 0x1F
    /* FB48 8001F348 03006214 */  bne        $v1, $v0, .L8001F358
    /* FB4C 8001F34C 00010524 */   addiu     $a1, $zero, 0x100
    /* FB50 8001F350 267D0008 */  j          .L8001F498
    /* FB54 8001F354 21900000 */   addu      $s2, $zero, $zero
  .L8001F358:
    /* FB58 8001F358 D8010286 */  lh         $v0, 0x1D8($s0)
    /* FB5C 8001F35C CA010386 */  lh         $v1, 0x1CA($s0)
    /* FB60 8001F360 00000000 */  nop
    /* FB64 8001F364 23104300 */  subu       $v0, $v0, $v1
    /* FB68 8001F368 FF0F5130 */  andi       $s1, $v0, 0xFFF
    /* FB6C 8001F36C 0008222A */  slti       $v0, $s1, 0x800
    /* FB70 8001F370 03004014 */  bnez       $v0, .L8001F380
    /* FB74 8001F374 01FE222A */   slti      $v0, $s1, -0x1FF
    /* FB78 8001F378 00F03126 */  addiu      $s1, $s1, -0x1000
    /* FB7C 8001F37C 01FE222A */  slti       $v0, $s1, -0x1FF
  .L8001F380:
    /* FB80 8001F380 03004010 */  beqz       $v0, .L8001F390
    /* FB84 8001F384 0002222A */   slti      $v0, $s1, 0x200
    /* FB88 8001F388 E77C0008 */  j          .L8001F39C
    /* FB8C 8001F38C 01FE1124 */   addiu     $s1, $zero, -0x1FF
  .L8001F390:
    /* FB90 8001F390 02004014 */  bnez       $v0, .L8001F39C
    /* FB94 8001F394 00000000 */   nop
    /* FB98 8001F398 FF011124 */  addiu      $s1, $zero, 0x1FF
  .L8001F39C:
    /* FB9C 8001F39C 0000048E */  lw         $a0, 0x0($s0)
    /* FBA0 8001F3A0 8001028E */  lw         $v0, 0x180($s0)
    /* FBA4 8001F3A4 8001838C */  lw         $v1, 0x180($a0)
    /* FBA8 8001F3A8 00000000 */  nop
    /* FBAC 8001F3AC 23186200 */  subu       $v1, $v1, $v0
    /* FBB0 8001F3B0 18006300 */  mult       $v1, $v1
    /* FBB4 8001F3B4 8801838C */  lw         $v1, 0x188($a0)
    /* FBB8 8001F3B8 8801028E */  lw         $v0, 0x188($s0)
    /* FBBC 8001F3BC 12280000 */  mflo       $a1
    /* FBC0 8001F3C0 23186200 */  subu       $v1, $v1, $v0
    /* FBC4 8001F3C4 00000000 */  nop
    /* FBC8 8001F3C8 18006300 */  mult       $v1, $v1
    /* FBCC 8001F3CC 12180000 */  mflo       $v1
    /* FBD0 8001F3D0 2120A300 */  addu       $a0, $a1, $v1
    /* FBD4 8001F3D4 0004822C */  sltiu      $v0, $a0, 0x400
    /* FBD8 8001F3D8 06004010 */  beqz       $v0, .L8001F3F4
    /* FBDC 8001F3DC 00000000 */   nop
    /* FBE0 8001F3E0 0980013C */  lui        $at, %hi(D_8008D118)
    /* FBE4 8001F3E4 21082400 */  addu       $at, $at, $a0
    /* FBE8 8001F3E8 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* FBEC 8001F3EC 147D0008 */  j          .L8001F450
    /* FBF0 8001F3F0 C2200200 */   srl       $a0, $v0, 3
  .L8001F3F4:
    /* FBF4 8001F3F4 09008004 */  bltz       $a0, .L8001F41C
    /* FBF8 8001F3F8 21180000 */   addu      $v1, $zero, $zero
    /* FBFC 8001F3FC 21608000 */  addu       $t4, $a0, $zero
    /* FC00 8001F400 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* FC04 8001F404 00000000 */  nop
    /* FC08 8001F408 00000000 */  nop
    /* FC0C 8001F40C 1000A227 */  addiu      $v0, $sp, 0x10
    /* FC10 8001F410 21604000 */  addu       $t4, $v0, $zero
    /* FC14 8001F414 00009FE9 */  swc2       $31, 0x0($t4)
    /* FC18 8001F418 1000A38F */  lw         $v1, 0x10($sp)
  .L8001F41C:
    /* FC1C 8001F41C FEFF0224 */  addiu      $v0, $zero, -0x2
    /* FC20 8001F420 24106200 */  and        $v0, $v1, $v0
    /* FC24 8001F424 16000324 */  addiu      $v1, $zero, 0x16
    /* FC28 8001F428 23186200 */  subu       $v1, $v1, $v0
    /* FC2C 8001F42C 06106400 */  srlv       $v0, $a0, $v1
    /* FC30 8001F430 0980013C */  lui        $at, %hi(D_8008D118)
    /* FC34 8001F434 21082200 */  addu       $at, $at, $v0
    /* FC38 8001F438 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* FC3C 8001F43C 42180300 */  srl        $v1, $v1, 1
    /* FC40 8001F440 13000224 */  addiu      $v0, $zero, 0x13
    /* FC44 8001F444 23104300 */  subu       $v0, $v0, $v1
    /* FC48 8001F448 00240400 */  sll        $a0, $a0, 16
    /* FC4C 8001F44C 06204400 */  srlv       $a0, $a0, $v0
  .L8001F450:
    /* FC50 8001F450 0000028E */  lw         $v0, 0x0($s0)
    /* FC54 8001F454 8401058E */  lw         $a1, 0x184($s0)
    /* FC58 8001F458 8401428C */  lw         $v0, 0x184($v0)
    /* FC5C 8001F45C 57FF010C */  jal        func_8007FD5C
    /* FC60 8001F460 23284500 */   subu      $a1, $v0, $a1
    /* FC64 8001F464 00040324 */  addiu      $v1, $zero, 0x400
    /* FC68 8001F468 23286200 */  subu       $a1, $v1, $v0
    /* FC6C 8001F46C 01FFA228 */  slti       $v0, $a1, -0xFF
    /* FC70 8001F470 03004010 */  beqz       $v0, .L8001F480
    /* FC74 8001F474 0001A228 */   slti      $v0, $a1, 0x100
    /* FC78 8001F478 237D0008 */  j          .L8001F48C
    /* FC7C 8001F47C 01FF0524 */   addiu     $a1, $zero, -0xFF
  .L8001F480:
    /* FC80 8001F480 06004014 */  bnez       $v0, .L8001F49C
    /* FC84 8001F484 21900000 */   addu      $s2, $zero, $zero
    /* FC88 8001F488 FF000524 */  addiu      $a1, $zero, 0xFF
  .L8001F48C:
    /* FC8C 8001F48C 277D0008 */  j          .L8001F49C
    /* FC90 8001F490 21900000 */   addu      $s2, $zero, $zero
  .L8001F494:
    /* FC94 8001F494 21280000 */  addu       $a1, $zero, $zero
  .L8001F498:
    /* FC98 8001F498 21880000 */  addu       $s1, $zero, $zero
  .L8001F49C:
    /* FC9C 8001F49C E6010286 */  lh         $v0, 0x1E6($s0)
    /* FCA0 8001F4A0 00000000 */  nop
    /* FCA4 8001F4A4 23182202 */  subu       $v1, $s1, $v0
    /* FCA8 8001F4A8 FF0F6430 */  andi       $a0, $v1, 0xFFF
    /* FCAC 8001F4AC 21304000 */  addu       $a2, $v0, $zero
    /* FCB0 8001F4B0 00088228 */  slti       $v0, $a0, 0x800
    /* FCB4 8001F4B4 02004014 */  bnez       $v0, .L8001F4C0
    /* FCB8 8001F4B8 00000000 */   nop
    /* FCBC 8001F4BC 00F08424 */  addiu      $a0, $a0, -0x1000
  .L8001F4C0:
    /* FCC0 8001F4C0 02008104 */  bgez       $a0, .L8001F4CC
    /* FCC4 8001F4C4 21108000 */   addu      $v0, $a0, $zero
    /* FCC8 8001F4C8 07008224 */  addiu      $v0, $a0, 0x7
  .L8001F4CC:
    /* FCCC 8001F4CC C3100200 */  sra        $v0, $v0, 3
    /* FCD0 8001F4D0 E8010386 */  lh         $v1, 0x1E8($s0)
    /* FCD4 8001F4D4 2110C200 */  addu       $v0, $a2, $v0
    /* FCD8 8001F4D8 E60102A6 */  sh         $v0, 0x1E6($s0)
    /* FCDC 8001F4DC 2310A300 */  subu       $v0, $a1, $v1
    /* FCE0 8001F4E0 FF0F4430 */  andi       $a0, $v0, 0xFFF
    /* FCE4 8001F4E4 00088228 */  slti       $v0, $a0, 0x800
    /* FCE8 8001F4E8 02004014 */  bnez       $v0, .L8001F4F4
    /* FCEC 8001F4EC 00000000 */   nop
    /* FCF0 8001F4F0 00F08424 */  addiu      $a0, $a0, -0x1000
  .L8001F4F4:
    /* FCF4 8001F4F4 02008104 */  bgez       $a0, .L8001F500
    /* FCF8 8001F4F8 21108000 */   addu      $v0, $a0, $zero
    /* FCFC 8001F4FC 07008224 */  addiu      $v0, $a0, 0x7
  .L8001F500:
    /* FD00 8001F500 36006426 */  addiu      $a0, $s3, 0x36
    /* FD04 8001F504 C3100200 */  sra        $v0, $v0, 3
    /* FD08 8001F508 21106200 */  addu       $v0, $v1, $v0
    /* FD0C 8001F50C 00340200 */  sll        $a2, $v0, 16
    /* FD10 8001F510 03340600 */  sra        $a2, $a2, 16
    /* FD14 8001F514 E6010586 */  lh         $a1, 0x1E6($s0)
    /* FD18 8001F518 21384002 */  addu       $a3, $s2, $zero
    /* FD1C 8001F51C DCBD000C */  jal        func_8002F770
    /* FD20 8001F520 E80102A6 */   sh        $v0, 0x1E8($s0)
    /* FD24 8001F524 36008426 */  addiu      $a0, $s4, 0x36
    /* FD28 8001F528 E6010586 */  lh         $a1, 0x1E6($s0)
    /* FD2C 8001F52C E8010686 */  lh         $a2, 0x1E8($s0)
    /* FD30 8001F530 DCBD000C */  jal        func_8002F770
    /* FD34 8001F534 21384002 */   addu      $a3, $s2, $zero
    /* FD38 8001F538 0C000386 */  lh         $v1, 0xC($s0)
    /* FD3C 8001F53C 1D000224 */  addiu      $v0, $zero, 0x1D
    /* FD40 8001F540 03006210 */  beq        $v1, $v0, .L8001F550
    /* FD44 8001F544 0E000224 */   addiu     $v0, $zero, 0xE
    /* FD48 8001F548 23006214 */  bne        $v1, $v0, .L8001F5D8
    /* FD4C 8001F54C 00000000 */   nop
  .L8001F550:
    /* FD50 8001F550 8C000286 */  lh         $v0, 0x8C($s0)
    /* FD54 8001F554 00000000 */  nop
    /* FD58 8001F558 1F004010 */  beqz       $v0, .L8001F5D8
    /* FD5C 8001F55C 00000000 */   nop
    /* FD60 8001F560 0000028E */  lw         $v0, 0x0($s0)
    /* FD64 8001F564 F800058E */  lw         $a1, 0xF8($s0)
    /* FD68 8001F568 F800428C */  lw         $v0, 0xF8($v0)
    /* FD6C 8001F56C 0A80043C */  lui        $a0, %hi(D_800A387C)
    /* FD70 8001F570 7C38848C */  lw         $a0, %lo(D_800A387C)($a0)
    /* FD74 8001F574 57FF010C */  jal        func_8007FD5C
    /* FD78 8001F578 23284500 */   subu      $a1, $v0, $a1
    /* FD7C 8001F57C 00FC4224 */  addiu      $v0, $v0, -0x400
    /* FD80 8001F580 FF0F4430 */  andi       $a0, $v0, 0xFFF
    /* FD84 8001F584 00088228 */  slti       $v0, $a0, 0x800
    /* FD88 8001F588 03004014 */  bnez       $v0, .L8001F598
    /* FD8C 8001F58C 00028228 */   slti      $v0, $a0, 0x200
    /* FD90 8001F590 00F08424 */  addiu      $a0, $a0, -0x1000
    /* FD94 8001F594 00028228 */  slti       $v0, $a0, 0x200
  .L8001F598:
    /* FD98 8001F598 03004014 */  bnez       $v0, .L8001F5A8
    /* FD9C 8001F59C 01FE8228 */   slti      $v0, $a0, -0x1FF
    /* FDA0 8001F5A0 6D7D0008 */  j          .L8001F5B4
    /* FDA4 8001F5A4 FF010424 */   addiu     $a0, $zero, 0x1FF
  .L8001F5A8:
    /* FDA8 8001F5A8 02004010 */  beqz       $v0, .L8001F5B4
    /* FDAC 8001F5AC 00000000 */   nop
    /* FDB0 8001F5B0 01FE0424 */  addiu      $a0, $zero, -0x1FF
  .L8001F5B4:
    /* FDB4 8001F5B4 EA0104A6 */  sh         $a0, 0x1EA($s0)
    /* FDB8 8001F5B8 7E006296 */  lhu        $v0, 0x7E($s3)
    /* FDBC 8001F5BC 00000000 */  nop
    /* FDC0 8001F5C0 21104400 */  addu       $v0, $v0, $a0
    /* FDC4 8001F5C4 7E0062A6 */  sh         $v0, 0x7E($s3)
    /* FDC8 8001F5C8 7E008296 */  lhu        $v0, 0x7E($s4)
    /* FDCC 8001F5CC 00000000 */  nop
    /* FDD0 8001F5D0 21104400 */  addu       $v0, $v0, $a0
    /* FDD4 8001F5D4 7E0082A6 */  sh         $v0, 0x7E($s4)
  .L8001F5D8:
    /* FDD8 8001F5D8 0E000296 */  lhu        $v0, 0xE($s0)
    /* FDDC 8001F5DC 00000000 */  nop
    /* FDE0 8001F5E0 FAFF4224 */  addiu      $v0, $v0, -0x6
    /* FDE4 8001F5E4 0200422C */  sltiu      $v0, $v0, 0x2
    /* FDE8 8001F5E8 67004010 */  beqz       $v0, .L8001F788
    /* FDEC 8001F5EC 02000224 */   addiu     $v0, $zero, 0x2
    /* FDF0 8001F5F0 6A000396 */  lhu        $v1, 0x6A($s0)
    /* FDF4 8001F5F4 00000000 */  nop
    /* FDF8 8001F5F8 63006214 */  bne        $v1, $v0, .L8001F788
    /* FDFC 8001F5FC 00000000 */   nop
    /* FE00 8001F600 6802028E */  lw         $v0, 0x268($s0)
    /* FE04 8001F604 00000000 */  nop
    /* FE08 8001F608 07004014 */  bnez       $v0, .L8001F628
    /* FE0C 8001F60C 00000000 */   nop
    /* FE10 8001F610 F400028E */  lw         $v0, 0xF4($s0)
    /* FE14 8001F614 F800038E */  lw         $v1, 0xF8($s0)
    /* FE18 8001F618 FC00048E */  lw         $a0, 0xFC($s0)
    /* FE1C 8001F61C 5C0202AE */  sw         $v0, 0x25C($s0)
    /* FE20 8001F620 600203AE */  sw         $v1, 0x260($s0)
    /* FE24 8001F624 640204AE */  sw         $a0, 0x264($s0)
  .L8001F628:
    /* FE28 8001F628 0000048E */  lw         $a0, 0x0($s0)
    /* FE2C 8001F62C 5C02028E */  lw         $v0, 0x25C($s0)
    /* FE30 8001F630 F400838C */  lw         $v1, 0xF4($a0)
    /* FE34 8001F634 00000000 */  nop
    /* FE38 8001F638 23186200 */  subu       $v1, $v1, $v0
    /* FE3C 8001F63C 18006300 */  mult       $v1, $v1
    /* FE40 8001F640 FC00838C */  lw         $v1, 0xFC($a0)
    /* FE44 8001F644 6402028E */  lw         $v0, 0x264($s0)
    /* FE48 8001F648 12280000 */  mflo       $a1
    /* FE4C 8001F64C 23186200 */  subu       $v1, $v1, $v0
    /* FE50 8001F650 00000000 */  nop
    /* FE54 8001F654 18006300 */  mult       $v1, $v1
    /* FE58 8001F658 12180000 */  mflo       $v1
    /* FE5C 8001F65C 2120A300 */  addu       $a0, $a1, $v1
    /* FE60 8001F660 0004822C */  sltiu      $v0, $a0, 0x400
    /* FE64 8001F664 06004010 */  beqz       $v0, .L8001F680
    /* FE68 8001F668 00000000 */   nop
    /* FE6C 8001F66C 0980013C */  lui        $at, %hi(D_8008D118)
    /* FE70 8001F670 21082400 */  addu       $at, $at, $a0
    /* FE74 8001F674 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* FE78 8001F678 B77D0008 */  j          .L8001F6DC
    /* FE7C 8001F67C C2200200 */   srl       $a0, $v0, 3
  .L8001F680:
    /* FE80 8001F680 09008004 */  bltz       $a0, .L8001F6A8
    /* FE84 8001F684 21180000 */   addu      $v1, $zero, $zero
    /* FE88 8001F688 21608000 */  addu       $t4, $a0, $zero
    /* FE8C 8001F68C 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* FE90 8001F690 00000000 */  nop
    /* FE94 8001F694 00000000 */  nop
    /* FE98 8001F698 1400A227 */  addiu      $v0, $sp, 0x14
    /* FE9C 8001F69C 21604000 */  addu       $t4, $v0, $zero
    /* FEA0 8001F6A0 00009FE9 */  swc2       $31, 0x0($t4)
    /* FEA4 8001F6A4 1400A38F */  lw         $v1, 0x14($sp)
  .L8001F6A8:
    /* FEA8 8001F6A8 FEFF0224 */  addiu      $v0, $zero, -0x2
    /* FEAC 8001F6AC 24106200 */  and        $v0, $v1, $v0
    /* FEB0 8001F6B0 16000324 */  addiu      $v1, $zero, 0x16
    /* FEB4 8001F6B4 23186200 */  subu       $v1, $v1, $v0
    /* FEB8 8001F6B8 06106400 */  srlv       $v0, $a0, $v1
    /* FEBC 8001F6BC 0980013C */  lui        $at, %hi(D_8008D118)
    /* FEC0 8001F6C0 21082200 */  addu       $at, $at, $v0
    /* FEC4 8001F6C4 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* FEC8 8001F6C8 42180300 */  srl        $v1, $v1, 1
    /* FECC 8001F6CC 13000224 */  addiu      $v0, $zero, 0x13
    /* FED0 8001F6D0 23104300 */  subu       $v0, $v0, $v1
    /* FED4 8001F6D4 00240400 */  sll        $a0, $a0, 16
    /* FED8 8001F6D8 06204400 */  srlv       $a0, $a0, $v0
  .L8001F6DC:
    /* FEDC 8001F6DC 0000028E */  lw         $v0, 0x0($s0)
    /* FEE0 8001F6E0 6002058E */  lw         $a1, 0x260($s0)
    /* FEE4 8001F6E4 F800428C */  lw         $v0, 0xF8($v0)
    /* FEE8 8001F6E8 57FF010C */  jal        func_8007FD5C
    /* FEEC 8001F6EC 23284500 */   subu      $a1, $v0, $a1
    /* FEF0 8001F6F0 00FC4224 */  addiu      $v0, $v0, -0x400
    /* FEF4 8001F6F4 FF0F4530 */  andi       $a1, $v0, 0xFFF
    /* FEF8 8001F6F8 0008A228 */  slti       $v0, $a1, 0x800
    /* FEFC 8001F6FC 03004014 */  bnez       $v0, .L8001F70C
    /* FF00 8001F700 0002A228 */   slti      $v0, $a1, 0x200
    /* FF04 8001F704 00F0A524 */  addiu      $a1, $a1, -0x1000
    /* FF08 8001F708 0002A228 */  slti       $v0, $a1, 0x200
  .L8001F70C:
    /* FF0C 8001F70C 03004014 */  bnez       $v0, .L8001F71C
    /* FF10 8001F710 01FEA228 */   slti      $v0, $a1, -0x1FF
    /* FF14 8001F714 CA7D0008 */  j          .L8001F728
    /* FF18 8001F718 FF010524 */   addiu     $a1, $zero, 0x1FF
  .L8001F71C:
    /* FF1C 8001F71C 02004010 */  beqz       $v0, .L8001F728
    /* FF20 8001F720 00000000 */   nop
    /* FF24 8001F724 01FE0524 */  addiu      $a1, $zero, -0x1FF
  .L8001F728:
    /* FF28 8001F728 EA010286 */  lh         $v0, 0x1EA($s0)
    /* FF2C 8001F72C 00000000 */  nop
    /* FF30 8001F730 2318A200 */  subu       $v1, $a1, $v0
    /* FF34 8001F734 FF0F6430 */  andi       $a0, $v1, 0xFFF
    /* FF38 8001F738 21284000 */  addu       $a1, $v0, $zero
    /* FF3C 8001F73C 00088228 */  slti       $v0, $a0, 0x800
    /* FF40 8001F740 02004014 */  bnez       $v0, .L8001F74C
    /* FF44 8001F744 00000000 */   nop
    /* FF48 8001F748 00F08424 */  addiu      $a0, $a0, -0x1000
  .L8001F74C:
    /* FF4C 8001F74C 02008104 */  bgez       $a0, .L8001F758
    /* FF50 8001F750 21188000 */   addu      $v1, $a0, $zero
    /* FF54 8001F754 07008324 */  addiu      $v1, $a0, 0x7
  .L8001F758:
    /* FF58 8001F758 C3180300 */  sra        $v1, $v1, 3
    /* FF5C 8001F75C 2118A300 */  addu       $v1, $a1, $v1
    /* FF60 8001F760 EA0103A6 */  sh         $v1, 0x1EA($s0)
    /* FF64 8001F764 72006296 */  lhu        $v0, 0x72($s3)
    /* FF68 8001F768 00000000 */  nop
    /* FF6C 8001F76C 21104300 */  addu       $v0, $v0, $v1
    /* FF70 8001F770 720062A6 */  sh         $v0, 0x72($s3)
    /* FF74 8001F774 72008296 */  lhu        $v0, 0x72($s4)
    /* FF78 8001F778 EA010396 */  lhu        $v1, 0x1EA($s0)
    /* FF7C 8001F77C 00000000 */  nop
    /* FF80 8001F780 21104300 */  addu       $v0, $v0, $v1
    /* FF84 8001F784 720082A6 */  sh         $v0, 0x72($s4)
  .L8001F788:
    /* FF88 8001F788 6E020286 */  lh         $v0, 0x26E($s0)
    /* FF8C 8001F78C 00000000 */  nop
    /* FF90 8001F790 2A004010 */  beqz       $v0, .L8001F83C
    /* FF94 8001F794 00000000 */   nop
    /* FF98 8001F798 96000286 */  lh         $v0, 0x96($s0)
    /* FF9C 8001F79C 00000000 */  nop
    /* FFA0 8001F7A0 26004014 */  bnez       $v0, .L8001F83C
    /* FFA4 8001F7A4 00000000 */   nop
    /* FFA8 8001F7A8 6E5C000C */  jal        rng_Next
    /* FFAC 8001F7AC 00000000 */   nop
    /* FFB0 8001F7B0 3F004230 */  andi       $v0, $v0, 0x3F
    /* FFB4 8001F7B4 0C006396 */  lhu        $v1, 0xC($s3)
    /* FFB8 8001F7B8 E0FF4424 */  addiu      $a0, $v0, -0x20
    /* FFBC 8001F7BC 21186400 */  addu       $v1, $v1, $a0
    /* FFC0 8001F7C0 0C0063A6 */  sh         $v1, 0xC($s3)
    /* FFC4 8001F7C4 0C008296 */  lhu        $v0, 0xC($s4)
    /* FFC8 8001F7C8 00000000 */  nop
    /* FFCC 8001F7CC 21104400 */  addu       $v0, $v0, $a0
    /* FFD0 8001F7D0 0C0082A6 */  sh         $v0, 0xC($s4)
    /* FFD4 8001F7D4 14006296 */  lhu        $v0, 0x14($s3)
    /* FFD8 8001F7D8 00000000 */  nop
    /* FFDC 8001F7DC 23104400 */  subu       $v0, $v0, $a0
    /* FFE0 8001F7E0 140062A6 */  sh         $v0, 0x14($s3)
    /* FFE4 8001F7E4 14008296 */  lhu        $v0, 0x14($s4)
    /* FFE8 8001F7E8 00000000 */  nop
    /* FFEC 8001F7EC 23104400 */  subu       $v0, $v0, $a0
    /* FFF0 8001F7F0 6E5C000C */  jal        rng_Next
    /* FFF4 8001F7F4 140082A6 */   sh        $v0, 0x14($s4)
    /* FFF8 8001F7F8 3F004230 */  andi       $v0, $v0, 0x3F
    /* FFFC 8001F7FC 1E006396 */  lhu        $v1, 0x1E($s3)
    /* 10000 8001F800 E0FF4424 */  addiu      $a0, $v0, -0x20
    /* 10004 8001F804 21186400 */  addu       $v1, $v1, $a0
    /* 10008 8001F808 1E0063A6 */  sh         $v1, 0x1E($s3)
    /* 1000C 8001F80C 1E008296 */  lhu        $v0, 0x1E($s4)
    /* 10010 8001F810 00000000 */  nop
    /* 10014 8001F814 21104400 */  addu       $v0, $v0, $a0
    /* 10018 8001F818 1E0082A6 */  sh         $v0, 0x1E($s4)
    /* 1001C 8001F81C 26006296 */  lhu        $v0, 0x26($s3)
    /* 10020 8001F820 00000000 */  nop
    /* 10024 8001F824 23104400 */  subu       $v0, $v0, $a0
    /* 10028 8001F828 260062A6 */  sh         $v0, 0x26($s3)
    /* 1002C 8001F82C 26008296 */  lhu        $v0, 0x26($s4)
    /* 10030 8001F830 00000000 */  nop
    /* 10034 8001F834 23104400 */  subu       $v0, $v0, $a0
    /* 10038 8001F838 260082A6 */  sh         $v0, 0x26($s4)
  .L8001F83C:
    /* 1003C 8001F83C 4400BF8F */  lw         $ra, 0x44($sp)
    /* 10040 8001F840 4000B48F */  lw         $s4, 0x40($sp)
    /* 10044 8001F844 3C00B38F */  lw         $s3, 0x3C($sp)
    /* 10048 8001F848 3800B28F */  lw         $s2, 0x38($sp)
    /* 1004C 8001F84C 3400B18F */  lw         $s1, 0x34($sp)
    /* 10050 8001F850 3000B08F */  lw         $s0, 0x30($sp)
    /* 10054 8001F854 4800BD27 */  addiu      $sp, $sp, 0x48
    /* 10058 8001F858 0800E003 */  jr         $ra
    /* 1005C 8001F85C 00000000 */   nop
endlabel md_game_rob_data_init
