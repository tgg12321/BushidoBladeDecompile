glabel func_8007FEDC
    /* 706DC 8007FEDC 0A80013C */  lui        $at, %hi(D_800A3658)
    /* 706E0 8007FEE0 58363FAC */  sw         $ra, %lo(D_800A3658)($at)
    /* 706E4 8007FEE4 6EE2010C */  jal        EnterCriticalSection
    /* 706E8 8007FEE8 00000000 */   nop
    /* 706EC 8007FEEC B0000A24 */  addiu      $t2, $zero, 0xB0
    /* 706F0 8007FEF0 09F84001 */  jalr       $t2
    /* 706F4 8007FEF4 56000924 */   addiu     $t1, $zero, 0x56
    /* 706F8 8007FEF8 08800A3C */  lui        $t2, %hi(D_8007FF44)
    /* 706FC 8007FEFC 0880093C */  lui        $t1, %hi(func_8007FF7C)
    /* 70700 8007FF00 1800428C */  lw         $v0, 0x18($v0)
    /* 70704 8007FF04 44FF4A25 */  addiu      $t2, $t2, %lo(D_8007FF44)
    /* 70708 8007FF08 7CFF2925 */  addiu      $t1, $t1, %lo(func_8007FF7C)
  .L8007FF0C:
    /* 7070C 8007FF0C 0000438D */  lw         $v1, 0x0($t2)
    /* 70710 8007FF10 04004A25 */  addiu      $t2, $t2, 0x4
    /* 70714 8007FF14 04004224 */  addiu      $v0, $v0, 0x4
    /* 70718 8007FF18 FCFF4915 */  bne        $t2, $t1, .L8007FF0C
    /* 7071C 8007FF1C FCFF43AC */   sw        $v1, -0x4($v0)
    /* 70720 8007FF20 FCE3010C */  jal        func_80078FF0
    /* 70724 8007FF24 00000000 */   nop
    /* 70728 8007FF28 72E2010C */  jal        ExitCriticalSection
    /* 7072C 8007FF2C 00000000 */   nop
    /* 70730 8007FF30 0A801F3C */  lui        $ra, %hi(D_800A3658)
    /* 70734 8007FF34 5836FF8F */  lw         $ra, %lo(D_800A3658)($ra)
    /* 70738 8007FF38 00000000 */  nop
    /* 7073C 8007FF3C 0800E003 */  jr         $ra
    /* 70740 8007FF40 00000000 */   nop
  alabel D_8007FF44
    /* 70744 8007FF44 00000000 */  nop
    /* 70748 8007FF48 00000000 */  nop
    /* 7074C 8007FF4C 00011A24 */  addiu      $k0, $zero, 0x100 /* handwritten instruction */
    /* 70750 8007FF50 08005A8F */  lw         $k0, 0x8($k0) /* handwritten instruction */
    /* 70754 8007FF54 00000000 */  nop
    /* 70758 8007FF58 00005A8F */  lw         $k0, 0x0($k0) /* handwritten instruction */
    /* 7075C 8007FF5C 00000000 */  nop
    /* 70760 8007FF60 08005A23 */  addi       $k0, $k0, 0x8 /* handwritten instruction */
    /* 70764 8007FF64 040041AF */  sw         $at, 0x4($k0) /* handwritten instruction */
    /* 70768 8007FF68 080042AF */  sw         $v0, 0x8($k0) /* handwritten instruction */
    /* 7076C 8007FF6C 0C0043AF */  sw         $v1, 0xC($k0) /* handwritten instruction */
    /* 70770 8007FF70 7C005FAF */  sw         $ra, 0x7C($k0) /* handwritten instruction */
    /* 70774 8007FF74 00680240 */  mfc0       $v0, $13 /* handwritten instruction */
endlabel func_8007FEDC
