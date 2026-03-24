glabel func_80078C9C
    /* 6949C 80078C9C D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 694A0 80078CA0 1000B0AF */  sw         $s0, 0x10($sp)
    /* 694A4 80078CA4 21808000 */  addu       $s0, $a0, $zero
    /* 694A8 80078CA8 1400B1AF */  sw         $s1, 0x14($sp)
    /* 694AC 80078CAC 2188A000 */  addu       $s1, $a1, $zero
    /* 694B0 80078CB0 1800B2AF */  sw         $s2, 0x18($sp)
    /* 694B4 80078CB4 2190C000 */  addu       $s2, $a2, $zero
    /* 694B8 80078CB8 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 694BC 80078CBC 2000BFAF */  sw         $ra, 0x20($sp)
    /* 694C0 80078CC0 30E4010C */  jal        func_800790C0
    /* 694C4 80078CC4 2198E000 */   addu      $s3, $a3, $zero
    /* 694C8 80078CC8 6EE2010C */  jal        func_800789B8
    /* 694CC 80078CCC 00000000 */   nop
    /* 694D0 80078CD0 E2E3010C */  jal        func_80078F88
    /* 694D4 80078CD4 00000000 */   nop
    /* 694D8 80078CD8 72E2010C */  jal        func_800789C8
    /* 694DC 80078CDC 00000000 */   nop
    /* 694E0 80078CE0 96E2010C */  jal        func_80078A58
    /* 694E4 80078CE4 21200000 */   addu      $a0, $zero, $zero
    /* 694E8 80078CE8 68E3010C */  jal        func_80078DA0
    /* 694EC 80078CEC 00000000 */   nop
    /* 694F0 80078CF0 21200002 */  addu       $a0, $s0, $zero
    /* 694F4 80078CF4 21282002 */  addu       $a1, $s1, $zero
    /* 694F8 80078CF8 21304002 */  addu       $a2, $s2, $zero
    /* 694FC 80078CFC C0E3010C */  jal        func_80078F00
    /* 69500 80078D00 21386002 */   addu      $a3, $s3, $zero
    /* 69504 80078D04 0AE4010C */  jal        func_80079028
    /* 69508 80078D08 00000000 */   nop
    /* 6950C 80078D0C 01000224 */  addiu      $v0, $zero, 0x1
    /* 69510 80078D10 0A80013C */  lui        $at, %hi(D_8009BD80)
    /* 69514 80078D14 80BD22AC */  sw         $v0, %lo(D_8009BD80)($at)
    /* 69518 80078D18 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6951C 80078D1C 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 69520 80078D20 1800B28F */  lw         $s2, 0x18($sp)
    /* 69524 80078D24 1400B18F */  lw         $s1, 0x14($sp)
    /* 69528 80078D28 1000B08F */  lw         $s0, 0x10($sp)
    /* 6952C 80078D2C 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 69530 80078D30 0800E003 */  jr         $ra
    /* 69534 80078D34 00000000 */   nop
endlabel func_80078C9C
