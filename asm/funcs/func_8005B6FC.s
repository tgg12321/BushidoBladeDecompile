glabel func_8005B6FC
    /* 4BEFC 8005B6FC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 4BF00 8005B700 1000BFAF */  sw         $ra, 0x10($sp)
    /* 4BF04 8005B704 D91F020C */  jal        func_80087F64
    /* 4BF08 8005B708 01000424 */   addiu     $a0, $zero, 0x1
    /* 4BF0C 8005B70C 0F80013C */  lui        $at, %hi(D_800EFC3C)
    /* 4BF10 8005B710 3CFC20AC */  sw         $zero, %lo(D_800EFC3C)($at)
    /* 4BF14 8005B714 0F80013C */  lui        $at, %hi(D_800EFB3C)
    /* 4BF18 8005B718 3CFB20AC */  sw         $zero, %lo(D_800EFB3C)($at)
    /* 4BF1C 8005B71C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 4BF20 8005B720 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 4BF24 8005B724 0800E003 */  jr         $ra
    /* 4BF28 8005B728 00000000 */   nop
endlabel func_8005B6FC
