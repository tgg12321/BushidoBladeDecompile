glabel func_8003A308
    /* 2AB08 8003A308 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2AB0C 8003A30C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2AB10 8003A310 03000424 */  addiu      $a0, $zero, 0x3
    /* 2AB14 8003A314 01000524 */  addiu      $a1, $zero, 0x1
    /* 2AB18 8003A318 1931020C */  jal        func_8008C464
    /* 2AB1C 8003A31C 21300000 */   addu      $a2, $zero, $zero
    /* 2AB20 8003A320 05004010 */  beqz       $v0, .L8003A338
    /* 2AB24 8003A324 01000224 */   addiu     $v0, $zero, 0x1
    /* 2AB28 8003A328 0A80013C */  lui        $at, %hi(D_800A38A0)
    /* 2AB2C 8003A32C A03822AC */  sw         $v0, %lo(D_800A38A0)($at)
    /* 2AB30 8003A330 D1E80008 */  j          .L8003A344
    /* 2AB34 8003A334 03000424 */   addiu     $a0, $zero, 0x3
  .L8003A338:
    /* 2AB38 8003A338 0A80013C */  lui        $at, %hi(D_800A38A0)
    /* 2AB3C 8003A33C A03820AC */  sw         $zero, %lo(D_800A38A0)($at)
    /* 2AB40 8003A340 03000424 */  addiu      $a0, $zero, 0x3
  .L8003A344:
    /* 2AB44 8003A344 21280000 */  addu       $a1, $zero, $zero
    /* 2AB48 8003A348 1931020C */  jal        func_8008C464
    /* 2AB4C 8003A34C 01000624 */   addiu     $a2, $zero, 0x1
    /* 2AB50 8003A350 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2AB54 8003A354 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2AB58 8003A358 0800E003 */  jr         $ra
    /* 2AB5C 8003A35C 00000000 */   nop
endlabel func_8003A308
