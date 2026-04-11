glabel func_8005B7C4
    /* 4BFC4 8005B7C4 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 4BFC8 8005B7C8 1400B1AF */  sw         $s1, 0x14($sp)
    /* 4BFCC 8005B7CC 21888000 */  addu       $s1, $a0, $zero
    /* 4BFD0 8005B7D0 21200000 */  addu       $a0, $zero, $zero
    /* 4BFD4 8005B7D4 1800BFAF */  sw         $ra, 0x18($sp)
    /* 4BFD8 8005B7D8 3416020C */  jal        title_mv_exec2
    /* 4BFDC 8005B7DC 1000B0AF */   sw        $s0, 0x10($sp)
    /* 4BFE0 8005B7E0 0180043C */  lui        $a0, %hi(D_800158B4)
    /* 4BFE4 8005B7E4 B4588424 */  addiu      $a0, $a0, %lo(D_800158B4)
    /* 4BFE8 8005B7E8 82E4010C */  jal        debug_printf
    /* 4BFEC 8005B7EC 21282002 */   addu      $a1, $s1, $zero
    /* 4BFF0 8005B7F0 D0DB000C */  jal        game_FrameLoop
    /* 4BFF4 8005B7F4 00000000 */   nop
    /* 4BFF8 8005B7F8 02000424 */  addiu      $a0, $zero, 0x2
    /* 4BFFC 8005B7FC AADB000C */  jal        func_80036EA8
    /* 4C000 8005B800 01000524 */   addiu     $a1, $zero, 0x1
    /* 4C004 8005B804 21804000 */  addu       $s0, $v0, $zero
    /* 4C008 8005B808 21200002 */  addu       $a0, $s0, $zero
    /* 4C00C 8005B80C 66DB000C */  jal        replay_camera_Init
    /* 4C010 8005B810 21282002 */   addu      $a1, $s1, $zero
    /* 4C014 8005B814 CADB000C */  jal        func_80036F28
    /* 4C018 8005B818 21200002 */   addu      $a0, $s0, $zero
    /* 4C01C 8005B81C D0DB000C */  jal        game_FrameLoop
    /* 4C020 8005B820 21804000 */   addu      $s0, $v0, $zero
    /* 4C024 8005B824 21202002 */  addu       $a0, $s1, $zero
    /* 4C028 8005B828 21280000 */  addu       $a1, $zero, $zero
    /* 4C02C 8005B82C 10100224 */  addiu      $v0, $zero, 0x1010
    /* 4C030 8005B830 3C0380AF */  sw         $zero, %gp_rel(D_800A3408)($gp)
    /* 4C034 8005B834 400382AF */  sw         $v0, %gp_rel(D_800A340C)($gp)
    /* 4C038 8005B838 380382AF */  sw         $v0, %gp_rel(D_800A3404)($gp)
    /* 4C03C 8005B83C AA70010C */  jal        func_8005C2A8
    /* 4C040 8005B840 21309000 */   addu      $a2, $a0, $s0
    /* 4C044 8005B844 3803838F */  lw         $v1, %gp_rel(D_800A3404)($gp)
    /* 4C048 8005B848 00000000 */  nop
    /* 4C04C 8005B84C 400383AF */  sw         $v1, %gp_rel(D_800A340C)($gp)
    /* 4C050 8005B850 1800BF8F */  lw         $ra, 0x18($sp)
    /* 4C054 8005B854 1400B18F */  lw         $s1, 0x14($sp)
    /* 4C058 8005B858 1000B08F */  lw         $s0, 0x10($sp)
    /* 4C05C 8005B85C 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 4C060 8005B860 0800E003 */  jr         $ra
    /* 4C064 8005B864 00000000 */   nop
endlabel func_8005B7C4
