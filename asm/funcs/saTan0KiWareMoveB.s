glabel saTan0KiWareMoveB
    /* 1E2D0 8002DAD0 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 1E2D4 8002DAD4 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 1E2D8 8002DAD8 21888000 */  addu       $s1, $a0, $zero
    /* 1E2DC 8002DADC 2000BFAF */  sw         $ra, 0x20($sp)
    /* 1E2E0 8002DAE0 1800B0AF */  sw         $s0, 0x18($sp)
    /* 1E2E4 8002DAE4 6400228E */  lw         $v0, 0x64($s1)
    /* 1E2E8 8002DAE8 6000238E */  lw         $v1, 0x60($s1)
    /* 1E2EC 8002DAEC 0000428C */  lw         $v0, 0x0($v0)
    /* 1E2F0 8002DAF0 0000638C */  lw         $v1, 0x0($v1)
    /* 1E2F4 8002DAF4 00000000 */  nop
    /* 1E2F8 8002DAF8 23104300 */  subu       $v0, $v0, $v1
    /* 1E2FC 8002DAFC A80022AE */  sw         $v0, 0xA8($s1)
    /* 1E300 8002DB00 6400228E */  lw         $v0, 0x64($s1)
    /* 1E304 8002DB04 6000238E */  lw         $v1, 0x60($s1)
    /* 1E308 8002DB08 0400428C */  lw         $v0, 0x4($v0)
    /* 1E30C 8002DB0C 0400638C */  lw         $v1, 0x4($v1)
    /* 1E310 8002DB10 00000000 */  nop
    /* 1E314 8002DB14 23104300 */  subu       $v0, $v0, $v1
    /* 1E318 8002DB18 AC0022AE */  sw         $v0, 0xAC($s1)
    /* 1E31C 8002DB1C 6400228E */  lw         $v0, 0x64($s1)
    /* 1E320 8002DB20 6000238E */  lw         $v1, 0x60($s1)
    /* 1E324 8002DB24 0800428C */  lw         $v0, 0x8($v0)
    /* 1E328 8002DB28 0800638C */  lw         $v1, 0x8($v1)
    /* 1E32C 8002DB2C 00000000 */  nop
    /* 1E330 8002DB30 23104300 */  subu       $v0, $v0, $v1
    /* 1E334 8002DB34 B00022AE */  sw         $v0, 0xB0($s1)
    /* 1E338 8002DB38 6800228E */  lw         $v0, 0x68($s1)
    /* 1E33C 8002DB3C 6000238E */  lw         $v1, 0x60($s1)
    /* 1E340 8002DB40 0000428C */  lw         $v0, 0x0($v0)
    /* 1E344 8002DB44 0000638C */  lw         $v1, 0x0($v1)
    /* 1E348 8002DB48 00000000 */  nop
    /* 1E34C 8002DB4C 23104300 */  subu       $v0, $v0, $v1
    /* 1E350 8002DB50 B80022AE */  sw         $v0, 0xB8($s1)
    /* 1E354 8002DB54 6800228E */  lw         $v0, 0x68($s1)
    /* 1E358 8002DB58 6000238E */  lw         $v1, 0x60($s1)
    /* 1E35C 8002DB5C 0400428C */  lw         $v0, 0x4($v0)
    /* 1E360 8002DB60 0400638C */  lw         $v1, 0x4($v1)
    /* 1E364 8002DB64 00000000 */  nop
    /* 1E368 8002DB68 23104300 */  subu       $v0, $v0, $v1
    /* 1E36C 8002DB6C BC0022AE */  sw         $v0, 0xBC($s1)
    /* 1E370 8002DB70 6800228E */  lw         $v0, 0x68($s1)
    /* 1E374 8002DB74 6000238E */  lw         $v1, 0x60($s1)
    /* 1E378 8002DB78 0800428C */  lw         $v0, 0x8($v0)
    /* 1E37C 8002DB7C 0800638C */  lw         $v1, 0x8($v1)
    /* 1E380 8002DB80 00000000 */  nop
    /* 1E384 8002DB84 23104300 */  subu       $v0, $v0, $v1
    /* 1E388 8002DB88 C00022AE */  sw         $v0, 0xC0($s1)
    /* 1E38C 8002DB8C A8002226 */  addiu      $v0, $s1, 0xA8
    /* 1E390 8002DB90 21604000 */  addu       $t4, $v0, $zero
    /* 1E394 8002DB94 00008D8D */  lw         $t5, 0x0($t4)
    /* 1E398 8002DB98 04008E8D */  lw         $t6, 0x4($t4)
    /* 1E39C 8002DB9C 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 1E3A0 8002DBA0 08008F8D */  lw         $t7, 0x8($t4)
    /* 1E3A4 8002DBA4 0010CE48 */  ctc2       $t6, $2 /* handwritten instruction */
    /* 1E3A8 8002DBA8 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 1E3AC 8002DBAC B8002226 */  addiu      $v0, $s1, 0xB8
    /* 1E3B0 8002DBB0 21604000 */  addu       $t4, $v0, $zero
    /* 1E3B4 8002DBB4 08008BC9 */  lwc2       $11, 0x8($t4)
    /* 1E3B8 8002DBB8 000089C9 */  lwc2       $9, 0x0($t4)
    /* 1E3BC 8002DBBC 04008AC9 */  lwc2       $10, 0x4($t4)
    /* 1E3C0 8002DBC0 00000000 */  nop
    /* 1E3C4 8002DBC4 00000000 */  nop
    /* 1E3C8 8002DBC8 0C00704B */  op         0
    /* 1E3CC 8002DBCC C8002226 */  addiu      $v0, $s1, 0xC8
    /* 1E3D0 8002DBD0 21604000 */  addu       $t4, $v0, $zero
    /* 1E3D4 8002DBD4 000099E9 */  swc2       $25, 0x0($t4)
    /* 1E3D8 8002DBD8 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1E3DC 8002DBDC 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1E3E0 8002DBE0 C800228E */  lw         $v0, 0xC8($s1)
    /* 1E3E4 8002DBE4 00000000 */  nop
    /* 1E3E8 8002DBE8 FF3F4224 */  addiu      $v0, $v0, 0x3FFF
    /* 1E3EC 8002DBEC FF7F422C */  sltiu      $v0, $v0, 0x7FFF
    /* 1E3F0 8002DBF0 0D004010 */  beqz       $v0, .L8002DC28
    /* 1E3F4 8002DBF4 00000000 */   nop
    /* 1E3F8 8002DBF8 CC00228E */  lw         $v0, 0xCC($s1)
    /* 1E3FC 8002DBFC 00000000 */  nop
    /* 1E400 8002DC00 FF3F4224 */  addiu      $v0, $v0, 0x3FFF
    /* 1E404 8002DC04 FF7F422C */  sltiu      $v0, $v0, 0x7FFF
    /* 1E408 8002DC08 07004010 */  beqz       $v0, .L8002DC28
    /* 1E40C 8002DC0C 00000000 */   nop
    /* 1E410 8002DC10 D000228E */  lw         $v0, 0xD0($s1)
    /* 1E414 8002DC14 00000000 */  nop
    /* 1E418 8002DC18 FF3F4224 */  addiu      $v0, $v0, 0x3FFF
    /* 1E41C 8002DC1C FF7F422C */  sltiu      $v0, $v0, 0x7FFF
    /* 1E420 8002DC20 79004014 */  bnez       $v0, .L8002DE08
    /* 1E424 8002DC24 21100000 */   addu      $v0, $zero, $zero
  .L8002DC28:
    /* 1E428 8002DC28 C800248E */  lw         $a0, 0xC8($s1)
    /* 1E42C 8002DC2C D000258E */  lw         $a1, 0xD0($s1)
    /* 1E430 8002DC30 57FF010C */  jal        func_8007FD5C
    /* 1E434 8002DC34 00000000 */   nop
    /* 1E438 8002DC38 C800238E */  lw         $v1, 0xC8($s1)
    /* 1E43C 8002DC3C 00000000 */  nop
    /* 1E440 8002DC40 83190300 */  sra        $v1, $v1, 6
    /* 1E444 8002DC44 C80023AE */  sw         $v1, 0xC8($s1)
    /* 1E448 8002DC48 C800238E */  lw         $v1, 0xC8($s1)
    /* 1E44C 8002DC4C 00000000 */  nop
    /* 1E450 8002DC50 18006300 */  mult       $v1, $v1
    /* 1E454 8002DC54 D000258E */  lw         $a1, 0xD0($s1)
    /* 1E458 8002DC58 12300000 */  mflo       $a2
    /* 1E45C 8002DC5C 83290500 */  sra        $a1, $a1, 6
    /* 1E460 8002DC60 00000000 */  nop
    /* 1E464 8002DC64 1800A500 */  mult       $a1, $a1
    /* 1E468 8002DC68 CC00248E */  lw         $a0, 0xCC($s1)
    /* 1E46C 8002DC6C 00000000 */  nop
    /* 1E470 8002DC70 83210400 */  sra        $a0, $a0, 6
    /* 1E474 8002DC74 CC0024AE */  sw         $a0, 0xCC($s1)
    /* 1E478 8002DC78 D00025AE */  sw         $a1, 0xD0($s1)
    /* 1E47C 8002DC7C 00080324 */  addiu      $v1, $zero, 0x800
    /* 1E480 8002DC80 23186200 */  subu       $v1, $v1, $v0
    /* 1E484 8002DC84 12400000 */  mflo       $t0
    /* 1E488 8002DC88 2120C800 */  addu       $a0, $a2, $t0
    /* 1E48C 8002DC8C 0004822C */  sltiu      $v0, $a0, 0x400
    /* 1E490 8002DC90 06004010 */  beqz       $v0, .L8002DCAC
    /* 1E494 8002DC94 FA0023A6 */   sh        $v1, 0xFA($s1)
    /* 1E498 8002DC98 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1E49C 8002DC9C 21082400 */  addu       $at, $at, $a0
    /* 1E4A0 8002DCA0 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1E4A4 8002DCA4 42B70008 */  j          .L8002DD08
    /* 1E4A8 8002DCA8 C2280200 */   srl       $a1, $v0, 3
  .L8002DCAC:
    /* 1E4AC 8002DCAC 09008004 */  bltz       $a0, .L8002DCD4
    /* 1E4B0 8002DCB0 21180000 */   addu      $v1, $zero, $zero
    /* 1E4B4 8002DCB4 21608000 */  addu       $t4, $a0, $zero
    /* 1E4B8 8002DCB8 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1E4BC 8002DCBC 00000000 */  nop
    /* 1E4C0 8002DCC0 00000000 */  nop
    /* 1E4C4 8002DCC4 1000A227 */  addiu      $v0, $sp, 0x10
    /* 1E4C8 8002DCC8 21604000 */  addu       $t4, $v0, $zero
    /* 1E4CC 8002DCCC 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1E4D0 8002DCD0 1000A38F */  lw         $v1, 0x10($sp)
  .L8002DCD4:
    /* 1E4D4 8002DCD4 FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1E4D8 8002DCD8 24106200 */  and        $v0, $v1, $v0
    /* 1E4DC 8002DCDC 16000324 */  addiu      $v1, $zero, 0x16
    /* 1E4E0 8002DCE0 23186200 */  subu       $v1, $v1, $v0
    /* 1E4E4 8002DCE4 06106400 */  srlv       $v0, $a0, $v1
    /* 1E4E8 8002DCE8 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1E4EC 8002DCEC 21082200 */  addu       $at, $at, $v0
    /* 1E4F0 8002DCF0 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1E4F4 8002DCF4 42180300 */  srl        $v1, $v1, 1
    /* 1E4F8 8002DCF8 13000224 */  addiu      $v0, $zero, 0x13
    /* 1E4FC 8002DCFC 23104300 */  subu       $v0, $v0, $v1
    /* 1E500 8002DD00 00240400 */  sll        $a0, $a0, 16
    /* 1E504 8002DD04 06284400 */  srlv       $a1, $a0, $v0
  .L8002DD08:
    /* 1E508 8002DD08 CC00248E */  lw         $a0, 0xCC($s1)
    /* 1E50C 8002DD0C 57FF010C */  jal        func_8007FD5C
    /* 1E510 8002DD10 D8003026 */   addiu     $s0, $s1, 0xD8
    /* 1E514 8002DD14 21280002 */  addu       $a1, $s0, $zero
    /* 1E518 8002DD18 00080324 */  addiu      $v1, $zero, 0x800
    /* 1E51C 8002DD1C 23186200 */  subu       $v1, $v1, $v0
    /* 1E520 8002DD20 FA002486 */  lh         $a0, 0xFA($s1)
    /* 1E524 8002DD24 00100224 */  addiu      $v0, $zero, 0x1000
    /* 1E528 8002DD28 F80023A6 */  sh         $v1, 0xF8($s1)
    /* 1E52C 8002DD2C D80022A6 */  sh         $v0, 0xD8($s1)
    /* 1E530 8002DD30 DA0020A6 */  sh         $zero, 0xDA($s1)
    /* 1E534 8002DD34 DC0020A6 */  sh         $zero, 0xDC($s1)
    /* 1E538 8002DD38 DE0020A6 */  sh         $zero, 0xDE($s1)
    /* 1E53C 8002DD3C E00022A6 */  sh         $v0, 0xE0($s1)
    /* 1E540 8002DD40 E20020A6 */  sh         $zero, 0xE2($s1)
    /* 1E544 8002DD44 E40020A6 */  sh         $zero, 0xE4($s1)
    /* 1E548 8002DD48 E60020A6 */  sh         $zero, 0xE6($s1)
    /* 1E54C 8002DD4C 87FE010C */  jal        func_8007FA1C
    /* 1E550 8002DD50 E80022A6 */   sh        $v0, 0xE8($s1)
    /* 1E554 8002DD54 F8002486 */  lh         $a0, 0xF8($s1)
    /* 1E558 8002DD58 1FFE010C */  jal        func_8007F87C
    /* 1E55C 8002DD5C 21280002 */   addu      $a1, $s0, $zero
    /* 1E560 8002DD60 21600002 */  addu       $t4, $s0, $zero
    /* 1E564 8002DD64 00008D8D */  lw         $t5, 0x0($t4)
    /* 1E568 8002DD68 04008E8D */  lw         $t6, 0x4($t4)
    /* 1E56C 8002DD6C 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 1E570 8002DD70 0008CE48 */  ctc2       $t6, $1 /* handwritten instruction */
    /* 1E574 8002DD74 08008D8D */  lw         $t5, 0x8($t4)
    /* 1E578 8002DD78 0C008E8D */  lw         $t6, 0xC($t4)
    /* 1E57C 8002DD7C 10008F8D */  lw         $t7, 0x10($t4)
    /* 1E580 8002DD80 0010CD48 */  ctc2       $t5, $2 /* handwritten instruction */
    /* 1E584 8002DD84 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 1E588 8002DD88 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 1E58C 8002DD8C A8002226 */  addiu      $v0, $s1, 0xA8
    /* 1E590 8002DD90 21604000 */  addu       $t4, $v0, $zero
    /* 1E594 8002DD94 04008E95 */  lhu        $t6, 0x4($t4)
    /* 1E598 8002DD98 00008D95 */  lhu        $t5, 0x0($t4)
    /* 1E59C 8002DD9C 00740E00 */  sll        $t6, $t6, 16
    /* 1E5A0 8002DDA0 2568AE01 */  or         $t5, $t5, $t6
    /* 1E5A4 8002DDA4 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 1E5A8 8002DDA8 080081C9 */  lwc2       $1, 0x8($t4)
    /* 1E5AC 8002DDAC 00000000 */  nop
    /* 1E5B0 8002DDB0 00000000 */  nop
    /* 1E5B4 8002DDB4 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1E5B8 8002DDB8 21604000 */  addu       $t4, $v0, $zero
    /* 1E5BC 8002DDBC 000099E9 */  swc2       $25, 0x0($t4)
    /* 1E5C0 8002DDC0 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1E5C4 8002DDC4 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1E5C8 8002DDC8 B8002226 */  addiu      $v0, $s1, 0xB8
    /* 1E5CC 8002DDCC 21604000 */  addu       $t4, $v0, $zero
    /* 1E5D0 8002DDD0 04008E95 */  lhu        $t6, 0x4($t4)
    /* 1E5D4 8002DDD4 00008D95 */  lhu        $t5, 0x0($t4)
    /* 1E5D8 8002DDD8 00740E00 */  sll        $t6, $t6, 16
    /* 1E5DC 8002DDDC 2568AE01 */  or         $t5, $t5, $t6
    /* 1E5E0 8002DDE0 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 1E5E4 8002DDE4 080081C9 */  lwc2       $1, 0x8($t4)
    /* 1E5E8 8002DDE8 00000000 */  nop
    /* 1E5EC 8002DDEC 00000000 */  nop
    /* 1E5F0 8002DDF0 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1E5F4 8002DDF4 21604000 */  addu       $t4, $v0, $zero
    /* 1E5F8 8002DDF8 000099E9 */  swc2       $25, 0x0($t4)
    /* 1E5FC 8002DDFC 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1E600 8002DE00 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1E604 8002DE04 01000224 */  addiu      $v0, $zero, 0x1
  .L8002DE08:
    /* 1E608 8002DE08 2000BF8F */  lw         $ra, 0x20($sp)
    /* 1E60C 8002DE0C 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 1E610 8002DE10 1800B08F */  lw         $s0, 0x18($sp)
    /* 1E614 8002DE14 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 1E618 8002DE18 0800E003 */  jr         $ra
    /* 1E61C 8002DE1C 00000000 */   nop
endlabel saTan0KiWareMoveB
