glabel func_8003B328
    /* 2BB28 8003B328 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2BB2C 8003B32C 1080033C */  lui        $v1, %hi(D_8010277C)
    /* 2BB30 8003B330 7C276324 */  addiu      $v1, $v1, %lo(D_8010277C)
    /* 2BB34 8003B334 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2BB38 8003B338 00006690 */  lbu        $a2, 0x0($v1)
    /* 2BB3C 8003B33C 0A80073C */  lui        $a3, %hi(D_800A376A)
    /* 2BB40 8003B340 6A37E790 */  lbu        $a3, %lo(D_800A376A)($a3)
    /* 2BB44 8003B344 0A80083C */  lui        $t0, %hi(D_800A3836)
    /* 2BB48 8003B348 36380891 */  lbu        $t0, %lo(D_800A3836)($t0)
    /* 2BB4C 8003B34C 0A80053C */  lui        $a1, %hi(D_800A36C8)
    /* 2BB50 8003B350 C836A590 */  lbu        $a1, %lo(D_800A36C8)($a1)
    /* 2BB54 8003B354 FF000224 */  addiu      $v0, $zero, 0xFF
    /* 2BB58 8003B358 0A80013C */  lui        $at, %hi(D_800A3836)
    /* 2BB5C 8003B35C 363822A0 */  sb         $v0, %lo(D_800A3836)($at)
    /* 2BB60 8003B360 0A80013C */  lui        $at, %hi(D_800A3915)
    /* 2BB64 8003B364 153926A0 */  sb         $a2, %lo(D_800A3915)($at)
    /* 2BB68 8003B368 0A80013C */  lui        $at, %hi(D_800A36F4)
    /* 2BB6C 8003B36C F43627A0 */  sb         $a3, %lo(D_800A36F4)($at)
    /* 2BB70 8003B370 000068A0 */  sb         $t0, 0x0($v1)
    /* 2BB74 8003B374 0A80013C */  lui        $at, %hi(D_800A376A)
    /* 2BB78 8003B378 6A3725A0 */  sb         $a1, %lo(D_800A376A)($at)
    /* 2BB7C 8003B37C 8105010C */  jal        func_80041604
    /* 2BB80 8003B380 21200000 */   addu      $a0, $zero, $zero
    /* 2BB84 8003B384 1080043C */  lui        $a0, %hi(D_80101EC8)
    /* 2BB88 8003B388 C81E8424 */  addiu      $a0, $a0, %lo(D_80101EC8)
    /* 2BB8C 8003B38C 5A89000C */  jal        func_80022568
    /* 2BB90 8003B390 00000000 */   nop
    /* 2BB94 8003B394 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2BB98 8003B398 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2BB9C 8003B39C 0800E003 */  jr         $ra
    /* 2BBA0 8003B3A0 00000000 */   nop
endlabel func_8003B328
