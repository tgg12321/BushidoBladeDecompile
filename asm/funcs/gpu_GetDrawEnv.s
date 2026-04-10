glabel gpu_GetDrawEnv
    /* 6C3D0 8007BBD0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C3D4 8007BBD4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C3D8 8007BBD8 21808000 */  addu       $s0, $a0, $zero
    /* 6C3DC 8007BBDC 0A80053C */  lui        $a1, %hi(D_8009BE84)
    /* 6C3E0 8007BBE0 84BEA524 */  addiu      $a1, $a1, %lo(D_8009BE84)
    /* 6C3E4 8007BBE4 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6C3E8 8007BBE8 48E4010C */  jal        bb2_memcpy
    /* 6C3EC 8007BBEC 5C000624 */   addiu     $a2, $zero, 0x5C
    /* 6C3F0 8007BBF0 21100002 */  addu       $v0, $s0, $zero
    /* 6C3F4 8007BBF4 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6C3F8 8007BBF8 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C3FC 8007BBFC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6C400 8007BC00 0800E003 */  jr         $ra
    /* 6C404 8007BC04 00000000 */   nop
endlabel gpu_GetDrawEnv
