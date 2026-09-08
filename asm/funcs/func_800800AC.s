glabel CdMode
    /* 708AC 800800AC 0A80023C */  lui        $v0, %hi(CD_mode)
    /* 708B0 800800B0 D4114290 */  lbu        $v0, %lo(CD_mode)($v0)
    /* 708B4 800800B4 0800E003 */  jr         $ra
    /* 708B8 800800B8 00000000 */   nop
    /* 708BC 800800BC 0A80023C */  lui        $v0, %hi(CD_com)
    /* 708C0 800800C0 D5114290 */  lbu        $v0, %lo(CD_com)($v0)
    /* 708C4 800800C4 0800E003 */  jr         $ra
    /* 708C8 800800C8 00000000 */   nop
endlabel CdMode
