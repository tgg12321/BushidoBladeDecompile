glabel func_8007F24C
    /* 6FA4C 8007F24C 000080C8 */  lwc2       $0, 0x0($a0)
    /* 6FA50 8007F250 040081C8 */  lwc2       $1, 0x4($a0)
    /* 6FA54 8007F254 0000A2C8 */  lwc2       $2, 0x0($a1)
    /* 6FA58 8007F258 0400A3C8 */  lwc2       $3, 0x4($a1)
    /* 6FA5C 8007F25C 0000C4C8 */  lwc2       $4, 0x0($a2)
    /* 6FA60 8007F260 0400C5C8 */  lwc2       $5, 0x4($a2)
    /* 6FA64 8007F264 00000000 */  nop
    /* 6FA68 8007F268 3000284A */  rtpt
    /* 6FA6C 8007F26C 1000A88F */  lw         $t0, 0x10($sp)
    /* 6FA70 8007F270 1400A98F */  lw         $t1, 0x14($sp)
    /* 6FA74 8007F274 1800AA8F */  lw         $t2, 0x18($sp)
    /* 6FA78 8007F278 1C00AB8F */  lw         $t3, 0x1C($sp)
    /* 6FA7C 8007F27C 0000ECE8 */  swc2       $12, 0x0($a3)
    /* 6FA80 8007F280 00000DE9 */  swc2       $13, 0x0($t0)
    /* 6FA84 8007F284 00002EE9 */  swc2       $14, 0x0($t1)
    /* 6FA88 8007F288 000048E9 */  swc2       $8, 0x0($t2)
    /* 6FA8C 8007F28C 00F84348 */  cfc2       $v1, $31 /* handwritten instruction */
    /* 6FA90 8007F290 00980248 */  mfc2       $v0, $19 /* handwritten instruction */
    /* 6FA94 8007F294 000063AD */  sw         $v1, 0x0($t3)
    /* 6FA98 8007F298 0800E003 */  jr         $ra
    /* 6FA9C 8007F29C 83100200 */   sra       $v0, $v0, 2
endlabel func_8007F24C
