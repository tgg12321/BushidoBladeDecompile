glabel func_8003A3F0
    /* 2ABF0 8003A3F0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2ABF4 8003A3F4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2ABF8 8003A3F8 E7E8000C */  jal        func_8003A39C
    /* 2ABFC 8003A3FC 00000000 */   nop
    /* 2AC00 8003A400 01000224 */  addiu      $v0, $zero, 0x1
    /* 2AC04 8003A404 0A80013C */  lui        $at, %hi(D_800A3928)
    /* 2AC08 8003A408 283922A0 */  sb         $v0, %lo(D_800A3928)($at)
    /* 2AC0C 8003A40C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2AC10 8003A410 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2AC14 8003A414 0800E003 */  jr         $ra
    /* 2AC18 8003A418 00000000 */   nop
endlabel func_8003A3F0
