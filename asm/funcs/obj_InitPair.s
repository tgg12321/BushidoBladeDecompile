glabel obj_InitPair
    /* 4C068 8005B868 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 4C06C 8005B86C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 4C070 8005B870 3416020C */  jal        func_800858D0
    /* 4C074 8005B874 21200000 */   addu      $a0, $zero, $zero
    /* 4C078 8005B878 D91F020C */  jal        func_80087F64
    /* 4C07C 8005B87C 08000424 */   addiu     $a0, $zero, 0x8
    /* 4C080 8005B880 0F80013C */  lui        $at, %hi(D_800EFC58)
    /* 4C084 8005B884 58FC20AC */  sw         $zero, %lo(D_800EFC58)($at)
    /* 4C088 8005B888 0F80013C */  lui        $at, %hi(D_800EFB58)
    /* 4C08C 8005B88C 58FB20AC */  sw         $zero, %lo(D_800EFB58)($at)
    /* 4C090 8005B890 D91F020C */  jal        func_80087F64
    /* 4C094 8005B894 04000424 */   addiu     $a0, $zero, 0x4
    /* 4C098 8005B898 0F80013C */  lui        $at, %hi(D_800EFC48)
    /* 4C09C 8005B89C 48FC20AC */  sw         $zero, %lo(D_800EFC48)($at)
    /* 4C0A0 8005B8A0 0F80013C */  lui        $at, %hi(D_800EFB48)
    /* 4C0A4 8005B8A4 48FB20AC */  sw         $zero, %lo(D_800EFB48)($at)
    /* 4C0A8 8005B8A8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 4C0AC 8005B8AC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 4C0B0 8005B8B0 0800E003 */  jr         $ra
    /* 4C0B4 8005B8B4 00000000 */   nop
endlabel obj_InitPair
