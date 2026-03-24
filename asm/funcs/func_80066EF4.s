glabel func_80066EF4
    /* 576F4 80066EF4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 576F8 80066EF8 21200000 */  addu       $a0, $zero, $zero
    /* 576FC 80066EFC 21280000 */  addu       $a1, $zero, $zero
    /* 57700 80066F00 1000BFAF */  sw         $ra, 0x10($sp)
    /* 57704 80066F04 809C010C */  jal        func_80067200
    /* 57708 80066F08 01000624 */   addiu     $a2, $zero, 0x1
    /* 5770C 80066F0C 02000324 */  addiu      $v1, $zero, 0x2
    /* 57710 80066F10 0F80013C */  lui        $at, %hi(D_800F10D8)
    /* 57714 80066F14 D81023AC */  sw         $v1, %lo(D_800F10D8)($at)
    /* 57718 80066F18 1000BF8F */  lw         $ra, 0x10($sp)
    /* 5771C 80066F1C FF004230 */  andi       $v0, $v0, 0xFF
    /* 57720 80066F20 0800E003 */  jr         $ra
    /* 57724 80066F24 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80066EF4
