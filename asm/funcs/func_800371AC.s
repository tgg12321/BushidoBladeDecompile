glabel func_800371AC
    /* 279AC 800371AC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 279B0 800371B0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 279B4 800371B4 44DC000C */  jal        func_80037110
    /* 279B8 800371B8 00000000 */   nop
    /* 279BC 800371BC 03004014 */  bnez       $v0, .L800371CC
    /* 279C0 800371C0 01000224 */   addiu     $v0, $zero, 0x1
    /* 279C4 800371C4 76DC0008 */  j          .L800371D8
    /* 279C8 800371C8 21100000 */   addu      $v0, $zero, $zero
  .L800371CC:
    /* 279CC 800371CC 1080013C */  lui        $at, %hi(D_80101E64)
    /* 279D0 800371D0 641E22A4 */  sh         $v0, %lo(D_80101E64)($at)
    /* 279D4 800371D4 01000224 */  addiu      $v0, $zero, 0x1
  .L800371D8:
    /* 279D8 800371D8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 279DC 800371DC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 279E0 800371E0 0800E003 */  jr         $ra
    /* 279E4 800371E4 00000000 */   nop
endlabel func_800371AC
