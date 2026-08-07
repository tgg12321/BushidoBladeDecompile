glabel func_8002F2D0
    /* 1FAD0 8002F2D0 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 1FAD4 8002F2D4 2400B3AF */  sw         $s3, 0x24($sp)
    /* 1FAD8 8002F2D8 2198A000 */  addu       $s3, $a1, $zero
    /* 1FADC 8002F2DC 801F023C */  lui        $v0, (0x1F800390 >> 16)
    /* 1FAE0 8002F2E0 90034234 */  ori        $v0, $v0, (0x1F800390 & 0xFFFF)
    /* 1FAE4 8002F2E4 3C00BFAF */  sw         $ra, 0x3C($sp)
    /* 1FAE8 8002F2E8 3800BEAF */  sw         $fp, 0x38($sp)
    /* 1FAEC 8002F2EC 3400B7AF */  sw         $s7, 0x34($sp)
    /* 1FAF0 8002F2F0 3000B6AF */  sw         $s6, 0x30($sp)
    /* 1FAF4 8002F2F4 2C00B5AF */  sw         $s5, 0x2C($sp)
    /* 1FAF8 8002F2F8 2800B4AF */  sw         $s4, 0x28($sp)
    /* 1FAFC 8002F2FC 2000B2AF */  sw         $s2, 0x20($sp)
    /* 1FB00 8002F300 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 1FB04 8002F304 1800B0AF */  sw         $s0, 0x18($sp)
    /* 1FB08 8002F308 0000838C */  lw         $v1, 0x0($a0)
    /* 1FB0C 8002F30C 0400858C */  lw         $a1, 0x4($a0)
    /* 1FB10 8002F310 0800868C */  lw         $a2, 0x8($a0)
    /* 1FB14 8002F314 0C00878C */  lw         $a3, 0xC($a0)
    /* 1FB18 8002F318 000043AC */  sw         $v1, 0x0($v0)
    /* 1FB1C 8002F31C 040045AC */  sw         $a1, 0x4($v0)
    /* 1FB20 8002F320 080046AC */  sw         $a2, 0x8($v0)
    /* 1FB24 8002F324 0C0047AC */  sw         $a3, 0xC($v0)
    /* 1FB28 8002F328 1000838C */  lw         $v1, 0x10($a0)
    /* 1FB2C 8002F32C 1400858C */  lw         $a1, 0x14($a0)
    /* 1FB30 8002F330 1800868C */  lw         $a2, 0x18($a0)
    /* 1FB34 8002F334 1C00878C */  lw         $a3, 0x1C($a0)
    /* 1FB38 8002F338 100043AC */  sw         $v1, 0x10($v0)
    /* 1FB3C 8002F33C 140045AC */  sw         $a1, 0x14($v0)
    /* 1FB40 8002F340 180046AC */  sw         $a2, 0x18($v0)
    /* 1FB44 8002F344 1C0047AC */  sw         $a3, 0x1C($v0)
    /* 1FB48 8002F348 801F083C */  lui        $t0, (0x1F80039A >> 16)
    /* 1FB4C 8002F34C 9A030885 */  lh         $t0, (0x1F80039A & 0xFFFF)($t0)
    /* 1FB50 8002F350 801F043C */  lui        $a0, (0x1F80039E >> 16)
    /* 1FB54 8002F354 9E038484 */  lh         $a0, (0x1F80039E & 0xFFFF)($a0)
    /* 1FB58 8002F358 00000000 */  nop
    /* 1FB5C 8002F35C 18000401 */  mult       $t0, $a0
    /* 1FB60 8002F360 801F033C */  lui        $v1, (0x1F800398 >> 16)
    /* 1FB64 8002F364 98036384 */  lh         $v1, (0x1F800398 & 0xFFFF)($v1)
    /* 1FB68 8002F368 12280000 */  mflo       $a1
    /* 1FB6C 8002F36C 801F073C */  lui        $a3, (0x1F8003A0 >> 16)
    /* 1FB70 8002F370 A003E784 */  lh         $a3, (0x1F8003A0 & 0xFFFF)($a3)
    /* 1FB74 8002F374 00000000 */  nop
    /* 1FB78 8002F378 18006700 */  mult       $v1, $a3
    /* 1FB7C 8002F37C 00004984 */  lh         $t1, 0x0($v0)
    /* 1FB80 8002F380 12300000 */  mflo       $a2
    /* 1FB84 8002F384 2388A600 */  subu       $s1, $a1, $a2
    /* 1FB88 8002F388 03131100 */  sra        $v0, $s1, 12
    /* 1FB8C 8002F38C 18002201 */  mult       $t1, $v0
    /* 1FB90 8002F390 12580000 */  mflo       $t3
    /* 1FB94 8002F394 801F023C */  lui        $v0, (0x1F800392 >> 16)
    /* 1FB98 8002F398 92034284 */  lh         $v0, (0x1F800392 & 0xFFFF)($v0)
    /* 1FB9C 8002F39C 00000000 */  nop
    /* 1FBA0 8002F3A0 18004700 */  mult       $v0, $a3
    /* 1FBA4 8002F3A4 12300000 */  mflo       $a2
    /* 1FBA8 8002F3A8 801F053C */  lui        $a1, (0x1F800394 >> 16)
    /* 1FBAC 8002F3AC 9403A584 */  lh         $a1, (0x1F800394 & 0xFFFF)($a1)
    /* 1FBB0 8002F3B0 00000000 */  nop
    /* 1FBB4 8002F3B4 1800A400 */  mult       $a1, $a0
    /* 1FBB8 8002F3B8 12500000 */  mflo       $t2
    /* 1FBBC 8002F3BC 00000000 */  nop
    /* 1FBC0 8002F3C0 00000000 */  nop
    /* 1FBC4 8002F3C4 1800A300 */  mult       $a1, $v1
    /* 1FBC8 8002F3C8 12200000 */  mflo       $a0
    /* 1FBCC 8002F3CC 00000000 */  nop
    /* 1FBD0 8002F3D0 00000000 */  nop
    /* 1FBD4 8002F3D4 18004800 */  mult       $v0, $t0
    /* 1FBD8 8002F3D8 2380CA00 */  subu       $s0, $a2, $t2
    /* 1FBDC 8002F3DC 12180000 */  mflo       $v1
    /* 1FBE0 8002F3E0 801F063C */  lui        $a2, (0x1F800396 >> 16)
    /* 1FBE4 8002F3E4 9603C684 */  lh         $a2, (0x1F800396 & 0xFFFF)($a2)
    /* 1FBE8 8002F3E8 03131000 */  sra        $v0, $s0, 12
    /* 1FBEC 8002F3EC 1800C200 */  mult       $a2, $v0
    /* 1FBF0 8002F3F0 23188300 */  subu       $v1, $a0, $v1
    /* 1FBF4 8002F3F4 12500000 */  mflo       $t2
    /* 1FBF8 8002F3F8 801F043C */  lui        $a0, (0x1F80039C >> 16)
    /* 1FBFC 8002F3FC 9C038484 */  lh         $a0, (0x1F80039C & 0xFFFF)($a0)
    /* 1FC00 8002F400 03130300 */  sra        $v0, $v1, 12
    /* 1FC04 8002F404 18008200 */  mult       $a0, $v0
    /* 1FC08 8002F408 21106A01 */  addu       $v0, $t3, $t2
    /* 1FC0C 8002F40C 12900000 */  mflo       $s2
    /* 1FC10 8002F410 21105200 */  addu       $v0, $v0, $s2
    /* 1FC14 8002F414 03530200 */  sra        $t2, $v0, 12
    /* 1FC18 8002F418 1A002A02 */  div        $zero, $s1, $t2
    /* 1FC1C 8002F41C 02004015 */  bnez       $t2, .L8002F428
    /* 1FC20 8002F420 00000000 */   nop
    /* 1FC24 8002F424 0D000700 */  break      7
  .L8002F428:
    /* 1FC28 8002F428 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1FC2C 8002F42C 04004115 */  bne        $t2, $at, .L8002F440
    /* 1FC30 8002F430 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1FC34 8002F434 02002116 */  bne        $s1, $at, .L8002F440
    /* 1FC38 8002F438 00000000 */   nop
    /* 1FC3C 8002F43C 0D000600 */  break      6
  .L8002F440:
    /* 1FC40 8002F440 12880000 */  mflo       $s1
    /* 1FC44 8002F444 00000000 */  nop
    /* 1FC48 8002F448 00000000 */  nop
    /* 1FC4C 8002F44C 1A000A02 */  div        $zero, $s0, $t2
    /* 1FC50 8002F450 02004015 */  bnez       $t2, .L8002F45C
    /* 1FC54 8002F454 00000000 */   nop
    /* 1FC58 8002F458 0D000700 */  break      7
  .L8002F45C:
    /* 1FC5C 8002F45C FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1FC60 8002F460 04004115 */  bne        $t2, $at, .L8002F474
    /* 1FC64 8002F464 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1FC68 8002F468 02000116 */  bne        $s0, $at, .L8002F474
    /* 1FC6C 8002F46C 00000000 */   nop
    /* 1FC70 8002F470 0D000600 */  break      6
  .L8002F474:
    /* 1FC74 8002F474 12800000 */  mflo       $s0
    /* 1FC78 8002F478 00000000 */  nop
    /* 1FC7C 8002F47C 00000000 */  nop
    /* 1FC80 8002F480 1A006A00 */  div        $zero, $v1, $t2
    /* 1FC84 8002F484 02004015 */  bnez       $t2, .L8002F490
    /* 1FC88 8002F488 00000000 */   nop
    /* 1FC8C 8002F48C 0D000700 */  break      7
  .L8002F490:
    /* 1FC90 8002F490 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1FC94 8002F494 04004115 */  bne        $t2, $at, .L8002F4A8
    /* 1FC98 8002F498 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1FC9C 8002F49C 02006114 */  bne        $v1, $at, .L8002F4A8
    /* 1FCA0 8002F4A0 00000000 */   nop
    /* 1FCA4 8002F4A4 0D000600 */  break      6
  .L8002F4A8:
    /* 1FCA8 8002F4A8 12A00000 */  mflo       $s4
    /* 1FCAC 8002F4AC 00000000 */  nop
    /* 1FCB0 8002F4B0 00000000 */  nop
    /* 1FCB4 8002F4B4 1800C700 */  mult       $a2, $a3
    /* 1FCB8 8002F4B8 12100000 */  mflo       $v0
    /* 1FCBC 8002F4BC 00000000 */  nop
    /* 1FCC0 8002F4C0 00000000 */  nop
    /* 1FCC4 8002F4C4 18000401 */  mult       $t0, $a0
    /* 1FCC8 8002F4C8 12180000 */  mflo       $v1
    /* 1FCCC 8002F4CC 23104300 */  subu       $v0, $v0, $v1
    /* 1FCD0 8002F4D0 00000000 */  nop
    /* 1FCD4 8002F4D4 1A004A00 */  div        $zero, $v0, $t2
    /* 1FCD8 8002F4D8 02004015 */  bnez       $t2, .L8002F4E4
    /* 1FCDC 8002F4DC 00000000 */   nop
    /* 1FCE0 8002F4E0 0D000700 */  break      7
  .L8002F4E4:
    /* 1FCE4 8002F4E4 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1FCE8 8002F4E8 04004115 */  bne        $t2, $at, .L8002F4FC
    /* 1FCEC 8002F4EC 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1FCF0 8002F4F0 02004114 */  bne        $v0, $at, .L8002F4FC
    /* 1FCF4 8002F4F4 00000000 */   nop
    /* 1FCF8 8002F4F8 0D000600 */  break      6
  .L8002F4FC:
    /* 1FCFC 8002F4FC 12F00000 */  mflo       $fp
    /* 1FD00 8002F500 00000000 */  nop
    /* 1FD04 8002F504 00000000 */  nop
    /* 1FD08 8002F508 1800A400 */  mult       $a1, $a0
    /* 1FD0C 8002F50C 12100000 */  mflo       $v0
    /* 1FD10 8002F510 00000000 */  nop
    /* 1FD14 8002F514 00000000 */  nop
    /* 1FD18 8002F518 18002701 */  mult       $t1, $a3
    /* 1FD1C 8002F51C 12180000 */  mflo       $v1
    /* 1FD20 8002F520 23104300 */  subu       $v0, $v0, $v1
    /* 1FD24 8002F524 00000000 */  nop
    /* 1FD28 8002F528 1A004A00 */  div        $zero, $v0, $t2
    /* 1FD2C 8002F52C 02004015 */  bnez       $t2, .L8002F538
    /* 1FD30 8002F530 00000000 */   nop
    /* 1FD34 8002F534 0D000700 */  break      7
  .L8002F538:
    /* 1FD38 8002F538 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1FD3C 8002F53C 04004115 */  bne        $t2, $at, .L8002F550
    /* 1FD40 8002F540 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1FD44 8002F544 02004114 */  bne        $v0, $at, .L8002F550
    /* 1FD48 8002F548 00000000 */   nop
    /* 1FD4C 8002F54C 0D000600 */  break      6
  .L8002F550:
    /* 1FD50 8002F550 12B80000 */  mflo       $s7
    /* 1FD54 8002F554 00000000 */  nop
    /* 1FD58 8002F558 00000000 */  nop
    /* 1FD5C 8002F55C 18002801 */  mult       $t1, $t0
    /* 1FD60 8002F560 12100000 */  mflo       $v0
    /* 1FD64 8002F564 00000000 */  nop
    /* 1FD68 8002F568 00000000 */  nop
    /* 1FD6C 8002F56C 1800A600 */  mult       $a1, $a2
    /* 1FD70 8002F570 12180000 */  mflo       $v1
    /* 1FD74 8002F574 23104300 */  subu       $v0, $v0, $v1
    /* 1FD78 8002F578 00000000 */  nop
    /* 1FD7C 8002F57C 1A004A00 */  div        $zero, $v0, $t2
    /* 1FD80 8002F580 02004015 */  bnez       $t2, .L8002F58C
    /* 1FD84 8002F584 00000000 */   nop
    /* 1FD88 8002F588 0D000700 */  break      7
  .L8002F58C:
    /* 1FD8C 8002F58C FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1FD90 8002F590 04004115 */  bne        $t2, $at, .L8002F5A4
    /* 1FD94 8002F594 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1FD98 8002F598 02004114 */  bne        $v0, $at, .L8002F5A4
    /* 1FD9C 8002F59C 00000000 */   nop
    /* 1FDA0 8002F5A0 0D000600 */  break      6
  .L8002F5A4:
    /* 1FDA4 8002F5A4 12B00000 */  mflo       $s6
    /* 1FDA8 8002F5A8 21200002 */  addu       $a0, $s0, $zero
    /* 1FDAC 8002F5AC 57FF010C */  jal        func_8007FD5C
    /* 1FDB0 8002F5B0 21282002 */   addu      $a1, $s1, $zero
    /* 1FDB4 8002F5B4 18003102 */  mult       $s1, $s1
    /* 1FDB8 8002F5B8 12180000 */  mflo       $v1
    /* 1FDBC 8002F5BC 00000000 */  nop
    /* 1FDC0 8002F5C0 00000000 */  nop
    /* 1FDC4 8002F5C4 18001002 */  mult       $s0, $s0
    /* 1FDC8 8002F5C8 801F123C */  lui        $s2, (0x1F8002B8 >> 16)
    /* 1FDCC 8002F5CC 23A80200 */  negu       $s5, $v0
    /* 1FDD0 8002F5D0 12300000 */  mflo       $a2
    /* 1FDD4 8002F5D4 21206600 */  addu       $a0, $v1, $a2
    /* 1FDD8 8002F5D8 0004822C */  sltiu      $v0, $a0, 0x400
    /* 1FDDC 8002F5DC 06004010 */  beqz       $v0, .L8002F5F8
    /* 1FDE0 8002F5E0 B8025236 */   ori       $s2, $s2, (0x1F8002B8 & 0xFFFF)
    /* 1FDE4 8002F5E4 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1FDE8 8002F5E8 21082400 */  addu       $at, $at, $a0
    /* 1FDEC 8002F5EC 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1FDF0 8002F5F0 95BD0008 */  j          .L8002F654
    /* 1FDF4 8002F5F4 C2500200 */   srl       $t2, $v0, 3
  .L8002F5F8:
    /* 1FDF8 8002F5F8 09008004 */  bltz       $a0, .L8002F620
    /* 1FDFC 8002F5FC 21180000 */   addu      $v1, $zero, $zero
    /* 1FE00 8002F600 21608000 */  addu       $t4, $a0, $zero
    /* 1FE04 8002F604 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1FE08 8002F608 00000000 */  nop
    /* 1FE0C 8002F60C 00000000 */  nop
    /* 1FE10 8002F610 1000A227 */  addiu      $v0, $sp, 0x10
    /* 1FE14 8002F614 21604000 */  addu       $t4, $v0, $zero
    /* 1FE18 8002F618 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1FE1C 8002F61C 1000A38F */  lw         $v1, 0x10($sp)
  .L8002F620:
    /* 1FE20 8002F620 FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1FE24 8002F624 24106200 */  and        $v0, $v1, $v0
    /* 1FE28 8002F628 16000324 */  addiu      $v1, $zero, 0x16
    /* 1FE2C 8002F62C 23186200 */  subu       $v1, $v1, $v0
    /* 1FE30 8002F630 06106400 */  srlv       $v0, $a0, $v1
    /* 1FE34 8002F634 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1FE38 8002F638 21082200 */  addu       $at, $at, $v0
    /* 1FE3C 8002F63C 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1FE40 8002F640 42180300 */  srl        $v1, $v1, 1
    /* 1FE44 8002F644 13000224 */  addiu      $v0, $zero, 0x13
    /* 1FE48 8002F648 23104300 */  subu       $v0, $v0, $v1
    /* 1FE4C 8002F64C 00240400 */  sll        $a0, $a0, 16
    /* 1FE50 8002F650 06504400 */  srlv       $t2, $a0, $v0
  .L8002F654:
    /* 1FE54 8002F654 21208002 */  addu       $a0, $s4, $zero
    /* 1FE58 8002F658 57FF010C */  jal        func_8007FD5C
    /* 1FE5C 8002F65C 21284001 */   addu      $a1, $t2, $zero
    /* 1FE60 8002F660 2120A002 */  addu       $a0, $s5, $zero
    /* 1FE64 8002F664 D8005026 */  addiu      $s0, $s2, 0xD8
    /* 1FE68 8002F668 21280002 */  addu       $a1, $s0, $zero
    /* 1FE6C 8002F66C 00100324 */  addiu      $v1, $zero, 0x1000
    /* 1FE70 8002F670 21884000 */  addu       $s1, $v0, $zero
    /* 1FE74 8002F674 D80043A6 */  sh         $v1, 0xD8($s2)
    /* 1FE78 8002F678 DA0040A6 */  sh         $zero, 0xDA($s2)
    /* 1FE7C 8002F67C DC0040A6 */  sh         $zero, 0xDC($s2)
    /* 1FE80 8002F680 DE0040A6 */  sh         $zero, 0xDE($s2)
    /* 1FE84 8002F684 E00043A6 */  sh         $v1, 0xE0($s2)
    /* 1FE88 8002F688 E20040A6 */  sh         $zero, 0xE2($s2)
    /* 1FE8C 8002F68C E40040A6 */  sh         $zero, 0xE4($s2)
    /* 1FE90 8002F690 E60040A6 */  sh         $zero, 0xE6($s2)
    /* 1FE94 8002F694 EFFE010C */  jal        func_8007FBBC
    /* 1FE98 8002F698 E80043A6 */   sh        $v1, 0xE8($s2)
    /* 1FE9C 8002F69C 21202002 */  addu       $a0, $s1, $zero
    /* 1FEA0 8002F6A0 87FE010C */  jal        func_8007FA1C
    /* 1FEA4 8002F6A4 21280002 */   addu      $a1, $s0, $zero
    /* 1FEA8 8002F6A8 21600002 */  addu       $t4, $s0, $zero
    /* 1FEAC 8002F6AC 00008D8D */  lw         $t5, 0x0($t4)
    /* 1FEB0 8002F6B0 04008E8D */  lw         $t6, 0x4($t4)
    /* 1FEB4 8002F6B4 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 1FEB8 8002F6B8 0008CE48 */  ctc2       $t6, $1 /* handwritten instruction */
    /* 1FEBC 8002F6BC 08008D8D */  lw         $t5, 0x8($t4)
    /* 1FEC0 8002F6C0 0C008E8D */  lw         $t6, 0xC($t4)
    /* 1FEC4 8002F6C4 10008F8D */  lw         $t7, 0x10($t4)
    /* 1FEC8 8002F6C8 0010CD48 */  ctc2       $t5, $2 /* handwritten instruction */
    /* 1FECC 8002F6CC 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 1FED0 8002F6D0 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 1FED4 8002F6D4 A8004226 */  addiu      $v0, $s2, 0xA8
    /* 1FED8 8002F6D8 A8005EAE */  sw         $fp, 0xA8($s2)
    /* 1FEDC 8002F6DC AC0057AE */  sw         $s7, 0xAC($s2)
    /* 1FEE0 8002F6E0 B00056AE */  sw         $s6, 0xB0($s2)
    /* 1FEE4 8002F6E4 21604000 */  addu       $t4, $v0, $zero
    /* 1FEE8 8002F6E8 04008E95 */  lhu        $t6, 0x4($t4)
    /* 1FEEC 8002F6EC 00008D95 */  lhu        $t5, 0x0($t4)
    /* 1FEF0 8002F6F0 00740E00 */  sll        $t6, $t6, 16
    /* 1FEF4 8002F6F4 2568AE01 */  or         $t5, $t5, $t6
    /* 1FEF8 8002F6F8 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 1FEFC 8002F6FC 080081C9 */  lwc2       $1, 0x8($t4)
    /* 1FF00 8002F700 00000000 */  nop
    /* 1FF04 8002F704 00000000 */  nop
    /* 1FF08 8002F708 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1FF0C 8002F70C 21604000 */  addu       $t4, $v0, $zero
    /* 1FF10 8002F710 000099E9 */  swc2       $25, 0x0($t4)
    /* 1FF14 8002F714 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1FF18 8002F718 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1FF1C 8002F71C B000448E */  lw         $a0, 0xB0($s2)
    /* 1FF20 8002F720 AC00458E */  lw         $a1, 0xAC($s2)
    /* 1FF24 8002F724 57FF010C */  jal        func_8007FD5C
    /* 1FF28 8002F728 23881100 */   negu      $s1, $s1
    /* 1FF2C 8002F72C 000062A6 */  sh         $v0, 0x0($s3)
    /* 1FF30 8002F730 23101500 */  negu       $v0, $s5
    /* 1FF34 8002F734 020071A6 */  sh         $s1, 0x2($s3)
    /* 1FF38 8002F738 040062A6 */  sh         $v0, 0x4($s3)
    /* 1FF3C 8002F73C 3C00BF8F */  lw         $ra, 0x3C($sp)
    /* 1FF40 8002F740 3800BE8F */  lw         $fp, 0x38($sp)
    /* 1FF44 8002F744 3400B78F */  lw         $s7, 0x34($sp)
    /* 1FF48 8002F748 3000B68F */  lw         $s6, 0x30($sp)
    /* 1FF4C 8002F74C 2C00B58F */  lw         $s5, 0x2C($sp)
    /* 1FF50 8002F750 2800B48F */  lw         $s4, 0x28($sp)
    /* 1FF54 8002F754 2400B38F */  lw         $s3, 0x24($sp)
    /* 1FF58 8002F758 2000B28F */  lw         $s2, 0x20($sp)
    /* 1FF5C 8002F75C 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 1FF60 8002F760 1800B08F */  lw         $s0, 0x18($sp)
    /* 1FF64 8002F764 4000BD27 */  addiu      $sp, $sp, 0x40
    /* 1FF68 8002F768 0800E003 */  jr         $ra
    /* 1FF6C 8002F76C 00000000 */   nop
endlabel func_8002F2D0
