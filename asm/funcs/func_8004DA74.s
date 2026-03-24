glabel func_8004DA74
    /* 3E274 8004DA74 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3E278 8004DA78 3000BFAF */  sw         $ra, 0x30($sp)
    /* 3E27C 8004DA7C 2C00BEAF */  sw         $fp, 0x2C($sp)
    /* 3E280 8004DA80 2800B7AF */  sw         $s7, 0x28($sp)
    /* 3E284 8004DA84 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3E288 8004DA88 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3E28C 8004DA8C 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3E290 8004DA90 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3E294 8004DA94 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3E298 8004DA98 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3E29C 8004DA9C 21888000 */  addu       $s1, $a0, $zero
    /* 3E2A0 8004DAA0 21A8A000 */  addu       $s5, $a1, $zero
    /* 3E2A4 8004DAA4 04002886 */  lh         $t0, 0x4($s1)
    /* 3E2A8 8004DAA8 02002986 */  lh         $t1, 0x2($s1)
    /* 3E2AC 8004DAAC 80400800 */  sll        $t0, $t0, 2
    /* 3E2B0 8004DAB0 10800A3C */  lui        $t2, %hi(D_80103608)
    /* 3E2B4 8004DAB4 08364A25 */  addiu      $t2, $t2, %lo(D_80103608)
    /* 3E2B8 8004DAB8 21400A01 */  addu       $t0, $t0, $t2
    /* 3E2BC 8004DABC 0000088D */  lw         $t0, 0x0($t0)
    /* 3E2C0 8004DAC0 80480900 */  sll        $t1, $t1, 2
    /* 3E2C4 8004DAC4 21400901 */  addu       $t0, $t0, $t1
    /* 3E2C8 8004DAC8 0000108D */  lw         $s0, 0x0($t0)
    /* 3E2CC 8004DACC 00000000 */  nop
    /* 3E2D0 8004DAD0 0000058E */  lw         $a1, 0x0($s0)
    /* 3E2D4 8004DAD4 00000000 */  nop
    /* 3E2D8 8004DAD8 FFFFA430 */  andi       $a0, $a1, 0xFFFF
    /* 3E2DC 8004DADC 022C0500 */  srl        $a1, $a1, 16
    /* 3E2E0 8004DAE0 A8008010 */  beqz       $a0, .L8004DD84
    /* 3E2E4 8004DAE4 04001022 */   addi      $s0, $s0, 0x4 /* handwritten instruction */
    /* 3E2E8 8004DAE8 20408400 */  add        $t0, $a0, $a0 /* handwritten instruction */
    /* 3E2EC 8004DAEC 20400401 */  add        $t0, $t0, $a0 /* handwritten instruction */
    /* 3E2F0 8004DAF0 20400801 */  add        $t0, $t0, $t0 /* handwritten instruction */
    /* 3E2F4 8004DAF4 20800802 */  add        $s0, $s0, $t0 /* handwritten instruction */
    /* 3E2F8 8004DAF8 03000832 */  andi       $t0, $s0, 0x3
    /* 3E2FC 8004DAFC 20800802 */  add        $s0, $s0, $t0 /* handwritten instruction */
    /* 3E300 8004DB00 6000328E */  lw         $s2, 0x60($s1)
    /* 3E304 8004DB04 0A80023C */  lui        $v0, %hi(D_800A3708)
    /* 3E308 8004DB08 0837428C */  lw         $v0, %lo(D_800A3708)($v0)
    /* 3E30C 8004DB0C FFFF8424 */  addiu      $a0, $a0, -0x1
    /* 3E310 8004DB10 2C004B8C */  lw         $t3, 0x2C($v0)
    /* 3E314 8004DB14 30004C8C */  lw         $t4, 0x30($v0)
    /* 3E318 8004DB18 34004D8C */  lw         $t5, 0x34($v0)
    /* 3E31C 8004DB1C 0000488E */  lw         $t0, 0x0($s2)
    /* 3E320 8004DB20 0400498E */  lw         $t1, 0x4($s2)
    /* 3E324 8004DB24 08004A8E */  lw         $t2, 0x8($s2)
    /* 3E328 8004DB28 C3410800 */  sra        $t0, $t0, 7
    /* 3E32C 8004DB2C C3490900 */  sra        $t1, $t1, 7
    /* 3E330 8004DB30 C3510A00 */  sra        $t2, $t2, 7
    /* 3E334 8004DB34 22400B01 */  sub        $t0, $t0, $t3 /* handwritten instruction */
    /* 3E338 8004DB38 22482C01 */  sub        $t1, $t1, $t4 /* handwritten instruction */
    /* 3E33C 8004DB3C 22504D01 */  sub        $t2, $t2, $t5 /* handwritten instruction */
    /* 3E340 8004DB40 0740A802 */  srav       $t0, $t0, $s5
    /* 3E344 8004DB44 0748A902 */  srav       $t1, $t1, $s5
    /* 3E348 8004DB48 0750AA02 */  srav       $t2, $t2, $s5
    /* 3E34C 8004DB4C 004C0900 */  sll        $t1, $t1, 16
    /* 3E350 8004DB50 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3E354 8004DB54 25400901 */  or         $t0, $t0, $t1
    /* 3E358 8004DB58 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 3E35C 8004DB5C 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 3E360 8004DB60 4000488E */  lw         $t0, 0x40($s2)
    /* 3E364 8004DB64 4400498E */  lw         $t1, 0x44($s2)
    /* 3E368 8004DB68 48004A8E */  lw         $t2, 0x48($s2)
    /* 3E36C 8004DB6C C3410800 */  sra        $t0, $t0, 7
    /* 3E370 8004DB70 C3490900 */  sra        $t1, $t1, 7
    /* 3E374 8004DB74 C3510A00 */  sra        $t2, $t2, 7
    /* 3E378 8004DB78 22400B01 */  sub        $t0, $t0, $t3 /* handwritten instruction */
    /* 3E37C 8004DB7C 22482C01 */  sub        $t1, $t1, $t4 /* handwritten instruction */
    /* 3E380 8004DB80 22504D01 */  sub        $t2, $t2, $t5 /* handwritten instruction */
    /* 3E384 8004DB84 0740A802 */  srav       $t0, $t0, $s5
    /* 3E388 8004DB88 0748A902 */  srav       $t1, $t1, $s5
    /* 3E38C 8004DB8C 0750AA02 */  srav       $t2, $t2, $s5
    /* 3E390 8004DB90 004C0900 */  sll        $t1, $t1, 16
    /* 3E394 8004DB94 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3E398 8004DB98 25400901 */  or         $t0, $t0, $t1
    /* 3E39C 8004DB9C 00108848 */  mtc2       $t0, $2 /* handwritten instruction */
    /* 3E3A0 8004DBA0 00188A48 */  mtc2       $t2, $3 /* handwritten instruction */
    /* 3E3A4 8004DBA4 8000488E */  lw         $t0, 0x80($s2)
    /* 3E3A8 8004DBA8 8400498E */  lw         $t1, 0x84($s2)
    /* 3E3AC 8004DBAC 88004A8E */  lw         $t2, 0x88($s2)
    /* 3E3B0 8004DBB0 C3410800 */  sra        $t0, $t0, 7
    /* 3E3B4 8004DBB4 C3490900 */  sra        $t1, $t1, 7
    /* 3E3B8 8004DBB8 C3510A00 */  sra        $t2, $t2, 7
    /* 3E3BC 8004DBBC 22400B01 */  sub        $t0, $t0, $t3 /* handwritten instruction */
    /* 3E3C0 8004DBC0 22482C01 */  sub        $t1, $t1, $t4 /* handwritten instruction */
    /* 3E3C4 8004DBC4 22504D01 */  sub        $t2, $t2, $t5 /* handwritten instruction */
    /* 3E3C8 8004DBC8 0740A802 */  srav       $t0, $t0, $s5
    /* 3E3CC 8004DBCC 0748A902 */  srav       $t1, $t1, $s5
    /* 3E3D0 8004DBD0 0750AA02 */  srav       $t2, $t2, $s5
    /* 3E3D4 8004DBD4 004C0900 */  sll        $t1, $t1, 16
    /* 3E3D8 8004DBD8 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3E3DC 8004DBDC 25400901 */  or         $t0, $t0, $t1
    /* 3E3E0 8004DBE0 00208848 */  mtc2       $t0, $4 /* handwritten instruction */
    /* 3E3E4 8004DBE4 00288A48 */  mtc2       $t2, $5 /* handwritten instruction */
    /* 3E3E8 8004DBE8 801F133C */  lui        $s3, (0x1F800020 >> 16)
    /* 3E3EC 8004DBEC 20007336 */  ori        $s3, $s3, (0x1F800020 & 0xFFFF)
    /* 3E3F0 8004DBF0 801F143C */  lui        $s4, (0x1F8002B4 >> 16)
    /* 3E3F4 8004DBF4 B4029436 */  ori        $s4, $s4, (0x1F8002B4 & 0xFFFF)
    /* 3E3F8 8004DBF8 3000284A */  rtpt
    /* 3E3FC 8004DBFC 06370108 */  j          .L8004DC18
    /* 3E400 8004DC00 C0005222 */   addi      $s2, $s2, 0xC0 /* handwritten instruction */
  .L8004DC04:
    /* 3E404 8004DC04 3000284A */  rtpt
    /* 3E408 8004DC08 06009426 */  addiu      $s4, $s4, 0x6
    /* 3E40C 8004DC0C FAFF8EA6 */  sh         $t6, -0x6($s4)
    /* 3E410 8004DC10 FCFF8FA6 */  sh         $t7, -0x4($s4)
    /* 3E414 8004DC14 FEFF98A6 */  sh         $t8, -0x2($s4)
  .L8004DC18:
    /* 3E418 8004DC18 00006CEA */  swc2       $12, 0x0($s3)
    /* 3E41C 8004DC1C 04006DEA */  swc2       $13, 0x4($s3)
    /* 3E420 8004DC20 08006EEA */  swc2       $14, 0x8($s3)
    /* 3E424 8004DC24 00880E48 */  mfc2       $t6, $17 /* handwritten instruction */
    /* 3E428 8004DC28 00900F48 */  mfc2       $t7, $18 /* handwritten instruction */
    /* 3E42C 8004DC2C 00981848 */  mfc2       $t8, $19 /* handwritten instruction */
    /* 3E430 8004DC30 0C007326 */  addiu      $s3, $s3, 0xC
    /* 3E434 8004DC34 36008010 */  beqz       $a0, .L8004DD10
    /* 3E438 8004DC38 FFFF8424 */   addiu     $a0, $a0, -0x1
    /* 3E43C 8004DC3C 0000488E */  lw         $t0, 0x0($s2)
    /* 3E440 8004DC40 0400498E */  lw         $t1, 0x4($s2)
    /* 3E444 8004DC44 08004A8E */  lw         $t2, 0x8($s2)
    /* 3E448 8004DC48 C3410800 */  sra        $t0, $t0, 7
    /* 3E44C 8004DC4C C3490900 */  sra        $t1, $t1, 7
    /* 3E450 8004DC50 C3510A00 */  sra        $t2, $t2, 7
    /* 3E454 8004DC54 22400B01 */  sub        $t0, $t0, $t3 /* handwritten instruction */
    /* 3E458 8004DC58 22482C01 */  sub        $t1, $t1, $t4 /* handwritten instruction */
    /* 3E45C 8004DC5C 22504D01 */  sub        $t2, $t2, $t5 /* handwritten instruction */
    /* 3E460 8004DC60 0740A802 */  srav       $t0, $t0, $s5
    /* 3E464 8004DC64 0748A902 */  srav       $t1, $t1, $s5
    /* 3E468 8004DC68 0750AA02 */  srav       $t2, $t2, $s5
    /* 3E46C 8004DC6C 004C0900 */  sll        $t1, $t1, 16
    /* 3E470 8004DC70 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3E474 8004DC74 25400901 */  or         $t0, $t0, $t1
    /* 3E478 8004DC78 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 3E47C 8004DC7C 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 3E480 8004DC80 4000488E */  lw         $t0, 0x40($s2)
    /* 3E484 8004DC84 4400498E */  lw         $t1, 0x44($s2)
    /* 3E488 8004DC88 48004A8E */  lw         $t2, 0x48($s2)
    /* 3E48C 8004DC8C C3410800 */  sra        $t0, $t0, 7
    /* 3E490 8004DC90 C3490900 */  sra        $t1, $t1, 7
    /* 3E494 8004DC94 C3510A00 */  sra        $t2, $t2, 7
    /* 3E498 8004DC98 22400B01 */  sub        $t0, $t0, $t3 /* handwritten instruction */
    /* 3E49C 8004DC9C 22482C01 */  sub        $t1, $t1, $t4 /* handwritten instruction */
    /* 3E4A0 8004DCA0 22504D01 */  sub        $t2, $t2, $t5 /* handwritten instruction */
    /* 3E4A4 8004DCA4 0740A802 */  srav       $t0, $t0, $s5
    /* 3E4A8 8004DCA8 0748A902 */  srav       $t1, $t1, $s5
    /* 3E4AC 8004DCAC 0750AA02 */  srav       $t2, $t2, $s5
    /* 3E4B0 8004DCB0 004C0900 */  sll        $t1, $t1, 16
    /* 3E4B4 8004DCB4 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3E4B8 8004DCB8 25400901 */  or         $t0, $t0, $t1
    /* 3E4BC 8004DCBC 00108848 */  mtc2       $t0, $2 /* handwritten instruction */
    /* 3E4C0 8004DCC0 00188A48 */  mtc2       $t2, $3 /* handwritten instruction */
    /* 3E4C4 8004DCC4 8000488E */  lw         $t0, 0x80($s2)
    /* 3E4C8 8004DCC8 8400498E */  lw         $t1, 0x84($s2)
    /* 3E4CC 8004DCCC 88004A8E */  lw         $t2, 0x88($s2)
    /* 3E4D0 8004DCD0 C3410800 */  sra        $t0, $t0, 7
    /* 3E4D4 8004DCD4 C3490900 */  sra        $t1, $t1, 7
    /* 3E4D8 8004DCD8 C3510A00 */  sra        $t2, $t2, 7
    /* 3E4DC 8004DCDC 22400B01 */  sub        $t0, $t0, $t3 /* handwritten instruction */
    /* 3E4E0 8004DCE0 22482C01 */  sub        $t1, $t1, $t4 /* handwritten instruction */
    /* 3E4E4 8004DCE4 22504D01 */  sub        $t2, $t2, $t5 /* handwritten instruction */
    /* 3E4E8 8004DCE8 0740A802 */  srav       $t0, $t0, $s5
    /* 3E4EC 8004DCEC 0748A902 */  srav       $t1, $t1, $s5
    /* 3E4F0 8004DCF0 0750AA02 */  srav       $t2, $t2, $s5
    /* 3E4F4 8004DCF4 004C0900 */  sll        $t1, $t1, 16
    /* 3E4F8 8004DCF8 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3E4FC 8004DCFC 25400901 */  or         $t0, $t0, $t1
    /* 3E500 8004DD00 00208848 */  mtc2       $t0, $4 /* handwritten instruction */
    /* 3E504 8004DD04 00288A48 */  mtc2       $t2, $5 /* handwritten instruction */
    /* 3E508 8004DD08 01370108 */  j          .L8004DC04
    /* 3E50C 8004DD0C C0005226 */   addiu     $s2, $s2, 0xC0
  .L8004DD10:
    /* 3E510 8004DD10 00008EA6 */  sh         $t6, 0x0($s4)
    /* 3E514 8004DD14 02008FA6 */  sh         $t7, 0x2($s4)
    /* 3E518 8004DD18 040098A6 */  sh         $t8, 0x4($s4)
    /* 3E51C 8004DD1C 03000824 */  addiu      $t0, $zero, 0x3
    /* 3E520 8004DD20 23401501 */  subu       $t0, $t0, $s5
    /* 3E524 8004DD24 801F013C */  lui        $at, (0x1F800008 >> 16)
    /* 3E528 8004DD28 080028AC */  sw         $t0, (0x1F800008 & 0xFFFF)($at)
    /* 3E52C 8004DD2C 00000496 */  lhu        $a0, 0x0($s0)
    /* 3E530 8004DD30 02001026 */  addiu      $s0, $s0, 0x2
  .L8004DD34:
    /* 3E534 8004DD34 00000896 */  lhu        $t0, 0x0($s0)
    /* 3E538 8004DD38 02001026 */  addiu      $s0, $s0, 0x2
    /* 3E53C 8004DD3C C2400800 */  srl        $t0, $t0, 3
    /* 3E540 8004DD40 80400800 */  sll        $t0, $t0, 2
    /* 3E544 8004DD44 01002992 */  lbu        $t1, 0x1($s1)
    /* 3E548 8004DD48 01800A3C */  lui        $t2, %hi(D_80015480)
    /* 3E54C 8004DD4C 80544A25 */  addiu      $t2, $t2, %lo(D_80015480)
    /* 3E550 8004DD50 20400A01 */  add        $t0, $t0, $t2 /* handwritten instruction */
    /* 3E554 8004DD54 01002931 */  andi       $t1, $t1, 0x1
    /* 3E558 8004DD58 02002011 */  beqz       $t1, .L8004DD64
    /* 3E55C 8004DD5C 00000000 */   nop
    /* 3E560 8004DD60 10000821 */  addi       $t0, $t0, 0x10 /* handwritten instruction */
  .L8004DD64:
    /* 3E564 8004DD64 0000088D */  lw         $t0, 0x0($t0)
    /* 3E568 8004DD68 21280002 */  addu       $a1, $s0, $zero
    /* 3E56C 8004DD6C 09F80001 */  jalr       $t0
    /* 3E570 8004DD70 00000000 */   nop
    /* 3E574 8004DD74 00000496 */  lhu        $a0, 0x0($s0)
    /* 3E578 8004DD78 02001026 */  addiu      $s0, $s0, 0x2
    /* 3E57C 8004DD7C EDFF8014 */  bnez       $a0, .L8004DD34
    /* 3E580 8004DD80 00000000 */   nop
  .L8004DD84:
    /* 3E584 8004DD84 21102002 */  addu       $v0, $s1, $zero
    /* 3E588 8004DD88 3000BF8F */  lw         $ra, 0x30($sp)
    /* 3E58C 8004DD8C 2C00BE8F */  lw         $fp, 0x2C($sp)
    /* 3E590 8004DD90 2800B78F */  lw         $s7, 0x28($sp)
    /* 3E594 8004DD94 2400B68F */  lw         $s6, 0x24($sp)
    /* 3E598 8004DD98 2000B58F */  lw         $s5, 0x20($sp)
    /* 3E59C 8004DD9C 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3E5A0 8004DDA0 1800B38F */  lw         $s3, 0x18($sp)
    /* 3E5A4 8004DDA4 1400B28F */  lw         $s2, 0x14($sp)
    /* 3E5A8 8004DDA8 1000B18F */  lw         $s1, 0x10($sp)
    /* 3E5AC 8004DDAC 0800E003 */  jr         $ra
    /* 3E5B0 8004DDB0 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004DA74
