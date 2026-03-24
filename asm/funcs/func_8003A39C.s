glabel func_8003A39C
    /* 2AB9C 8003A39C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2ABA0 8003A3A0 02000424 */  addiu      $a0, $zero, 0x2
    /* 2ABA4 8003A3A4 21280000 */  addu       $a1, $zero, $zero
    /* 2ABA8 8003A3A8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2ABAC 8003A3AC 400180A3 */  sb         $zero, %gp_rel(D_800A320C)($gp)
    /* 2ABB0 8003A3B0 640680AF */  sw         $zero, %gp_rel(D_800A3730)($gp)
    /* 2ABB4 8003A3B4 1931020C */  jal        func_8008C464
    /* 2ABB8 8003A3B8 21300000 */   addu      $a2, $zero, $zero
    /* 2ABBC 8003A3BC 01000424 */  addiu      $a0, $zero, 0x1
    /* 2ABC0 8003A3C0 01000524 */  addiu      $a1, $zero, 0x1
    /* 2ABC4 8003A3C4 1931020C */  jal        func_8008C464
    /* 2ABC8 8003A3C8 21300000 */   addu      $a2, $zero, $zero
    /* 2ABCC 8003A3CC 99E8000C */  jal        func_8003A264
    /* 2ABD0 8003A3D0 00000000 */   nop
    /* 2ABD4 8003A3D4 08000224 */  addiu      $v0, $zero, 0x8
    /* 2ABD8 8003A3D8 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2ABDC 8003A3DC 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2ABE0 8003A3E0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2ABE4 8003A3E4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2ABE8 8003A3E8 0800E003 */  jr         $ra
    /* 2ABEC 8003A3EC 00000000 */   nop
endlabel func_8003A39C
