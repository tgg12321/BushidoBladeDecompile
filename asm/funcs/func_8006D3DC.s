glabel func_8006D3DC
    /* 5DBDC 8006D3DC 90FFBD27 */  addiu      $sp, $sp, -0x70
    /* 5DBE0 8006D3E0 5000B0AF */  sw         $s0, 0x50($sp)
    /* 5DBE4 8006D3E4 21808000 */  addu       $s0, $a0, $zero
    /* 5DBE8 8006D3E8 6400B5AF */  sw         $s5, 0x64($sp)
    /* 5DBEC 8006D3EC 21A80000 */  addu       $s5, $zero, $zero
    /* 5DBF0 8006D3F0 5400B1AF */  sw         $s1, 0x54($sp)
    /* 5DBF4 8006D3F4 21880000 */  addu       $s1, $zero, $zero
    /* 5DBF8 8006D3F8 0A000224 */  addiu      $v0, $zero, 0xA
    /* 5DBFC 8006D3FC 5800B2AF */  sw         $s2, 0x58($sp)
    /* 5DC00 8006D400 40001224 */  addiu      $s2, $zero, 0x40
    /* 5DC04 8006D404 6800BFAF */  sw         $ra, 0x68($sp)
    /* 5DC08 8006D408 6000B4AF */  sw         $s4, 0x60($sp)
    /* 5DC0C 8006D40C 5C00B3AF */  sw         $s3, 0x5C($sp)
    /* 5DC10 8006D410 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5DC14 8006D414 0400028E */  lw         $v0, 0x4($s0)
    /* 5DC18 8006D418 01001324 */  addiu      $s3, $zero, 0x1
    /* 5DC1C 8006D41C 3800548C */  lw         $s4, 0x38($v0)
    /* 5DC20 8006D420 01000224 */  addiu      $v0, $zero, 0x1
    /* 5DC24 8006D424 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5DC28 8006D428 4000A2A3 */  sb         $v0, 0x40($sp)
  .L8006D42C:
    /* 5DC2C 8006D42C 00141100 */  sll        $v0, $s1, 16
    /* 5DC30 8006D430 031C0200 */  sra        $v1, $v0, 16
    /* 5DC34 8006D434 05006014 */  bnez       $v1, .L8006D44C
    /* 5DC38 8006D438 4000B3A3 */   sb        $s3, 0x40($sp)
    /* 5DC3C 8006D43C 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5DC40 8006D440 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5DC44 8006D444 3CB50108 */  j          .L8006D4F0
    /* 5DC48 8006D448 2800A0AF */   sw        $zero, 0x28($sp)
  .L8006D44C:
    /* 5DC4C 8006D44C 5C048487 */  lh         $a0, %gp_rel(D_800A3528)($gp)
    /* 5DC50 8006D450 00000000 */  nop
    /* 5DC54 8006D454 01008224 */  addiu      $v0, $a0, 0x1
    /* 5DC58 8006D458 11006214 */  bne        $v1, $v0, .L8006D4A0
    /* 5DC5C 8006D45C FFFF2226 */   addiu     $v0, $s1, -0x1
    /* 5DC60 8006D460 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5DC64 8006D464 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5DC68 8006D468 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5DC6C 8006D46C C0210400 */  sll        $a0, $a0, 7
    /* 5DC70 8006D470 0E004284 */  lh         $v0, 0xE($v0)
    /* 5DC74 8006D474 FF018424 */  addiu      $a0, $a0, 0x1FF
    /* 5DC78 8006D478 C8F7010C */  jal        math_Sin
    /* 5DC7C 8006D47C 3400A2AF */   sw        $v0, 0x34($sp)
    /* 5DC80 8006D480 40110200 */  sll        $v0, $v0, 5
    /* 5DC84 8006D484 03130200 */  sra        $v0, $v0, 12
    /* 5DC88 8006D488 80FF4224 */  addiu      $v0, $v0, -0x80
    /* 5DC8C 8006D48C 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5DC90 8006D490 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5DC94 8006D494 4100A2A3 */  sb         $v0, 0x41($sp)
    /* 5DC98 8006D498 3BB50108 */  j          .L8006D4EC
    /* 5DC9C 8006D49C 2800A0AF */   sw        $zero, 0x28($sp)
  .L8006D4A0:
    /* 5DCA0 8006D4A0 FFFF4230 */  andi       $v0, $v0, 0xFFFF
    /* 5DCA4 8006D4A4 0200422C */  sltiu      $v0, $v0, 0x2
    /* 5DCA8 8006D4A8 0B004014 */  bnez       $v0, .L8006D4D8
    /* 5DCAC 8006D4AC 03000224 */   addiu     $v0, $zero, 0x3
    /* 5DCB0 8006D4B0 09006210 */  beq        $v1, $v0, .L8006D4D8
    /* 5DCB4 8006D4B4 04008224 */   addiu     $v0, $a0, 0x4
    /* 5DCB8 8006D4B8 07006214 */  bne        $v1, $v0, .L8006D4D8
    /* 5DCBC 8006D4BC 80000224 */   addiu     $v0, $zero, 0x80
    /* 5DCC0 8006D4C0 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5DCC4 8006D4C4 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5DCC8 8006D4C8 4100A2A3 */  sb         $v0, 0x41($sp)
    /* 5DCCC 8006D4CC 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5DCD0 8006D4D0 3BB50108 */  j          .L8006D4EC
    /* 5DCD4 8006D4D4 2800A0AF */   sw        $zero, 0x28($sp)
  .L8006D4D8:
    /* 5DCD8 8006D4D8 4300B2A3 */  sb         $s2, 0x43($sp)
    /* 5DCDC 8006D4DC 4200B2A3 */  sb         $s2, 0x42($sp)
    /* 5DCE0 8006D4E0 4100B2A3 */  sb         $s2, 0x41($sp)
    /* 5DCE4 8006D4E4 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5DCE8 8006D4E8 2800B3AF */  sw         $s3, 0x28($sp)
  .L8006D4EC:
    /* 5DCEC 8006D4EC 00141100 */  sll        $v0, $s1, 16
  .L8006D4F0:
    /* 5DCF0 8006D4F0 83130200 */  sra        $v0, $v0, 14
    /* 5DCF4 8006D4F4 21105400 */  addu       $v0, $v0, $s4
    /* 5DCF8 8006D4F8 0000428C */  lw         $v0, 0x0($v0)
    /* 5DCFC 8006D4FC 00000000 */  nop
    /* 5DD00 8006D500 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5DD04 8006D504 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5DD08 8006D508 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 5DD0C 8006D50C 1400028E */  lw         $v0, 0x14($s0)
    /* 5DD10 8006D510 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5DD14 8006D514 4BCD010C */  jal        func_8007352C
    /* 5DD18 8006D518 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5DD1C 8006D51C 140002AE */  sw         $v0, 0x14($s0)
    /* 5DD20 8006D520 1800A48F */  lw         $a0, 0x18($sp)
    /* 5DD24 8006D524 20B9010C */  jal        func_8006E480
    /* 5DD28 8006D528 2128A002 */   addu      $a1, $s5, $zero
    /* 5DD2C 8006D52C 01000524 */  addiu      $a1, $zero, 0x1
    /* 5DD30 8006D530 21300000 */  addu       $a2, $zero, $zero
    /* 5DD34 8006D534 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5DD38 8006D538 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5DD3C 8006D53C 92F0010C */  jal        initTexPage
    /* 5DD40 8006D540 21384000 */   addu      $a3, $v0, $zero
    /* 5DD44 8006D544 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5DD48 8006D548 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5DD4C 8006D54C 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5DD50 8006D550 2DEA010C */  jal        ot_Link
    /* 5DD54 8006D554 28008424 */   addiu     $a0, $a0, 0x28
    /* 5DD58 8006D558 01002226 */  addiu      $v0, $s1, 0x1
    /* 5DD5C 8006D55C 21884000 */  addu       $s1, $v0, $zero
    /* 5DD60 8006D560 00140200 */  sll        $v0, $v0, 16
    /* 5DD64 8006D564 03140200 */  sra        $v0, $v0, 16
    /* 5DD68 8006D568 1C00038E */  lw         $v1, 0x1C($s0)
    /* 5DD6C 8006D56C 06004228 */  slti       $v0, $v0, 0x6
    /* 5DD70 8006D570 0C006324 */  addiu      $v1, $v1, 0xC
    /* 5DD74 8006D574 ADFF4014 */  bnez       $v0, .L8006D42C
    /* 5DD78 8006D578 1C0003AE */   sw        $v1, 0x1C($s0)
    /* 5DD7C 8006D57C 21200002 */  addu       $a0, $s0, $zero
    /* 5DD80 8006D580 4800A527 */  addiu      $a1, $sp, 0x48
    /* 5DD84 8006D584 11000624 */  addiu      $a2, $zero, 0x11
    /* 5DD88 8006D588 DA000224 */  addiu      $v0, $zero, 0xDA
    /* 5DD8C 8006D58C 4800A2A7 */  sh         $v0, 0x48($sp)
    /* 5DD90 8006D590 25000224 */  addiu      $v0, $zero, 0x25
    /* 5DD94 8006D594 4A00A2A7 */  sh         $v0, 0x4A($sp)
    /* 5DD98 8006D598 CB000224 */  addiu      $v0, $zero, 0xCB
    /* 5DD9C 8006D59C 4C00A2A7 */  sh         $v0, 0x4C($sp)
    /* 5DDA0 8006D5A0 01000224 */  addiu      $v0, $zero, 0x1
    /* 5DDA4 8006D5A4 26A6010C */  jal        func_80069898
    /* 5DDA8 8006D5A8 4E00A2A7 */   sh        $v0, 0x4E($sp)
    /* 5DDAC 8006D5AC 6800BF8F */  lw         $ra, 0x68($sp)
    /* 5DDB0 8006D5B0 6400B58F */  lw         $s5, 0x64($sp)
    /* 5DDB4 8006D5B4 6000B48F */  lw         $s4, 0x60($sp)
    /* 5DDB8 8006D5B8 5C00B38F */  lw         $s3, 0x5C($sp)
    /* 5DDBC 8006D5BC 5800B28F */  lw         $s2, 0x58($sp)
    /* 5DDC0 8006D5C0 5400B18F */  lw         $s1, 0x54($sp)
    /* 5DDC4 8006D5C4 5000B08F */  lw         $s0, 0x50($sp)
    /* 5DDC8 8006D5C8 7000BD27 */  addiu      $sp, $sp, 0x70
    /* 5DDCC 8006D5CC 0800E003 */  jr         $ra
    /* 5DDD0 8006D5D0 00000000 */   nop
endlabel func_8006D3DC
