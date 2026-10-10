glabel _status
    /* 6D2B0 8007CAB0 0A80023C */  lui        $v0, %hi(GPU_STATUS)
    /* 6D2B4 8007CAB4 48BF428C */  lw         $v0, %lo(GPU_STATUS)($v0)
    /* 6D2B8 8007CAB8 00000000 */  nop
    /* 6D2BC 8007CABC 0000428C */  lw         $v0, 0x0($v0)
    /* 6D2C0 8007CAC0 0800E003 */  jr         $ra
    /* 6D2C4 8007CAC4 00000000 */   nop
endlabel _status
