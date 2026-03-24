glabel func_8003AB44
    /* 2B344 8003AB44 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2B348 8003AB48 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2B34C 8003AB4C E0078393 */  lbu        $v1, %gp_rel(D_800A38AC)($gp)
    /* 2B350 8003AB50 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2B354 8003AB54 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2B358 8003AB58 01004224 */  addiu      $v0, $v0, 0x1
    /* 2B35C 8003AB5C 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2B360 8003AB60 B83722AC */  sw         $v0, %lo(D_800A37B8)($at)
    /* 2B364 8003AB64 0800622C */  sltiu      $v0, $v1, 0x8
    /* 2B368 8003AB68 4E004010 */  beqz       $v0, .L8003ACA4
    /* 2B36C 8003AB6C 80100300 */   sll       $v0, $v1, 2
    /* 2B370 8003AB70 0180013C */  lui        $at, %hi(jtbl_80010CA4)
    /* 2B374 8003AB74 21082200 */  addu       $at, $at, $v0
    /* 2B378 8003AB78 A40C228C */  lw         $v0, %lo(jtbl_80010CA4)($at)
    /* 2B37C 8003AB7C 00000000 */  nop
    /* 2B380 8003AB80 08004000 */  jr         $v0
    /* 2B384 8003AB84 00000000 */   nop
  jlabel .L8003AB88
    /* 2B388 8003AB88 01000224 */  addiu      $v0, $zero, 0x1
    /* 2B38C 8003AB8C E00782A3 */  sb         $v0, %gp_rel(D_800A38AC)($gp)
    /* 2B390 8003AB90 2AEB0008 */  j          .L8003ACA8
    /* 2B394 8003AB94 21100000 */   addu      $v0, $zero, $zero
  jlabel .L8003AB98
    /* 2B398 8003AB98 C2E8000C */  jal        func_8003A308
    /* 2B39C 8003AB9C 00000000 */   nop
    /* 2B3A0 8003ABA0 0A80023C */  lui        $v0, %hi(D_800A38A0)
    /* 2B3A4 8003ABA4 A038428C */  lw         $v0, %lo(D_800A38A0)($v0)
    /* 2B3A8 8003ABA8 0C0780AF */  sw         $zero, %gp_rel(D_800A37D8)($gp)
    /* 2B3AC 8003ABAC 07004014 */  bnez       $v0, .L8003ABCC
    /* 2B3B0 8003ABB0 03000224 */   addiu     $v0, $zero, 0x3
    /* 2B3B4 8003ABB4 A8EC010C */  jal        func_8007B2A0
    /* 2B3B8 8003ABB8 01000424 */   addiu     $a0, $zero, 0x1
    /* 2B3BC 8003ABBC 02000224 */  addiu      $v0, $zero, 0x2
    /* 2B3C0 8003ABC0 E00782A3 */  sb         $v0, %gp_rel(D_800A38AC)($gp)
    /* 2B3C4 8003ABC4 2AEB0008 */  j          .L8003ACA8
    /* 2B3C8 8003ABC8 21100000 */   addu      $v0, $zero, $zero
  .L8003ABCC:
    /* 2B3CC 8003ABCC E00782A3 */  sb         $v0, %gp_rel(D_800A38AC)($gp)
    /* 2B3D0 8003ABD0 2AEB0008 */  j          .L8003ACA8
    /* 2B3D4 8003ABD4 21100000 */   addu      $v0, $zero, $zero
  jlabel .L8003ABD8
    /* 2B3D8 8003ABD8 1080023C */  lui        $v0, %hi(D_80102794)
    /* 2B3DC 8003ABDC 9427428C */  lw         $v0, %lo(D_80102794)($v0)
    /* 2B3E0 8003ABE0 00000000 */  nop
    /* 2B3E4 8003ABE4 10004230 */  andi       $v0, $v0, 0x10
    /* 2B3E8 8003ABE8 1C004014 */  bnez       $v0, .L8003AC5C
    /* 2B3EC 8003ABEC 03000424 */   addiu     $a0, $zero, 0x3
    /* 2B3F0 8003ABF0 01000524 */  addiu      $a1, $zero, 0x1
    /* 2B3F4 8003ABF4 1931020C */  jal        func_8008C464
    /* 2B3F8 8003ABF8 21300000 */   addu      $a2, $zero, $zero
    /* 2B3FC 8003ABFC 2A004010 */  beqz       $v0, .L8003ACA8
    /* 2B400 8003AC00 21100000 */   addu      $v0, $zero, $zero
    /* 2B404 8003AC04 09EB0008 */  j          .L8003AC24
    /* 2B408 8003AC08 03000424 */   addiu     $a0, $zero, 0x3
  jlabel .L8003AC0C
    /* 2B40C 8003AC0C 03000424 */  addiu      $a0, $zero, 0x3
    /* 2B410 8003AC10 01000524 */  addiu      $a1, $zero, 0x1
    /* 2B414 8003AC14 1931020C */  jal        func_8008C464
    /* 2B418 8003AC18 21300000 */   addu      $a2, $zero, $zero
    /* 2B41C 8003AC1C 08004014 */  bnez       $v0, .L8003AC40
    /* 2B420 8003AC20 03000424 */   addiu     $a0, $zero, 0x3
  .L8003AC24:
    /* 2B424 8003AC24 21280000 */  addu       $a1, $zero, $zero
    /* 2B428 8003AC28 1931020C */  jal        func_8008C464
    /* 2B42C 8003AC2C 21300000 */   addu      $a2, $zero, $zero
    /* 2B430 8003AC30 04000224 */  addiu      $v0, $zero, 0x4
    /* 2B434 8003AC34 E00782A3 */  sb         $v0, %gp_rel(D_800A38AC)($gp)
    /* 2B438 8003AC38 2AEB0008 */  j          .L8003ACA8
    /* 2B43C 8003AC3C 21100000 */   addu      $v0, $zero, $zero
  .L8003AC40:
    /* 2B440 8003AC40 0C07828F */  lw         $v0, %gp_rel(D_800A37D8)($gp)
    /* 2B444 8003AC44 00000000 */  nop
    /* 2B448 8003AC48 01004224 */  addiu      $v0, $v0, 0x1
    /* 2B44C 8003AC4C 0C0782AF */  sw         $v0, %gp_rel(D_800A37D8)($gp)
    /* 2B450 8003AC50 04004228 */  slti       $v0, $v0, 0x4
    /* 2B454 8003AC54 14004014 */  bnez       $v0, .L8003ACA8
    /* 2B458 8003AC58 21100000 */   addu      $v0, $zero, $zero
  .L8003AC5C:
    /* 2B45C 8003AC5C E7E8000C */  jal        func_8003A39C
    /* 2B460 8003AC60 00000000 */   nop
    /* 2B464 8003AC64 2AEB0008 */  j          .L8003ACA8
    /* 2B468 8003AC68 FFFF0224 */   addiu     $v0, $zero, -0x1
  jlabel .L8003AC6C
    /* 2B46C 8003AC6C A8EC010C */  jal        func_8007B2A0
    /* 2B470 8003AC70 21200000 */   addu      $a0, $zero, $zero
  jlabel .L8003AC74
    /* 2B474 8003AC74 E0078293 */  lbu        $v0, %gp_rel(D_800A38AC)($gp)
    /* 2B478 8003AC78 00000000 */  nop
    /* 2B47C 8003AC7C 01004224 */  addiu      $v0, $v0, 0x1
    /* 2B480 8003AC80 E00782A3 */  sb         $v0, %gp_rel(D_800A38AC)($gp)
    /* 2B484 8003AC84 2AEB0008 */  j          .L8003ACA8
    /* 2B488 8003AC88 21100000 */   addu      $v0, $zero, $zero
  jlabel .L8003AC8C
    /* 2B48C 8003AC8C 01000224 */  addiu      $v0, $zero, 0x1
    /* 2B490 8003AC90 4A0882A3 */  sb         $v0, %gp_rel(D_800A3916)($gp)
    /* 2B494 8003AC94 D8E8000C */  jal        func_8003A360
    /* 2B498 8003AC98 00000000 */   nop
    /* 2B49C 8003AC9C 2AEB0008 */  j          .L8003ACA8
    /* 2B4A0 8003ACA0 01000224 */   addiu     $v0, $zero, 0x1
  .L8003ACA4:
    /* 2B4A4 8003ACA4 21100000 */  addu       $v0, $zero, $zero
  .L8003ACA8:
    /* 2B4A8 8003ACA8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2B4AC 8003ACAC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2B4B0 8003ACB0 0800E003 */  jr         $ra
    /* 2B4B4 8003ACB4 00000000 */   nop
endlabel func_8003AB44
