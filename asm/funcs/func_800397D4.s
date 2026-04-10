glabel func_800397D4
    /* 29FD4 800397D4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 29FD8 800397D8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 29FDC 800397DC 1A5A000C */  jal        gpu_EnableDisplay
    /* 29FE0 800397E0 00000000 */   nop
    /* 29FE4 800397E4 8BF8000C */  jal        func_8003E22C
    /* 29FE8 800397E8 00000000 */   nop
    /* 29FEC 800397EC 86FC000C */  jal        func_8003F218
    /* 29FF0 800397F0 21200000 */   addu      $a0, $zero, $zero
    /* 29FF4 800397F4 B159000C */  jal        disp_CalcFov
    /* 29FF8 800397F8 2D000424 */   addiu     $a0, $zero, 0x2D
    /* 29FFC 800397FC FFFB010C */  jal        func_8007EFFC
    /* 2A000 80039800 21204000 */   addu      $a0, $v0, $zero
    /* 2A004 80039804 21200000 */  addu       $a0, $zero, $zero
    /* 2A008 80039808 A205010C */  jal        func_80041688
    /* 2A00C 8003980C 21280000 */   addu      $a1, $zero, $zero
    /* 2A010 80039810 01000424 */  addiu      $a0, $zero, 0x1
    /* 2A014 80039814 A205010C */  jal        func_80041688
    /* 2A018 80039818 21280000 */   addu      $a1, $zero, $zero
    /* 2A01C 8003981C BD6D000C */  jal        func_8001B6F4
    /* 2A020 80039820 00000000 */   nop
    /* 2A024 80039824 5E84010C */  jal        game_Cleanup
    /* 2A028 80039828 00000000 */   nop
    /* 2A02C 8003982C 05000224 */  addiu      $v0, $zero, 0x5
    /* 2A030 80039830 040780A3 */  sb         $zero, %gp_rel(D_800A37D0)($gp)
    /* 2A034 80039834 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2A038 80039838 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2A03C 8003983C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2A040 80039840 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2A044 80039844 0800E003 */  jr         $ra
    /* 2A048 80039848 00000000 */   nop
endlabel func_800397D4
