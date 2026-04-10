glabel stage_ApplyLighting
    /* 2FDCC 8003F5CC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2FDD0 8003F5D0 0B80043C */  lui        $a0, %hi(D_800A93B0)
    /* 2FDD4 8003F5D4 B093848C */  lw         $a0, %lo(D_800A93B0)($a0)
    /* 2FDD8 8003F5D8 0B80053C */  lui        $a1, %hi(D_800A93BC)
    /* 2FDDC 8003F5DC BC93A58C */  lw         $a1, %lo(D_800A93BC)($a1)
    /* 2FDE0 8003F5E0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2FDE4 8003F5E4 E65F000C */  jal        sys_StubEmpty3
    /* 2FDE8 8003F5E8 21300000 */   addu      $a2, $zero, $zero
    /* 2FDEC 8003F5EC 0B80043C */  lui        $a0, %hi(D_800A93B4)
    /* 2FDF0 8003F5F0 B493848C */  lw         $a0, %lo(D_800A93B4)($a0)
    /* 2FDF4 8003F5F4 0B80053C */  lui        $a1, %hi(D_800A93C0)
    /* 2FDF8 8003F5F8 C093A58C */  lw         $a1, %lo(D_800A93C0)($a1)
    /* 2FDFC 8003F5FC E65F000C */  jal        sys_StubEmpty3
    /* 2FE00 8003F600 01000624 */   addiu     $a2, $zero, 0x1
    /* 2FE04 8003F604 0B80043C */  lui        $a0, %hi(D_800A93B8)
    /* 2FE08 8003F608 B893848C */  lw         $a0, %lo(D_800A93B8)($a0)
    /* 2FE0C 8003F60C 0B80053C */  lui        $a1, %hi(D_800A93C4)
    /* 2FE10 8003F610 C493A58C */  lw         $a1, %lo(D_800A93C4)($a1)
    /* 2FE14 8003F614 E65F000C */  jal        sys_StubEmpty3
    /* 2FE18 8003F618 02000624 */   addiu     $a2, $zero, 0x2
    /* 2FE1C 8003F61C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2FE20 8003F620 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2FE24 8003F624 0800E003 */  jr         $ra
    /* 2FE28 8003F628 00000000 */   nop
endlabel stage_ApplyLighting
