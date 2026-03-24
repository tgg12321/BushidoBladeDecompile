glabel func_8007C0B0
    /* 6C8B0 8007C0B0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C8B4 8007C0B4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C8B8 8007C0B8 21808000 */  addu       $s0, $a0, $zero
    /* 6C8BC 8007C0BC 0A80053C */  lui        $a1, %hi(D_8009BEE0)
    /* 6C8C0 8007C0C0 E0BEA524 */  addiu      $a1, $a1, %lo(D_8009BEE0)
    /* 6C8C4 8007C0C4 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6C8C8 8007C0C8 48E4010C */  jal        func_80079120
    /* 6C8CC 8007C0CC 14000624 */   addiu     $a2, $zero, 0x14
    /* 6C8D0 8007C0D0 21100002 */  addu       $v0, $s0, $zero
    /* 6C8D4 8007C0D4 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6C8D8 8007C0D8 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C8DC 8007C0DC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6C8E0 8007C0E0 0800E003 */  jr         $ra
    /* 6C8E4 8007C0E4 00000000 */   nop
endlabel func_8007C0B0
