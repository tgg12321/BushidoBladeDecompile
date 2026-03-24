glabel func_80066F90
    /* 57790 80066F90 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 57794 80066F94 02000424 */  addiu      $a0, $zero, 0x2
    /* 57798 80066F98 01000524 */  addiu      $a1, $zero, 0x1
    /* 5779C 80066F9C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 577A0 80066FA0 809C010C */  jal        func_80067200
    /* 577A4 80066FA4 21300000 */   addu      $a2, $zero, $zero
    /* 577A8 80066FA8 01000324 */  addiu      $v1, $zero, 0x1
    /* 577AC 80066FAC 0F80013C */  lui        $at, %hi(D_800F1120)
    /* 577B0 80066FB0 201123AC */  sw         $v1, %lo(D_800F1120)($at)
    /* 577B4 80066FB4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 577B8 80066FB8 FF004230 */  andi       $v0, $v0, 0xFF
    /* 577BC 80066FBC 0800E003 */  jr         $ra
    /* 577C0 80066FC0 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80066F90
