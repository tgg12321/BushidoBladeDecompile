glabel obj_InitTask
    /* 4C1C4 8005B9C4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 4C1C8 8005B9C8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 4C1CC 8005B9CC 3416020C */  jal        func_800858D0
    /* 4C1D0 8005B9D0 21200000 */   addu      $a0, $zero, $zero
    /* 4C1D4 8005B9D4 D91F020C */  jal        func_80087F64
    /* 4C1D8 8005B9D8 09000424 */   addiu     $a0, $zero, 0x9
    /* 4C1DC 8005B9DC 0F80013C */  lui        $at, %hi(D_800EFC5C)
    /* 4C1E0 8005B9E0 5CFC20AC */  sw         $zero, %lo(D_800EFC5C)($at)
    /* 4C1E4 8005B9E4 0F80013C */  lui        $at, %hi(D_800EFB5C)
    /* 4C1E8 8005B9E8 5CFB20AC */  sw         $zero, %lo(D_800EFB5C)($at)
    /* 4C1EC 8005B9EC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 4C1F0 8005B9F0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 4C1F4 8005B9F4 0800E003 */  jr         $ra
    /* 4C1F8 8005B9F8 00000000 */   nop
endlabel obj_InitTask
