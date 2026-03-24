glabel func_8007F2DC
    /* 6FADC 8007F2DC 000080C8 */  lwc2       $0, 0x0($a0)
    /* 6FAE0 8007F2E0 040081C8 */  lwc2       $1, 0x4($a0)
    /* 6FAE4 8007F2E4 0000A2C8 */  lwc2       $2, 0x0($a1)
    /* 6FAE8 8007F2E8 0400A3C8 */  lwc2       $3, 0x4($a1)
    /* 6FAEC 8007F2EC 0000C4C8 */  lwc2       $4, 0x0($a2)
    /* 6FAF0 8007F2F0 0400C5C8 */  lwc2       $5, 0x4($a2)
    /* 6FAF4 8007F2F4 00000000 */  nop
    /* 6FAF8 8007F2F8 3000284A */  rtpt
    /* 6FAFC 8007F2FC 1000A88F */  lw         $t0, 0x10($sp)
    /* 6FB00 8007F300 1400A98F */  lw         $t1, 0x14($sp)
    /* 6FB04 8007F304 1800AA8F */  lw         $t2, 0x18($sp)
    /* 6FB08 8007F308 00000CE9 */  swc2       $12, 0x0($t0)
    /* 6FB0C 8007F30C 00002DE9 */  swc2       $13, 0x0($t1)
    /* 6FB10 8007F310 00004EE9 */  swc2       $14, 0x0($t2)
    /* 6FB14 8007F314 00F84348 */  cfc2       $v1, $31 /* handwritten instruction */
    /* 6FB18 8007F318 0000E0C8 */  lwc2       $0, 0x0($a3)
    /* 6FB1C 8007F31C 0400E1C8 */  lwc2       $1, 0x4($a3)
    /* 6FB20 8007F320 00000000 */  nop
    /* 6FB24 8007F324 0100184A */  rtps
    /* 6FB28 8007F328 1C00A88F */  lw         $t0, 0x1C($sp)
    /* 6FB2C 8007F32C 2000A98F */  lw         $t1, 0x20($sp)
    /* 6FB30 8007F330 2400AA8F */  lw         $t2, 0x24($sp)
    /* 6FB34 8007F334 00000EE9 */  swc2       $14, 0x0($t0)
    /* 6FB38 8007F338 000028E9 */  swc2       $8, 0x0($t1)
    /* 6FB3C 8007F33C 00F84848 */  cfc2       $t0, $31 /* handwritten instruction */
    /* 6FB40 8007F340 00980248 */  mfc2       $v0, $19 /* handwritten instruction */
    /* 6FB44 8007F344 25400301 */  or         $t0, $t0, $v1
    /* 6FB48 8007F348 000048AD */  sw         $t0, 0x0($t2)
    /* 6FB4C 8007F34C 0800E003 */  jr         $ra
    /* 6FB50 8007F350 83100200 */   sra       $v0, $v0, 2
endlabel func_8007F2DC
