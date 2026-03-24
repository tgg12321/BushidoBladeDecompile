glabel func_8005B6AC
    /* 4BEAC 8005B6AC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 4BEB0 8005B6B0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 4BEB4 8005B6B4 3416020C */  jal        func_800858D0
    /* 4BEB8 8005B6B8 21200000 */   addu      $a0, $zero, $zero
    /* 4BEBC 8005B6BC D91F020C */  jal        func_80087F64
    /* 4BEC0 8005B6C0 02000424 */   addiu     $a0, $zero, 0x2
    /* 4BEC4 8005B6C4 0F80013C */  lui        $at, %hi(D_800EFC40)
    /* 4BEC8 8005B6C8 40FC20AC */  sw         $zero, %lo(D_800EFC40)($at)
    /* 4BECC 8005B6CC 0F80013C */  lui        $at, %hi(D_800EFB40)
    /* 4BED0 8005B6D0 40FB20AC */  sw         $zero, %lo(D_800EFB40)($at)
    /* 4BED4 8005B6D4 D91F020C */  jal        func_80087F64
    /* 4BED8 8005B6D8 05000424 */   addiu     $a0, $zero, 0x5
    /* 4BEDC 8005B6DC 0F80013C */  lui        $at, %hi(D_800EFC4C)
    /* 4BEE0 8005B6E0 4CFC20AC */  sw         $zero, %lo(D_800EFC4C)($at)
    /* 4BEE4 8005B6E4 0F80013C */  lui        $at, %hi(D_800EFB4C)
    /* 4BEE8 8005B6E8 4CFB20AC */  sw         $zero, %lo(D_800EFB4C)($at)
    /* 4BEEC 8005B6EC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 4BEF0 8005B6F0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 4BEF4 8005B6F4 0800E003 */  jr         $ra
    /* 4BEF8 8005B6F8 00000000 */   nop
endlabel func_8005B6AC
