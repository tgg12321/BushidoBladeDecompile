glabel func_8003C958
    /* 2D158 8003C958 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2D15C 8003C95C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2D160 8003C960 225A000C */  jal        gpu_InitDisplay
    /* 2D164 8003C964 00000000 */   nop
    /* 2D168 8003C968 19000224 */  addiu      $v0, $zero, 0x19
    /* 2D16C 8003C96C 0A80013C */  lui        $at, %hi(D_800A3817)
    /* 2D170 8003C970 173820A0 */  sb         $zero, %lo(D_800A3817)($at)
    /* 2D174 8003C974 0A80013C */  lui        $at, %hi(D_800A3929)
    /* 2D178 8003C978 293920A0 */  sb         $zero, %lo(D_800A3929)($at)
    /* 2D17C 8003C97C 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2D180 8003C980 B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* 2D184 8003C984 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2D188 8003C988 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2D18C 8003C98C 345A000C */  jal        gpu_DisableDisplay
    /* 2D190 8003C990 00000000 */   nop
    /* 2D194 8003C994 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2D198 8003C998 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2D19C 8003C99C 0800E003 */  jr         $ra
    /* 2D1A0 8003C9A0 00000000 */   nop
endlabel func_8003C958
