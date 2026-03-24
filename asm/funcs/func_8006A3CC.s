glabel func_8006A3CC
    /* 5ABCC 8006A3CC D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 5ABD0 8006A3D0 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 5ABD4 8006A3D4 21888000 */  addu       $s1, $a0, $zero
    /* 5ABD8 8006A3D8 2000BFAF */  sw         $ra, 0x20($sp)
    /* 5ABDC 8006A3DC 1800B0AF */  sw         $s0, 0x18($sp)
    /* 5ABE0 8006A3E0 0400228E */  lw         $v0, 0x4($s1)
    /* 5ABE4 8006A3E4 00000000 */  nop
    /* 5ABE8 8006A3E8 1C00428C */  lw         $v0, 0x1C($v0)
    /* 5ABEC 8006A3EC 00000000 */  nop
    /* 5ABF0 8006A3F0 1000428C */  lw         $v0, 0x10($v0)
    /* 5ABF4 8006A3F4 2180A000 */  addu       $s0, $a1, $zero
    /* 5ABF8 8006A3F8 000002AE */  sw         $v0, 0x0($s0)
    /* 5ABFC 8006A3FC 0000028E */  lw         $v0, 0x0($s0)
    /* 5AC00 8006A400 01000324 */  addiu      $v1, $zero, 0x1
    /* 5AC04 8006A404 180000AE */  sw         $zero, 0x18($s0)
    /* 5AC08 8006A408 1C0000AE */  sw         $zero, 0x1C($s0)
    /* 5AC0C 8006A40C 280000A2 */  sb         $zero, 0x28($s0)
    /* 5AC10 8006A410 100000AE */  sw         $zero, 0x10($s0)
    /* 5AC14 8006A414 140003AE */  sw         $v1, 0x14($s0)
    /* 5AC18 8006A418 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5AC1C 8006A41C 040002AE */  sw         $v0, 0x4($s0)
    /* 5AC20 8006A420 1400228E */  lw         $v0, 0x14($s1)
    /* 5AC24 8006A424 21200002 */  addu       $a0, $s0, $zero
    /* 5AC28 8006A428 4BCD010C */  jal        func_8007352C
    /* 5AC2C 8006A42C 080002AE */   sw        $v0, 0x8($s0)
    /* 5AC30 8006A430 140022AE */  sw         $v0, 0x14($s1)
    /* 5AC34 8006A434 0000048E */  lw         $a0, 0x0($s0)
    /* 5AC38 8006A438 20B9010C */  jal        func_8006E480
    /* 5AC3C 8006A43C 21280000 */   addu      $a1, $zero, $zero
    /* 5AC40 8006A440 01000524 */  addiu      $a1, $zero, 0x1
    /* 5AC44 8006A444 21300000 */  addu       $a2, $zero, $zero
    /* 5AC48 8006A448 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5AC4C 8006A44C 1C00248E */  lw         $a0, 0x1C($s1)
    /* 5AC50 8006A450 92F0010C */  jal        func_8007C248
    /* 5AC54 8006A454 21384000 */   addu      $a3, $v0, $zero
    /* 5AC58 8006A458 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5AC5C 8006A45C 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5AC60 8006A460 1C00258E */  lw         $a1, 0x1C($s1)
    /* 5AC64 8006A464 2DEA010C */  jal        func_8007A8B4
    /* 5AC68 8006A468 04008424 */   addiu     $a0, $a0, 0x4
    /* 5AC6C 8006A46C 1C00228E */  lw         $v0, 0x1C($s1)
    /* 5AC70 8006A470 00000000 */  nop
    /* 5AC74 8006A474 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5AC78 8006A478 1C0022AE */  sw         $v0, 0x1C($s1)
    /* 5AC7C 8006A47C 2000BF8F */  lw         $ra, 0x20($sp)
    /* 5AC80 8006A480 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 5AC84 8006A484 1800B08F */  lw         $s0, 0x18($sp)
    /* 5AC88 8006A488 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 5AC8C 8006A48C 0800E003 */  jr         $ra
    /* 5AC90 8006A490 00000000 */   nop
endlabel func_8006A3CC
