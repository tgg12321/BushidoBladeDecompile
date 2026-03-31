glabel func_8002CA8C
    /* 1D28C 8002CA8C A0FFBD27 */  addiu      $sp, $sp, -0x60
    /* 1D290 8002CA90 3C00B1AF */  sw         $s1, 0x3C($sp)
    /* 1D294 8002CA94 801F113C */  lui        $s1, (0x1F8002B8 >> 16)
    /* 1D298 8002CA98 B8023136 */  ori        $s1, $s1, (0x1F8002B8 & 0xFFFF)
    /* 1D29C 8002CA9C 5000B6AF */  sw         $s6, 0x50($sp)
    /* 1D2A0 8002CAA0 21B00000 */  addu       $s6, $zero, $zero
    /* 1D2A4 8002CAA4 4400B3AF */  sw         $s3, 0x44($sp)
    /* 1D2A8 8002CAA8 21980000 */  addu       $s3, $zero, $zero
    /* 1D2AC 8002CAAC 5400B7AF */  sw         $s7, 0x54($sp)
    /* 1D2B0 8002CAB0 21B80000 */  addu       $s7, $zero, $zero
    /* 1D2B4 8002CAB4 5C00BFAF */  sw         $ra, 0x5C($sp)
    /* 1D2B8 8002CAB8 5800BEAF */  sw         $fp, 0x58($sp)
    /* 1D2BC 8002CABC 4C00B5AF */  sw         $s5, 0x4C($sp)
    /* 1D2C0 8002CAC0 4800B4AF */  sw         $s4, 0x48($sp)
    /* 1D2C4 8002CAC4 4000B2AF */  sw         $s2, 0x40($sp)
    /* 1D2C8 8002CAC8 3800B0AF */  sw         $s0, 0x38($sp)
    /* 1D2CC 8002CACC 1800A4AF */  sw         $a0, 0x18($sp)
    /* 1D2D0 8002CAD0 2000A5AF */  sw         $a1, 0x20($sp)
    /* 1D2D4 8002CAD4 2800A6AF */  sw         $a2, 0x28($sp)
    /* 1D2D8 8002CAD8 3000A0AF */  sw         $zero, 0x30($sp)
    /* 1D2DC 8002CADC 04008484 */  lh         $a0, 0x4($a0)
    /* 1D2E0 8002CAE0 0F80033C */  lui        $v1, %hi(D_800F5F68)
    /* 1D2E4 8002CAE4 685F6324 */  addiu      $v1, $v1, %lo(D_800F5F68)
    /* 1D2E8 8002CAE8 C0100400 */  sll        $v0, $a0, 3
    /* 1D2EC 8002CAEC 23104400 */  subu       $v0, $v0, $a0
    /* 1D2F0 8002CAF0 C0100200 */  sll        $v0, $v0, 3
    /* 1D2F4 8002CAF4 23104400 */  subu       $v0, $v0, $a0
    /* 1D2F8 8002CAF8 C0100200 */  sll        $v0, $v0, 3
    /* 1D2FC 8002CAFC 21A84300 */  addu       $s5, $v0, $v1
    /* 1D300 8002CB00 1200B226 */  addiu      $s2, $s5, 0x12
    /* 1D304 8002CB04 40110400 */  sll        $v0, $a0, 5
    /* 1D308 8002CB08 21104400 */  addu       $v0, $v0, $a0
    /* 1D30C 8002CB0C C0F00200 */  sll        $fp, $v0, 3
    /* 1D310 8002CB10 21A0C003 */  addu       $s4, $fp, $zero
  .L8002CB14:
    /* 1D314 8002CB14 1800A88F */  lw         $t0, 0x18($sp)
    /* 1D318 8002CB18 00000000 */  nop
    /* 1D31C 8002CB1C 6C020285 */  lh         $v0, 0x26C($t0)
    /* 1D320 8002CB20 00000000 */  nop
    /* 1D324 8002CB24 04004014 */  bnez       $v0, .L8002CB38
    /* 1D328 8002CB28 FAFF6226 */   addiu     $v0, $s3, -0x6
    /* 1D32C 8002CB2C 0400422C */  sltiu      $v0, $v0, 0x4
    /* 1D330 8002CB30 71004014 */  bnez       $v0, .L8002CCF8
    /* 1D334 8002CB34 00000000 */   nop
  .L8002CB38:
    /* 1D338 8002CB38 FAFF4796 */  lhu        $a3, -0x6($s2)
    /* 1D33C 8002CB3C 801F013C */  lui        $at, (0x1F8000A8 >> 16)
    /* 1D340 8002CB40 21088102 */  addu       $at, $s4, $at
    /* 1D344 8002CB44 A800248C */  lw         $a0, (0x1F8000A8 & 0xFFFF)($at)
    /* 1D348 8002CB48 8400228E */  lw         $v0, 0x84($s1)
    /* 1D34C 8002CB4C 23188700 */  subu       $v1, $a0, $a3
    /* 1D350 8002CB50 2A104300 */  slt        $v0, $v0, $v1
    /* 1D354 8002CB54 20004014 */  bnez       $v0, .L8002CBD8
    /* 1D358 8002CB58 21800000 */   addu      $s0, $zero, $zero
    /* 1D35C 8002CB5C 7800238E */  lw         $v1, 0x78($s1)
    /* 1D360 8002CB60 21108700 */  addu       $v0, $a0, $a3
    /* 1D364 8002CB64 2A104300 */  slt        $v0, $v0, $v1
    /* 1D368 8002CB68 1B004014 */  bnez       $v0, .L8002CBD8
    /* 1D36C 8002CB6C 00000000 */   nop
    /* 1D370 8002CB70 801F013C */  lui        $at, (0x1F8000AC >> 16)
    /* 1D374 8002CB74 21088102 */  addu       $at, $s4, $at
    /* 1D378 8002CB78 AC00248C */  lw         $a0, (0x1F8000AC & 0xFFFF)($at)
    /* 1D37C 8002CB7C 8800228E */  lw         $v0, 0x88($s1)
    /* 1D380 8002CB80 23188700 */  subu       $v1, $a0, $a3
    /* 1D384 8002CB84 2A104300 */  slt        $v0, $v0, $v1
    /* 1D388 8002CB88 13004014 */  bnez       $v0, .L8002CBD8
    /* 1D38C 8002CB8C 21108700 */   addu      $v0, $a0, $a3
    /* 1D390 8002CB90 7C00238E */  lw         $v1, 0x7C($s1)
    /* 1D394 8002CB94 00000000 */  nop
    /* 1D398 8002CB98 2A104300 */  slt        $v0, $v0, $v1
    /* 1D39C 8002CB9C 0E004014 */  bnez       $v0, .L8002CBD8
    /* 1D3A0 8002CBA0 00000000 */   nop
    /* 1D3A4 8002CBA4 801F013C */  lui        $at, (0x1F8000B0 >> 16)
    /* 1D3A8 8002CBA8 21088102 */  addu       $at, $s4, $at
    /* 1D3AC 8002CBAC B000248C */  lw         $a0, (0x1F8000B0 & 0xFFFF)($at)
    /* 1D3B0 8002CBB0 8C00228E */  lw         $v0, 0x8C($s1)
    /* 1D3B4 8002CBB4 23188700 */  subu       $v1, $a0, $a3
    /* 1D3B8 8002CBB8 2A104300 */  slt        $v0, $v0, $v1
    /* 1D3BC 8002CBBC 06004014 */  bnez       $v0, .L8002CBD8
    /* 1D3C0 8002CBC0 21108700 */   addu      $v0, $a0, $a3
    /* 1D3C4 8002CBC4 8000238E */  lw         $v1, 0x80($s1)
    /* 1D3C8 8002CBC8 00000000 */  nop
    /* 1D3CC 8002CBCC 2A104300 */  slt        $v0, $v0, $v1
    /* 1D3D0 8002CBD0 02004010 */  beqz       $v0, .L8002CBDC
    /* 1D3D4 8002CBD4 00000000 */   nop
  .L8002CBD8:
    /* 1D3D8 8002CBD8 01001024 */  addiu      $s0, $zero, 0x1
  .L8002CBDC:
    /* 1D3DC 8002CBDC 46000016 */  bnez       $s0, .L8002CCF8
    /* 1D3E0 8002CBE0 00000000 */   nop
    /* 1D3E4 8002CBE4 2000A88F */  lw         $t0, 0x20($sp)
    /* 1D3E8 8002CBE8 00000000 */  nop
    /* 1D3EC 8002CBEC 1C000011 */  beqz       $t0, .L8002CC60
    /* 1D3F0 8002CBF0 801F063C */   lui       $a2, (0x1F8000A8 >> 16)
    /* 1D3F4 8002CBF4 A800C634 */  ori        $a2, $a2, (0x1F8000A8 & 0xFFFF)
    /* 1D3F8 8002CBF8 21200000 */  addu       $a0, $zero, $zero
    /* 1D3FC 8002CBFC 21282002 */  addu       $a1, $s1, $zero
    /* 1D400 8002CC00 2130E602 */  addu       $a2, $s7, $a2
    /* 1D404 8002CC04 FCFF4296 */  lhu        $v0, -0x4($s2)
    /* 1D408 8002CC08 2130C603 */  addu       $a2, $fp, $a2
    /* 1D40C 8002CC0C E0B5000C */  jal        saTan0KiWareMoveA
    /* 1D410 8002CC10 1000A2AF */   sw        $v0, 0x10($sp)
    /* 1D414 8002CC14 21804000 */  addu       $s0, $v0, $zero
    /* 1D418 8002CC18 37000012 */  beqz       $s0, .L8002CCF8
    /* 1D41C 8002CC1C 00000000 */   nop
    /* 1D420 8002CC20 0000A286 */  lh         $v0, 0x0($s5)
    /* 1D424 8002CC24 00000000 */  nop
    /* 1D428 8002CC28 2B004010 */  beqz       $v0, .L8002CCD8
    /* 1D42C 8002CC2C 00000000 */   nop
    /* 1D430 8002CC30 2800A88F */  lw         $t0, 0x28($sp)
    /* 1D434 8002CC34 00000000 */  nop
    /* 1D438 8002CC38 29000011 */  beqz       $t0, .L8002CCE0
    /* 1D43C 8002CC3C 01000424 */   addiu     $a0, $zero, 0x1
    /* 1D440 8002CC40 21282002 */  addu       $a1, $s1, $zero
    /* 1D444 8002CC44 FEFF4796 */  lhu        $a3, -0x2($s2)
    /* 1D448 8002CC48 00004296 */  lhu        $v0, 0x0($s2)
    /* 1D44C 8002CC4C 21300000 */  addu       $a2, $zero, $zero
    /* 1D450 8002CC50 E0B5000C */  jal        saTan0KiWareMoveA
    /* 1D454 8002CC54 1000A2AF */   sw        $v0, 0x10($sp)
    /* 1D458 8002CC58 31B30008 */  j          .L8002CCC4
    /* 1D45C 8002CC5C 00000000 */   nop
  .L8002CC60:
    /* 1D460 8002CC60 A800C634 */  ori        $a2, $a2, (0x1F8000A8 & 0xFFFF)
    /* 1D464 8002CC64 21200000 */  addu       $a0, $zero, $zero
    /* 1D468 8002CC68 21282002 */  addu       $a1, $s1, $zero
    /* 1D46C 8002CC6C 2130E602 */  addu       $a2, $s7, $a2
    /* 1D470 8002CC70 FCFF4296 */  lhu        $v0, -0x4($s2)
    /* 1D474 8002CC74 2130C603 */  addu       $a2, $fp, $a2
    /* 1D478 8002CC78 C8B4000C */  jal        func_8002D320
    /* 1D47C 8002CC7C 1000A2AF */   sw        $v0, 0x10($sp)
    /* 1D480 8002CC80 21804000 */  addu       $s0, $v0, $zero
    /* 1D484 8002CC84 1C000012 */  beqz       $s0, .L8002CCF8
    /* 1D488 8002CC88 00000000 */   nop
    /* 1D48C 8002CC8C 0000A286 */  lh         $v0, 0x0($s5)
    /* 1D490 8002CC90 00000000 */  nop
    /* 1D494 8002CC94 10004010 */  beqz       $v0, .L8002CCD8
    /* 1D498 8002CC98 00000000 */   nop
    /* 1D49C 8002CC9C 2800A88F */  lw         $t0, 0x28($sp)
    /* 1D4A0 8002CCA0 00000000 */  nop
    /* 1D4A4 8002CCA4 0E000011 */  beqz       $t0, .L8002CCE0
    /* 1D4A8 8002CCA8 01000424 */   addiu     $a0, $zero, 0x1
    /* 1D4AC 8002CCAC 21282002 */  addu       $a1, $s1, $zero
    /* 1D4B0 8002CCB0 FEFF4796 */  lhu        $a3, -0x2($s2)
    /* 1D4B4 8002CCB4 00004296 */  lhu        $v0, 0x0($s2)
    /* 1D4B8 8002CCB8 21300000 */  addu       $a2, $zero, $zero
    /* 1D4BC 8002CCBC C8B4000C */  jal        func_8002D320
    /* 1D4C0 8002CCC0 1000A2AF */   sw        $v0, 0x10($sp)
  .L8002CCC4:
    /* 1D4C4 8002CCC4 04004010 */  beqz       $v0, .L8002CCD8
    /* 1D4C8 8002CCC8 00000000 */   nop
    /* 1D4CC 8002CCCC 01000824 */  addiu      $t0, $zero, 0x1
    /* 1D4D0 8002CCD0 04106802 */  sllv       $v0, $t0, $s3
    /* 1D4D4 8002CCD4 25B0C202 */  or         $s6, $s6, $v0
  .L8002CCD8:
    /* 1D4D8 8002CCD8 07000012 */  beqz       $s0, .L8002CCF8
    /* 1D4DC 8002CCDC 00000000 */   nop
  .L8002CCE0:
    /* 1D4E0 8002CCE0 01000824 */  addiu      $t0, $zero, 0x1
    /* 1D4E4 8002CCE4 04106802 */  sllv       $v0, $t0, $s3
    /* 1D4E8 8002CCE8 3000A88F */  lw         $t0, 0x30($sp)
    /* 1D4EC 8002CCEC 00000000 */  nop
    /* 1D4F0 8002CCF0 25400201 */  or         $t0, $t0, $v0
    /* 1D4F4 8002CCF4 3000A8AF */  sw         $t0, 0x30($sp)
  .L8002CCF8:
    /* 1D4F8 8002CCF8 0C00F726 */  addiu      $s7, $s7, 0xC
    /* 1D4FC 8002CCFC 0C009426 */  addiu      $s4, $s4, 0xC
    /* 1D500 8002CD00 01007326 */  addiu      $s3, $s3, 0x1
    /* 1D504 8002CD04 14005226 */  addiu      $s2, $s2, 0x14
    /* 1D508 8002CD08 1600622A */  slti       $v0, $s3, 0x16
    /* 1D50C 8002CD0C 81FF4014 */  bnez       $v0, .L8002CB14
    /* 1D510 8002CD10 1400B526 */   addiu     $s5, $s5, 0x14
    /* 1D514 8002CD14 3000A88F */  lw         $t0, 0x30($sp)
    /* 1D518 8002CD18 00000000 */  nop
    /* 1D51C 8002CD1C B40028AE */  sw         $t0, 0xB4($s1)
    /* 1D520 8002CD20 C40036AE */  sw         $s6, 0xC4($s1)
    /* 1D524 8002CD24 5C00BF8F */  lw         $ra, 0x5C($sp)
    /* 1D528 8002CD28 5800BE8F */  lw         $fp, 0x58($sp)
    /* 1D52C 8002CD2C 5400B78F */  lw         $s7, 0x54($sp)
    /* 1D530 8002CD30 5000B68F */  lw         $s6, 0x50($sp)
    /* 1D534 8002CD34 4C00B58F */  lw         $s5, 0x4C($sp)
    /* 1D538 8002CD38 4800B48F */  lw         $s4, 0x48($sp)
    /* 1D53C 8002CD3C 4400B38F */  lw         $s3, 0x44($sp)
    /* 1D540 8002CD40 4000B28F */  lw         $s2, 0x40($sp)
    /* 1D544 8002CD44 3C00B18F */  lw         $s1, 0x3C($sp)
    /* 1D548 8002CD48 3800B08F */  lw         $s0, 0x38($sp)
    /* 1D54C 8002CD4C 6000BD27 */  addiu      $sp, $sp, 0x60
    /* 1D550 8002CD50 0800E003 */  jr         $ra
    /* 1D554 8002CD54 00000000 */   nop
endlabel func_8002CA8C
