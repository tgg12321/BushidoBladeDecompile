glabel func_8002D320
    /* 1DB20 8002D320 F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 1DB24 8002D324 1800A88F */  lw         $t0, 0x18($sp)
    /* 1DB28 8002D328 1F008014 */  bnez       $a0, .L8002D3A8
    /* 1DB2C 8002D32C 00000000 */   nop
    /* 1DB30 8002D330 6000A38C */  lw         $v1, 0x60($a1)
    /* 1DB34 8002D334 0000C28C */  lw         $v0, 0x0($a2)
    /* 1DB38 8002D338 0000638C */  lw         $v1, 0x0($v1)
    /* 1DB3C 8002D33C 00000000 */  nop
    /* 1DB40 8002D340 23104300 */  subu       $v0, $v0, $v1
    /* 1DB44 8002D344 6000A38C */  lw         $v1, 0x60($a1)
    /* 1DB48 8002D348 F800A2A4 */  sh         $v0, 0xF8($a1)
    /* 1DB4C 8002D34C 0400C28C */  lw         $v0, 0x4($a2)
    /* 1DB50 8002D350 0400638C */  lw         $v1, 0x4($v1)
    /* 1DB54 8002D354 00000000 */  nop
    /* 1DB58 8002D358 23104300 */  subu       $v0, $v0, $v1
    /* 1DB5C 8002D35C 6000A38C */  lw         $v1, 0x60($a1)
    /* 1DB60 8002D360 FA00A2A4 */  sh         $v0, 0xFA($a1)
    /* 1DB64 8002D364 0800C28C */  lw         $v0, 0x8($a2)
    /* 1DB68 8002D368 0800638C */  lw         $v1, 0x8($v1)
    /* 1DB6C 8002D36C 00000000 */  nop
    /* 1DB70 8002D370 23104300 */  subu       $v0, $v0, $v1
    /* 1DB74 8002D374 FC00A2A4 */  sh         $v0, 0xFC($a1)
    /* 1DB78 8002D378 F800A224 */  addiu      $v0, $a1, 0xF8
    /* 1DB7C 8002D37C 21604000 */  addu       $t4, $v0, $zero
    /* 1DB80 8002D380 000080C9 */  lwc2       $0, 0x0($t4)
    /* 1DB84 8002D384 040081C9 */  lwc2       $1, 0x4($t4)
    /* 1DB88 8002D388 00000000 */  nop
    /* 1DB8C 8002D38C 00000000 */  nop
    /* 1DB90 8002D390 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1DB94 8002D394 0001A224 */  addiu      $v0, $a1, 0x100
    /* 1DB98 8002D398 21604000 */  addu       $t4, $v0, $zero
    /* 1DB9C 8002D39C 000099E9 */  swc2       $25, 0x0($t4)
    /* 1DBA0 8002D3A0 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1DBA4 8002D3A4 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
  .L8002D3A8:
    /* 1DBA8 8002D3A8 0001A48C */  lw         $a0, 0x100($a1)
    /* 1DBAC 8002D3AC 23300700 */  negu       $a2, $a3
    /* 1DBB0 8002D3B0 2A108600 */  slt        $v0, $a0, $a2
    /* 1DBB4 8002D3B4 55004014 */  bnez       $v0, .L8002D50C
    /* 1DBB8 8002D3B8 21100000 */   addu      $v0, $zero, $zero
    /* 1DBBC 8002D3BC 2A10E400 */  slt        $v0, $a3, $a0
    /* 1DBC0 8002D3C0 52004014 */  bnez       $v0, .L8002D50C
    /* 1DBC4 8002D3C4 21100000 */   addu      $v0, $zero, $zero
    /* 1DBC8 8002D3C8 0401A38C */  lw         $v1, 0x104($a1)
    /* 1DBCC 8002D3CC 00000000 */  nop
    /* 1DBD0 8002D3D0 2A106600 */  slt        $v0, $v1, $a2
    /* 1DBD4 8002D3D4 4D004014 */  bnez       $v0, .L8002D50C
    /* 1DBD8 8002D3D8 21100000 */   addu      $v0, $zero, $zero
    /* 1DBDC 8002D3DC 2A10E300 */  slt        $v0, $a3, $v1
    /* 1DBE0 8002D3E0 4A004014 */  bnez       $v0, .L8002D50C
    /* 1DBE4 8002D3E4 21100000 */   addu      $v0, $zero, $zero
    /* 1DBE8 8002D3E8 18008400 */  mult       $a0, $a0
    /* 1DBEC 8002D3EC 12100000 */  mflo       $v0
    /* 1DBF0 8002D3F0 00000000 */  nop
    /* 1DBF4 8002D3F4 00000000 */  nop
    /* 1DBF8 8002D3F8 18006300 */  mult       $v1, $v1
    /* 1DBFC 8002D3FC 12180000 */  mflo       $v1
    /* 1DC00 8002D400 21204300 */  addu       $a0, $v0, $v1
    /* 1DC04 8002D404 2A100401 */  slt        $v0, $t0, $a0
    /* 1DC08 8002D408 40004014 */  bnez       $v0, .L8002D50C
    /* 1DC0C 8002D40C 21100000 */   addu      $v0, $zero, $zero
    /* 1DC10 8002D410 23200401 */  subu       $a0, $t0, $a0
    /* 1DC14 8002D414 0004822C */  sltiu      $v0, $a0, 0x400
    /* 1DC18 8002D418 06004010 */  beqz       $v0, .L8002D434
    /* 1DC1C 8002D41C 00000000 */   nop
    /* 1DC20 8002D420 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1DC24 8002D424 21082400 */  addu       $at, $at, $a0
    /* 1DC28 8002D428 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1DC2C 8002D42C 23B50008 */  j          .L8002D48C
    /* 1DC30 8002D430 C2200200 */   srl       $a0, $v0, 3
  .L8002D434:
    /* 1DC34 8002D434 08008004 */  bltz       $a0, .L8002D458
    /* 1DC38 8002D438 21180000 */   addu      $v1, $zero, $zero
    /* 1DC3C 8002D43C 21608000 */  addu       $t4, $a0, $zero
    /* 1DC40 8002D440 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1DC44 8002D444 00000000 */  nop
    /* 1DC48 8002D448 00000000 */  nop
    /* 1DC4C 8002D44C 2160A003 */  addu       $t4, $sp, $zero
    /* 1DC50 8002D450 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1DC54 8002D454 0000A38F */  lw         $v1, 0x0($sp)
  .L8002D458:
    /* 1DC58 8002D458 FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1DC5C 8002D45C 24106200 */  and        $v0, $v1, $v0
    /* 1DC60 8002D460 16000324 */  addiu      $v1, $zero, 0x16
    /* 1DC64 8002D464 23186200 */  subu       $v1, $v1, $v0
    /* 1DC68 8002D468 06106400 */  srlv       $v0, $a0, $v1
    /* 1DC6C 8002D46C 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1DC70 8002D470 21082200 */  addu       $at, $at, $v0
    /* 1DC74 8002D474 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1DC78 8002D478 42180300 */  srl        $v1, $v1, 1
    /* 1DC7C 8002D47C 13000224 */  addiu      $v0, $zero, 0x13
    /* 1DC80 8002D480 23104300 */  subu       $v0, $v0, $v1
    /* 1DC84 8002D484 00240400 */  sll        $a0, $a0, 16
    /* 1DC88 8002D488 06204400 */  srlv       $a0, $a0, $v0
  .L8002D48C:
    /* 1DC8C 8002D48C 21380000 */  addu       $a3, $zero, $zero
    /* 1DC90 8002D490 B000A38C */  lw         $v1, 0xB0($a1)
    /* 1DC94 8002D494 00000000 */  nop
    /* 1DC98 8002D498 03006104 */  bgez       $v1, .L8002D4A8
    /* 1DC9C 8002D49C 21300000 */   addu      $a2, $zero, $zero
    /* 1DCA0 8002D4A0 2EB50008 */  j          .L8002D4B8
    /* 1DCA4 8002D4A4 21306000 */   addu      $a2, $v1, $zero
  .L8002D4A8:
    /* 1DCA8 8002D4A8 2A10C300 */  slt        $v0, $a2, $v1
    /* 1DCAC 8002D4AC 02004010 */  beqz       $v0, .L8002D4B8
    /* 1DCB0 8002D4B0 00000000 */   nop
    /* 1DCB4 8002D4B4 21386000 */  addu       $a3, $v1, $zero
  .L8002D4B8:
    /* 1DCB8 8002D4B8 C000A38C */  lw         $v1, 0xC0($a1)
    /* 1DCBC 8002D4BC 00000000 */  nop
    /* 1DCC0 8002D4C0 2A106600 */  slt        $v0, $v1, $a2
    /* 1DCC4 8002D4C4 03004010 */  beqz       $v0, .L8002D4D4
    /* 1DCC8 8002D4C8 2A10E300 */   slt       $v0, $a3, $v1
    /* 1DCCC 8002D4CC 38B50008 */  j          .L8002D4E0
    /* 1DCD0 8002D4D0 21306000 */   addu      $a2, $v1, $zero
  .L8002D4D4:
    /* 1DCD4 8002D4D4 02004010 */  beqz       $v0, .L8002D4E0
    /* 1DCD8 8002D4D8 00000000 */   nop
    /* 1DCDC 8002D4DC 21386000 */  addu       $a3, $v1, $zero
  .L8002D4E0:
    /* 1DCE0 8002D4E0 0801A58C */  lw         $a1, 0x108($a1)
    /* 1DCE4 8002D4E4 00000000 */  nop
    /* 1DCE8 8002D4E8 2310A400 */  subu       $v0, $a1, $a0
    /* 1DCEC 8002D4EC 2A10E200 */  slt        $v0, $a3, $v0
    /* 1DCF0 8002D4F0 06004014 */  bnez       $v0, .L8002D50C
    /* 1DCF4 8002D4F4 21100000 */   addu      $v0, $zero, $zero
    /* 1DCF8 8002D4F8 2110A400 */  addu       $v0, $a1, $a0
    /* 1DCFC 8002D4FC 2A104600 */  slt        $v0, $v0, $a2
    /* 1DD00 8002D500 02004014 */  bnez       $v0, .L8002D50C
    /* 1DD04 8002D504 21100000 */   addu      $v0, $zero, $zero
    /* 1DD08 8002D508 01000224 */  addiu      $v0, $zero, 0x1
  .L8002D50C:
    /* 1DD0C 8002D50C 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 1DD10 8002D510 0800E003 */  jr         $ra
    /* 1DD14 8002D514 00000000 */   nop
endlabel func_8002D320
