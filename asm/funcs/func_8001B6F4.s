glabel func_8001B6F4
    /* BEF4 8001B6F4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* BEF8 8001B6F8 21200000 */  addu       $a0, $zero, $zero
    /* BEFC 8001B6FC 1000BFAF */  sw         $ra, 0x10($sp)
    /* BF00 8001B700 A205010C */  jal        func_80041688
    /* BF04 8001B704 21280000 */   addu      $a1, $zero, $zero
    /* BF08 8001B708 01000424 */  addiu      $a0, $zero, 0x1
    /* BF0C 8001B70C A205010C */  jal        func_80041688
    /* BF10 8001B710 21280000 */   addu      $a1, $zero, $zero
    /* BF14 8001B714 01000224 */  addiu      $v0, $zero, 0x1
    /* BF18 8001B718 0A80013C */  lui        $at, %hi(D_800A36FA)
    /* BF1C 8001B71C FA3622A0 */  sb         $v0, %lo(D_800A36FA)($at)
    /* BF20 8001B720 0F80013C */  lui        $at, %hi(D_800F6627)
    /* BF24 8001B724 276620A0 */  sb         $zero, %lo(D_800F6627)($at)
    /* BF28 8001B728 0F80013C */  lui        $at, %hi(D_800F5347)
    /* BF2C 8001B72C 475320A0 */  sb         $zero, %lo(D_800F5347)($at)
    /* BF30 8001B730 79FC000C */  jal        game_SetControllerPorts
    /* BF34 8001B734 21200000 */   addu      $a0, $zero, $zero
    /* BF38 8001B738 1000BF8F */  lw         $ra, 0x10($sp)
    /* BF3C 8001B73C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* BF40 8001B740 0800E003 */  jr         $ra
    /* BF44 8001B744 00000000 */   nop
endlabel func_8001B6F4
