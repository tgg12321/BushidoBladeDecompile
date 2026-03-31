glabel func_80033898
    /* 24098 80033898 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2409C 8003389C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 240A0 800338A0 1A5A000C */  jal        gpu_EnableDisplay
    /* 240A4 800338A4 00000000 */   nop
    /* 240A8 800338A8 03000224 */  addiu      $v0, $zero, 0x3
    /* 240AC 800338AC 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 240B0 800338B0 B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* 240B4 800338B4 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 240B8 800338B8 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 240BC 800338BC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 240C0 800338C0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 240C4 800338C4 0800E003 */  jr         $ra
    /* 240C8 800338C8 00000000 */   nop
endlabel func_80033898
