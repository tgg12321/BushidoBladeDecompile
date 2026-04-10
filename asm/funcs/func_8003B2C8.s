glabel func_8003B2C8
    /* 2BAC8 8003B2C8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2BACC 8003B2CC 1080043C */  lui        $a0, %hi(D_8010277C)
    /* 2BAD0 8003B2D0 7C278424 */  addiu      $a0, $a0, %lo(D_8010277C)
    /* 2BAD4 8003B2D4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2BAD8 8003B2D8 00008290 */  lbu        $v0, 0x0($a0)
    /* 2BADC 8003B2DC 0A80013C */  lui        $at, %hi(D_800A3836)
    /* 2BAE0 8003B2E0 363822A0 */  sb         $v0, %lo(D_800A3836)($at)
    /* 2BAE4 8003B2E4 0A80023C */  lui        $v0, %hi(D_800A3894)
    /* 2BAE8 8003B2E8 9438428C */  lw         $v0, %lo(D_800A3894)($v0)
    /* 2BAEC 8003B2EC 0A80033C */  lui        $v1, %hi(D_800A376A)
    /* 2BAF0 8003B2F0 6A376390 */  lbu        $v1, %lo(D_800A376A)($v1)
    /* 2BAF4 8003B2F4 00004290 */  lbu        $v0, 0x0($v0)
    /* 2BAF8 8003B2F8 21280000 */  addu       $a1, $zero, $zero
    /* 2BAFC 8003B2FC 0A80013C */  lui        $at, %hi(D_800A36C8)
    /* 2BB00 8003B300 C83623A0 */  sb         $v1, %lo(D_800A36C8)($at)
    /* 2BB04 8003B304 0A80013C */  lui        $at, %hi(D_800A376A)
    /* 2BB08 8003B308 6A3720A0 */  sb         $zero, %lo(D_800A376A)($at)
    /* 2BB0C 8003B30C 000082A0 */  sb         $v0, 0x0($a0)
    /* 2BB10 8003B310 8105010C */  jal        player_SetCharId
    /* 2BB14 8003B314 21200000 */   addu      $a0, $zero, $zero
    /* 2BB18 8003B318 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2BB1C 8003B31C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2BB20 8003B320 0800E003 */  jr         $ra
    /* 2BB24 8003B324 00000000 */   nop
endlabel func_8003B2C8
