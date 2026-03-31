glabel func_80047570
    /* 37D70 80047570 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 37D74 80047574 0F80043C */  lui        $a0, %hi(D_800EEDF0)
    /* 37D78 80047578 F0ED8424 */  addiu      $a0, $a0, %lo(D_800EEDF0)
    /* 37D7C 8004757C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 37D80 80047580 B01C010C */  jal        camera_InitRotation
    /* 37D84 80047584 00000000 */   nop
    /* 37D88 80047588 04000224 */  addiu      $v0, $zero, 0x4
    /* 37D8C 8004758C 0F80013C */  lui        $at, %hi(D_800EEDF8)
    /* 37D90 80047590 F8ED22A4 */  sh         $v0, %lo(D_800EEDF8)($at)
    /* 37D94 80047594 1000BF8F */  lw         $ra, 0x10($sp)
    /* 37D98 80047598 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 37D9C 8004759C 0800E003 */  jr         $ra
    /* 37DA0 800475A0 00000000 */   nop
endlabel func_80047570
