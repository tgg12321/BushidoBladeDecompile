glabel func_800790C0
    /* 698C0 800790C0 0A80013C */  lui        $at, %hi(D_800A3638)
    /* 698C4 800790C4 38363FAC */  sw         $ra, %lo(D_800A3638)($at)
    /* 698C8 800790C8 6EE2010C */  jal        EnterCriticalSection
    /* 698CC 800790CC 00000000 */   nop
    /* 698D0 800790D0 B0000A24 */  addiu      $t2, $zero, 0xB0
    /* 698D4 800790D4 09F84001 */  jalr       $t2
    /* 698D8 800790D8 57000924 */   addiu     $t1, $zero, 0x57
    /* 698DC 800790DC 6C01428C */  lw         $v0, 0x16C($v0)
    /* 698E0 800790E0 09000A24 */  addiu      $t2, $zero, 0x9
    /* 698E4 800790E4 2C064320 */  addi       $v1, $v0, 0x62C /* handwritten instruction */
  .L800790E8:
    /* 698E8 800790E8 FFFF4A25 */  addiu      $t2, $t2, -0x1
    /* 698EC 800790EC 000060AC */  sw         $zero, 0x0($v1)
    /* 698F0 800790F0 FDFF4015 */  bnez       $t2, .L800790E8
    /* 698F4 800790F4 04006324 */   addiu     $v1, $v1, 0x4
    /* 698F8 800790F8 FCE3010C */  jal        func_80078FF0
    /* 698FC 800790FC 00000000 */   nop
    /* 69900 80079100 72E2010C */  jal        ExitCriticalSection
    /* 69904 80079104 00000000 */   nop
    /* 69908 80079108 0A801F3C */  lui        $ra, %hi(D_800A3638)
    /* 6990C 8007910C 3836FF8F */  lw         $ra, %lo(D_800A3638)($ra)
    /* 69910 80079110 00000000 */  nop
    /* 69914 80079114 0800E003 */  jr         $ra
    /* 69918 80079118 00000000 */   nop
endlabel func_800790C0
