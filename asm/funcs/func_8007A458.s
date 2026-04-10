glabel func_8007A458
    /* 6AC58 8007A458 0A80013C */  lui        $at, %hi(D_800A3648)
    /* 6AC5C 8007A45C 48363FAC */  sw         $ra, %lo(D_800A3648)($at)
    /* 6AC60 8007A460 6EE2010C */  jal        EnterCriticalSection
    /* 6AC64 8007A464 00000000 */   nop
    /* 6AC68 8007A468 B0000A24 */  addiu      $t2, $zero, 0xB0
    /* 6AC6C 8007A46C 09F84001 */  jalr       $t2
    /* 6AC70 8007A470 56000924 */   addiu     $t1, $zero, 0x56
    /* 6AC74 8007A474 08800A3C */  lui        $t2, %hi(D_8007A4C0)
    /* 6AC78 8007A478 0880093C */  lui        $t1, %hi(D_8007A4CC)
    /* 6AC7C 8007A47C 1800428C */  lw         $v0, 0x18($v0)
    /* 6AC80 8007A480 C0A44A25 */  addiu      $t2, $t2, %lo(D_8007A4C0)
    /* 6AC84 8007A484 CCA42925 */  addiu      $t1, $t1, %lo(D_8007A4CC)
  .L8007A488:
    /* 6AC88 8007A488 0000438D */  lw         $v1, 0x0($t2)
    /* 6AC8C 8007A48C 04004A25 */  addiu      $t2, $t2, 0x4
    /* 6AC90 8007A490 04004224 */  addiu      $v0, $v0, 0x4
    /* 6AC94 8007A494 FCFF4915 */  bne        $t2, $t1, .L8007A488
    /* 6AC98 8007A498 6C0043AC */   sw        $v1, 0x6C($v0)
    /* 6AC9C 8007A49C FCE3010C */  jal        func_80078FF0
    /* 6ACA0 8007A4A0 00000000 */   nop
    /* 6ACA4 8007A4A4 72E2010C */  jal        ExitCriticalSection
    /* 6ACA8 8007A4A8 00000000 */   nop
    /* 6ACAC 8007A4AC 0A801F3C */  lui        $ra, %hi(D_800A3648)
    /* 6ACB0 8007A4B0 4836FF8F */  lw         $ra, %lo(D_800A3648)($ra)
    /* 6ACB4 8007A4B4 00000000 */  nop
    /* 6ACB8 8007A4B8 0800E003 */  jr         $ra
    /* 6ACBC 8007A4BC 00000000 */   nop
    /* Inline data: D_8007A4C0 and D_8007A4CC (zero-initialized) */
    .word 0x00000000 /* 8007A4C0 */
    .word 0x00000000 /* 8007A4C4 */
    .word 0x00000000 /* 8007A4C8 */
    .word 0x00000000 /* 8007A4CC */
    .word 0x00000000 /* 8007A4D0 */
    .word 0x00000000 /* 8007A4D4 */
endlabel func_8007A458
