glabel func_80044ED8
    /* 356D8 80044ED8 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 356DC 80044EDC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 356E0 80044EE0 21808000 */  addu       $s0, $a0, $zero
    /* 356E4 80044EE4 1400B1AF */  sw         $s1, 0x14($sp)
    /* 356E8 80044EE8 2188A000 */  addu       $s1, $a1, $zero
    /* 356EC 80044EEC 1F00022A */  slti       $v0, $s0, 0x1F
    /* 356F0 80044EF0 02004014 */  bnez       $v0, .L80044EFC
    /* 356F4 80044EF4 1800BFAF */   sw        $ra, 0x18($sp)
    /* 356F8 80044EF8 E5FF1026 */  addiu      $s0, $s0, -0x1B
  .L80044EFC:
    /* 356FC 80044EFC 21200002 */  addu       $a0, $s0, $zero
    /* 35700 80044F00 3D14010C */  jal        func_800450F4
    /* 35704 80044F04 21282002 */   addu      $a1, $s1, $zero
    /* 35708 80044F08 03004014 */  bnez       $v0, .L80044F18
    /* 3570C 80044F0C 21200002 */   addu      $a0, $s0, $zero
    /* 35710 80044F10 9D13010C */  jal        func_80044E74
    /* 35714 80044F14 21282002 */   addu      $a1, $s1, $zero
  .L80044F18:
    /* 35718 80044F18 1800BF8F */  lw         $ra, 0x18($sp)
    /* 3571C 80044F1C 1400B18F */  lw         $s1, 0x14($sp)
    /* 35720 80044F20 1000B08F */  lw         $s0, 0x10($sp)
    /* 35724 80044F24 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 35728 80044F28 0800E003 */  jr         $ra
    /* 3572C 80044F2C 00000000 */   nop
endlabel func_80044ED8
