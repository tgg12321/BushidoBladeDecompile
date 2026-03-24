glabel func_80078D68
    /* 69568 80078D68 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6956C 80078D6C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 69570 80078D70 DDE3010C */  jal        func_80078F74
    /* 69574 80078D74 00000000 */   nop
    /* 69578 80078D78 C8E3010C */  jal        func_80078F20
    /* 6957C 80078D7C 00000000 */   nop
    /* 69580 80078D80 88E3010C */  jal        func_80078E20
    /* 69584 80078D84 00000000 */   nop
    /* 69588 80078D88 0A80013C */  lui        $at, %hi(D_8009BD80)
    /* 6958C 80078D8C 80BD20AC */  sw         $zero, %lo(D_8009BD80)($at)
    /* 69590 80078D90 1000BF8F */  lw         $ra, 0x10($sp)
    /* 69594 80078D94 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 69598 80078D98 0800E003 */  jr         $ra
    /* 6959C 80078D9C 00000000 */   nop
endlabel func_80078D68
