glabel player_Destroy
    /* 31DC4 800415C4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 31DC8 800415C8 1000B0AF */  sw         $s0, 0x10($sp)
    /* 31DCC 800415CC 1400BFAF */  sw         $ra, 0x14($sp)
    /* 31DD0 800415D0 5B00010C */  jal        func_8004016C
    /* 31DD4 800415D4 21808000 */   addu      $s0, $a0, $zero
    /* 31DD8 800415D8 9416010C */  jal        func_80045A50
    /* 31DDC 800415DC 21200002 */   addu      $a0, $s0, $zero
    /* 31DE0 800415E0 80801000 */  sll        $s0, $s0, 2
    /* 31DE4 800415E4 0B80013C */  lui        $at, %hi(D_800A9A10)
    /* 31DE8 800415E8 21083000 */  addu       $at, $at, $s0
    /* 31DEC 800415EC 109A20AC */  sw         $zero, %lo(D_800A9A10)($at)
    /* 31DF0 800415F0 1400BF8F */  lw         $ra, 0x14($sp)
    /* 31DF4 800415F4 1000B08F */  lw         $s0, 0x10($sp)
    /* 31DF8 800415F8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 31DFC 800415FC 0800E003 */  jr         $ra
    /* 31E00 80041600 00000000 */   nop
endlabel player_Destroy
