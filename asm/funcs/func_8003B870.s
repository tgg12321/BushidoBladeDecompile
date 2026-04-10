glabel func_8003B870
    /* 2C070 8003B870 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2C074 8003B874 0A80053C */  lui        $a1, %hi(D_800A376A)
    /* 2C078 8003B878 6A37A590 */  lbu        $a1, %lo(D_800A376A)($a1)
    /* 2C07C 8003B87C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2C080 8003B880 8105010C */  jal        player_SetCharId
    /* 2C084 8003B884 21200000 */   addu      $a0, $zero, $zero
    /* 2C088 8003B888 01000424 */  addiu      $a0, $zero, 0x1
    /* 2C08C 8003B88C 8105010C */  jal        player_SetCharId
    /* 2C090 8003B890 21280000 */   addu      $a1, $zero, $zero
    /* 2C094 8003B894 6B6D010C */  jal        obj_InitChars
    /* 2C098 8003B898 00000000 */   nop
    /* 2C09C 8003B89C 225A000C */  jal        gpu_InitDisplay
    /* 2C0A0 8003B8A0 00000000 */   nop
    /* 2C0A4 8003B8A4 01000424 */  addiu      $a0, $zero, 0x1
    /* 2C0A8 8003B8A8 21280000 */  addu       $a1, $zero, $zero
    /* 2C0AC 8003B8AC 21300000 */  addu       $a2, $zero, $zero
    /* 2C0B0 8003B8B0 DA59000C */  jal        disp_SetFramebufferMode
    /* 2C0B4 8003B8B4 21380000 */   addu      $a3, $zero, $zero
    /* 2C0B8 8003B8B8 17000224 */  addiu      $v0, $zero, 0x17
    /* 2C0BC 8003B8BC 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2C0C0 8003B8C0 B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* 2C0C4 8003B8C4 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2C0C8 8003B8C8 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2C0CC 8003B8CC 345A000C */  jal        gpu_DisableDisplay
    /* 2C0D0 8003B8D0 00000000 */   nop
    /* 2C0D4 8003B8D4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2C0D8 8003B8D8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2C0DC 8003B8DC 0800E003 */  jr         $ra
    /* 2C0E0 8003B8E0 00000000 */   nop
endlabel func_8003B870
