glabel func_80078F88
    /* 69788 80078F88 0A80013C */  lui        $at, %hi(D_800A3618)
    /* 6978C 80078F8C 18363FAC */  sw         $ra, %lo(D_800A3618)($at)
    /* 69790 80078F90 6EE2010C */  jal        EnterCriticalSection
    /* 69794 80078F94 00000000 */   nop
    /* 69798 80078F98 B0000A24 */  addiu      $t2, $zero, 0xB0
    /* 6979C 80078F9C 09F84001 */  jalr       $t2
    /* 697A0 80078FA0 57000924 */   addiu     $t1, $zero, 0x57
    /* 697A4 80078FA4 6C01428C */  lw         $v0, 0x16C($v0)
    /* 697A8 80078FA8 0A80013C */  lui        $at, %hi(jtbl_800A3620)
    /* 697AC 80078FAC 84084320 */  addi       $v1, $v0, 0x884 /* handwritten instruction */
    /* 697B0 80078FB0 203623AC */  sw         $v1, %lo(jtbl_800A3620)($at)
    /* 697B4 80078FB4 0A80013C */  lui        $at, %hi(jtbl_800A3624)
    /* 697B8 80078FB8 94084320 */  addi       $v1, $v0, 0x894 /* handwritten instruction */
    /* 697BC 80078FBC 0B000924 */  addiu      $t1, $zero, 0xB
    /* 697C0 80078FC0 243623AC */  sw         $v1, %lo(jtbl_800A3624)($at)
  .L80078FC4:
    /* 697C4 80078FC4 FFFF2925 */  addiu      $t1, $t1, -0x1
    /* 697C8 80078FC8 940540AC */  sw         $zero, 0x594($v0)
    /* 697CC 80078FCC FDFF2015 */  bnez       $t1, .L80078FC4
    /* 697D0 80078FD0 04004224 */   addiu     $v0, $v0, 0x4
    /* 697D4 80078FD4 FCE3010C */  jal        func_80078FF0
    /* 697D8 80078FD8 00000000 */   nop
    /* 697DC 80078FDC 0A801F3C */  lui        $ra, %hi(D_800A3618)
    /* 697E0 80078FE0 1836FF8F */  lw         $ra, %lo(D_800A3618)($ra)
    /* 697E4 80078FE4 00000000 */  nop
    /* 697E8 80078FE8 0800E003 */  jr         $ra
    /* 697EC 80078FEC 00000000 */   nop
endlabel func_80078F88
