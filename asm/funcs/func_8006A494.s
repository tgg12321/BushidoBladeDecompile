glabel func_8006A494
    /* 5AC94 8006A494 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 5AC98 8006A498 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 5AC9C 8006A49C 21888000 */  addu       $s1, $a0, $zero
    /* 5ACA0 8006A4A0 2000BFAF */  sw         $ra, 0x20($sp)
    /* 5ACA4 8006A4A4 1800B0AF */  sw         $s0, 0x18($sp)
    /* 5ACA8 8006A4A8 0400228E */  lw         $v0, 0x4($s1)
    /* 5ACAC 8006A4AC 2180A000 */  addu       $s0, $a1, $zero
    /* 5ACB0 8006A4B0 1C00428C */  lw         $v0, 0x1C($v0)
    /* 5ACB4 8006A4B4 21200002 */  addu       $a0, $s0, $zero
    /* 5ACB8 8006A4B8 2400438C */  lw         $v1, 0x24($v0)
    /* 5ACBC 8006A4BC 01000224 */  addiu      $v0, $zero, 0x1
    /* 5ACC0 8006A4C0 180000AE */  sw         $zero, 0x18($s0)
    /* 5ACC4 8006A4C4 1C0000AE */  sw         $zero, 0x1C($s0)
    /* 5ACC8 8006A4C8 280000A2 */  sb         $zero, 0x28($s0)
    /* 5ACCC 8006A4CC 100000AE */  sw         $zero, 0x10($s0)
    /* 5ACD0 8006A4D0 140002AE */  sw         $v0, 0x14($s0)
    /* 5ACD4 8006A4D4 000003AE */  sw         $v1, 0x0($s0)
    /* 5ACD8 8006A4D8 0000028E */  lw         $v0, 0x0($s0)
    /* 5ACDC 8006A4DC 00010324 */  addiu      $v1, $zero, 0x100
    /* 5ACE0 8006A4E0 200003AE */  sw         $v1, 0x20($s0)
    /* 5ACE4 8006A4E4 240003AE */  sw         $v1, 0x24($s0)
    /* 5ACE8 8006A4E8 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5ACEC 8006A4EC 040002AE */  sw         $v0, 0x4($s0)
    /* 5ACF0 8006A4F0 0800228E */  lw         $v0, 0x8($s1)
    /* 5ACF4 8006A4F4 21280000 */  addu       $a1, $zero, $zero
    /* 5ACF8 8006A4F8 CACD010C */  jal        func_80073728
    /* 5ACFC 8006A4FC 0C0002AE */   sw        $v0, 0xC($s0)
    /* 5AD00 8006A500 080022AE */  sw         $v0, 0x8($s1)
    /* 5AD04 8006A504 0000048E */  lw         $a0, 0x0($s0)
    /* 5AD08 8006A508 20B9010C */  jal        func_8006E480
    /* 5AD0C 8006A50C 21280000 */   addu      $a1, $zero, $zero
    /* 5AD10 8006A510 01000524 */  addiu      $a1, $zero, 0x1
    /* 5AD14 8006A514 21300000 */  addu       $a2, $zero, $zero
    /* 5AD18 8006A518 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5AD1C 8006A51C 1C00248E */  lw         $a0, 0x1C($s1)
    /* 5AD20 8006A520 92F0010C */  jal        initTexPage
    /* 5AD24 8006A524 21384000 */   addu      $a3, $v0, $zero
    /* 5AD28 8006A528 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5AD2C 8006A52C 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5AD30 8006A530 1C00258E */  lw         $a1, 0x1C($s1)
    /* 5AD34 8006A534 2DEA010C */  jal        ot_Link
    /* 5AD38 8006A538 04008424 */   addiu     $a0, $a0, 0x4
    /* 5AD3C 8006A53C 1C00228E */  lw         $v0, 0x1C($s1)
    /* 5AD40 8006A540 00000000 */  nop
    /* 5AD44 8006A544 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5AD48 8006A548 1C0022AE */  sw         $v0, 0x1C($s1)
    /* 5AD4C 8006A54C 2000BF8F */  lw         $ra, 0x20($sp)
    /* 5AD50 8006A550 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 5AD54 8006A554 1800B08F */  lw         $s0, 0x18($sp)
    /* 5AD58 8006A558 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 5AD5C 8006A55C 0800E003 */  jr         $ra
    /* 5AD60 8006A560 00000000 */   nop
endlabel func_8006A494
