glabel func_800784E4
    /* 68CE4 800784E4 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 68CE8 800784E8 1400B1AF */  sw         $s1, 0x14($sp)
    /* 68CEC 800784EC 21888000 */  addu       $s1, $a0, $zero
    /* 68CF0 800784F0 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 68CF4 800784F4 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 68CF8 800784F8 08100524 */  addiu      $a1, $zero, 0x1008
    /* 68CFC 800784FC 1800BFAF */  sw         $ra, 0x18($sp)
    /* 68D00 80078500 11EE010C */  jal        func_8007B844
    /* 68D04 80078504 1000B0AF */   sw        $s0, 0x10($sp)
    /* 68D08 80078508 32000424 */  addiu      $a0, $zero, 0x32
    /* 68D0C 8007850C 58003026 */  addiu      $s0, $s1, 0x58
    /* 68D10 80078510 280591AF */  sw         $s1, %gp_rel(D_800A35F4)($gp)
    /* 68D14 80078514 2C0590AF */  sw         $s0, %gp_rel(D_800A35F8)($gp)
    /* 68D18 80078518 54BA010C */  jal        func_8006E950
    /* 68D1C 8007851C 21280002 */   addu      $a1, $s0, $zero
    /* 68D20 80078520 44DF010C */  jal        func_80077D10
    /* 68D24 80078524 21200002 */   addu      $a0, $s0, $zero
    /* 68D28 80078528 2805858F */  lw         $a1, %gp_rel(D_800A35F4)($gp)
    /* 68D2C 8007852C 27B9010C */  jal        func_8006E49C
    /* 68D30 80078530 21204000 */   addu      $a0, $v0, $zero
    /* 68D34 80078534 01000224 */  addiu      $v0, $zero, 0x1
    /* 68D38 80078538 300580AF */  sw         $zero, %gp_rel(D_800A35FC)($gp)
    /* 68D3C 8007853C 240580AF */  sw         $zero, %gp_rel(D_800A35F0)($gp)
    /* 68D40 80078540 340580AF */  sw         $zero, %gp_rel(D_800A3600)($gp)
    /* 68D44 80078544 1800BF8F */  lw         $ra, 0x18($sp)
    /* 68D48 80078548 1400B18F */  lw         $s1, 0x14($sp)
    /* 68D4C 8007854C 1000B08F */  lw         $s0, 0x10($sp)
    /* 68D50 80078550 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 68D54 80078554 0800E003 */  jr         $ra
    /* 68D58 80078558 00000000 */   nop
endlabel func_800784E4
