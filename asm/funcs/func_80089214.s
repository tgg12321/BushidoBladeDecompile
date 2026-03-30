glabel spu_ReadStatus
    /* 79A14 80089214 0A80043C */  lui        $a0, %hi(D_800A2CF0)
    /* 79A18 80089218 F02C848C */  lw         $a0, %lo(D_800A2CF0)($a0)
    /* 79A1C 8008921C FFF0033C */  lui        $v1, (0xF0FFFFFF >> 16)
    /* 79A20 80089220 0000828C */  lw         $v0, 0x0($a0)
    /* 79A24 80089224 FFFF6334 */  ori        $v1, $v1, (0xF0FFFFFF & 0xFFFF)
    /* 79A28 80089228 24104300 */  and        $v0, $v0, $v1
    /* 79A2C 8008922C 0020033C */  lui        $v1, (0x20000000 >> 16)
    /* 79A30 80089230 25104300 */  or         $v0, $v0, $v1
    /* 79A34 80089234 000082AC */  sw         $v0, 0x0($a0)
    /* 79A38 80089238 0800E003 */  jr         $ra
    /* 79A3C 8008923C 00000000 */   nop
endlabel spu_ReadStatus
