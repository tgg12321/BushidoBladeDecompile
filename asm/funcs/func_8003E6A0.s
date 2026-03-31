glabel func_8003E6A0
    /* 2EEA0 8003E6A0 2138A000 */  addu       $a3, $a1, $zero
    /* 2EEA4 8003E6A4 21308000 */  addu       $a2, $a0, $zero
    /* 2EEA8 8003E6A8 1080043C */  lui        $a0, %hi(D_80101E3C)
    /* 2EEAC 8003E6AC 3C1E848C */  lw         $a0, %lo(D_80101E3C)($a0)
    /* 2EEB0 8003E6B0 1080053C */  lui        $a1, %hi(D_80101E44)
    /* 2EEB4 8003E6B4 441EA58C */  lw         $a1, %lo(D_80101E44)($a1)
    /* 2EEB8 8003E6B8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2EEBC 8003E6BC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2EEC0 8003E6C0 B6F8000C */  jal        replay_camera_get_attack_number
    /* 2EEC4 8003E6C4 00000000 */   nop
    /* 2EEC8 8003E6C8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2EECC 8003E6CC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2EED0 8003E6D0 0800E003 */  jr         $ra
    /* 2EED4 8003E6D4 00000000 */   nop
endlabel func_8003E6A0
