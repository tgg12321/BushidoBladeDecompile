glabel game_FrameInit
    /* 276C0 80036EC0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 276C4 80036EC4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 276C8 80036EC8 9000020C */  jal        func_80080240
    /* 276CC 80036ECC 21200000 */   addu      $a0, $zero, $zero
    /* 276D0 80036ED0 21200000 */  addu       $a0, $zero, $zero
    /* 276D4 80036ED4 21280000 */  addu       $a1, $zero, $zero
    /* 276D8 80036ED8 21300000 */  addu       $a2, $zero, $zero
    /* 276DC 80036EDC CCD7000C */  jal        func_80035F30
    /* 276E0 80036EE0 21380000 */   addu      $a3, $zero, $zero
    /* 276E4 80036EE4 5200020C */  jal        func_80080148
    /* 276E8 80036EE8 00000000 */   nop
    /* 276EC 80036EEC 09000424 */  addiu      $a0, $zero, 0x9
    /* 276F0 80036EF0 E400020C */  jal        func_80080390
    /* 276F4 80036EF4 21280000 */   addu      $a1, $zero, $zero
    /* 276F8 80036EF8 01000224 */  addiu      $v0, $zero, 0x1
    /* 276FC 80036EFC 1080013C */  lui        $at, %hi(D_80101E68)
    /* 27700 80036F00 681E22A4 */  sh         $v0, %lo(D_80101E68)($at)
    /* 27704 80036F04 0B000224 */  addiu      $v0, $zero, 0xB
    /* 27708 80036F08 1080013C */  lui        $at, %hi(D_80101E62)
    /* 2770C 80036F0C 621E22A4 */  sh         $v0, %lo(D_80101E62)($at)
    /* 27710 80036F10 1080013C */  lui        $at, %hi(D_80101E5C)
    /* 27714 80036F14 5C1E20AC */  sw         $zero, %lo(D_80101E5C)($at)
    /* 27718 80036F18 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2771C 80036F1C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 27720 80036F20 0800E003 */  jr         $ra
    /* 27724 80036F24 00000000 */   nop
endlabel game_FrameInit
