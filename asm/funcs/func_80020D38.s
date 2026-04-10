glabel func_80020D38
    /* 11538 80020D38 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 1153C 80020D3C 0A80033C */  lui        $v1, %hi(D_800A38C6)
    /* 11540 80020D40 C6386394 */  lhu        $v1, %lo(D_800A38C6)($v1)
    /* 11544 80020D44 FFFF0234 */  ori        $v0, $zero, 0xFFFF
    /* 11548 80020D48 03006214 */  bne        $v1, $v0, .L80020D58
    /* 1154C 80020D4C 1000BFAF */   sw        $ra, 0x10($sp)
    /* 11550 80020D50 6214010C */  jal        seq_Reset
    /* 11554 80020D54 00000000 */   nop
  .L80020D58:
    /* 11558 80020D58 0A80013C */  lui        $at, %hi(D_800A38C6)
    /* 1155C 80020D5C C63820A4 */  sh         $zero, %lo(D_800A38C6)($at)
    /* 11560 80020D60 1000BF8F */  lw         $ra, 0x10($sp)
    /* 11564 80020D64 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 11568 80020D68 0800E003 */  jr         $ra
    /* 1156C 80020D6C 00000000 */   nop
endlabel func_80020D38
