glabel spu_ReadReg
    /* 79A40 80089240 0A80043C */  lui        $a0, %hi(D_800A2CF0)
    /* 79A44 80089244 F02C848C */  lw         $a0, %lo(D_800A2CF0)($a0)
    /* 79A48 80089248 FFF0033C */  lui        $v1, (0xF0FFFFFF >> 16)
    /* 79A4C 8008924C 0000828C */  lw         $v0, 0x0($a0)
    /* 79A50 80089250 FFFF6334 */  ori        $v1, $v1, (0xF0FFFFFF & 0xFFFF)
    /* 79A54 80089254 24104300 */  and        $v0, $v0, $v1
    /* 79A58 80089258 0022033C */  lui        $v1, (0x22000000 >> 16)
    /* 79A5C 8008925C 25104300 */  or         $v0, $v0, $v1
    /* 79A60 80089260 000082AC */  sw         $v0, 0x0($a0)
    /* 79A64 80089264 0800E003 */  jr         $ra
    /* 79A68 80089268 00000000 */   nop
endlabel spu_ReadReg
