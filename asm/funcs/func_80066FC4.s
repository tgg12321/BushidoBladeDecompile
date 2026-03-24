glabel func_80066FC4
    /* 577C4 80066FC4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 577C8 80066FC8 02000424 */  addiu      $a0, $zero, 0x2
    /* 577CC 80066FCC 01000524 */  addiu      $a1, $zero, 0x1
    /* 577D0 80066FD0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 577D4 80066FD4 809C010C */  jal        func_80067200
    /* 577D8 80066FD8 01000624 */   addiu     $a2, $zero, 0x1
    /* 577DC 80066FDC 02000324 */  addiu      $v1, $zero, 0x2
    /* 577E0 80066FE0 0F80013C */  lui        $at, %hi(D_800F1120)
    /* 577E4 80066FE4 201123AC */  sw         $v1, %lo(D_800F1120)($at)
    /* 577E8 80066FE8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 577EC 80066FEC FF004230 */  andi       $v0, $v0, 0xFF
    /* 577F0 80066FF0 0800E003 */  jr         $ra
    /* 577F4 80066FF4 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80066FC4
