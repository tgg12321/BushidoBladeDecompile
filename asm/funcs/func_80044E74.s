glabel func_80044E74
    /* 35674 80044E74 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 35678 80044E78 1000B0AF */  sw         $s0, 0x10($sp)
    /* 3567C 80044E7C 21808000 */  addu       $s0, $a0, $zero
    /* 35680 80044E80 1400B1AF */  sw         $s1, 0x14($sp)
    /* 35684 80044E84 1800BFAF */  sw         $ra, 0x18($sp)
    /* 35688 80044E88 D0DB000C */  jal        game_FrameLoop
    /* 3568C 80044E8C 2188A000 */   addu      $s1, $a1, $zero
    /* 35690 80044E90 80801000 */  sll        $s0, $s0, 2
    /* 35694 80044E94 21200000 */  addu       $a0, $zero, $zero
    /* 35698 80044E98 0980013C */  lui        $at, %hi(D_800963EC)
    /* 3569C 80044E9C 21083000 */  addu       $at, $at, $s0
    /* 356A0 80044EA0 EC632684 */  lh         $a2, %lo(D_800963EC)($at)
    /* 356A4 80044EA4 0980013C */  lui        $at, %hi(D_800963EE)
    /* 356A8 80044EA8 21083000 */  addu       $at, $at, $s0
    /* 356AC 80044EAC EE632784 */  lh         $a3, %lo(D_800963EE)($at)
    /* 356B0 80044EB0 8DDB000C */  jal        func_80036E34
    /* 356B4 80044EB4 21282002 */   addu      $a1, $s1, $zero
    /* 356B8 80044EB8 D0DB000C */  jal        game_FrameLoop
    /* 356BC 80044EBC 00000000 */   nop
    /* 356C0 80044EC0 1800BF8F */  lw         $ra, 0x18($sp)
    /* 356C4 80044EC4 1400B18F */  lw         $s1, 0x14($sp)
    /* 356C8 80044EC8 1000B08F */  lw         $s0, 0x10($sp)
    /* 356CC 80044ECC 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 356D0 80044ED0 0800E003 */  jr         $ra
    /* 356D4 80044ED4 00000000 */   nop
endlabel func_80044E74
