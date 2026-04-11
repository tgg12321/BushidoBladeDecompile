glabel func_80035438
    /* 25C38 80035438 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 25C3C 8003543C 01000224 */  addiu      $v0, $zero, 0x1
    /* 25C40 80035440 1000BFAF */  sw         $ra, 0x10($sp)
    /* 25C44 80035444 740682A3 */  sb         $v0, %gp_rel(D_800A3740)($gp)
    /* 25C48 80035448 A0D4000C */  jal        single_game_SetWazaData
    /* 25C4C 8003544C 00000000 */   nop
    /* 25C50 80035450 1080033C */  lui        $v1, %hi(D_80106A54)
    /* 25C54 80035454 546A6390 */  lbu        $v1, %lo(D_80106A54)($v1)
    /* 25C58 80035458 3F000224 */  addiu      $v0, $zero, 0x3F
    /* 25C5C 8003545C 02006214 */  bne        $v1, $v0, .L80035468
    /* 25C60 80035460 F7000424 */   addiu     $a0, $zero, 0xF7
    /* 25C64 80035464 FF000424 */  addiu      $a0, $zero, 0xFF
  .L80035468:
    /* 25C68 80035468 B3A3010C */  jal        func_80068ECC
    /* 25C6C 8003546C 00000000 */   nop
    /* 25C70 80035470 1000BF8F */  lw         $ra, 0x10($sp)
    /* 25C74 80035474 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 25C78 80035478 0800E003 */  jr         $ra
    /* 25C7C 8003547C 00000000 */   nop
endlabel func_80035438
