glabel func_8007D358
    /* 6DB58 8007D358 0004033C */  lui        $v1, (0x4000002 >> 16)
    /* 6DB5C 8007D35C 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6DB60 8007D360 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6DB64 8007D364 02006334 */  ori        $v1, $v1, (0x4000002 & 0xFFFF)
    /* 6DB68 8007D368 000043AC */  sw         $v1, 0x0($v0)
    /* 6DB6C 8007D36C 0A80023C */  lui        $v0, %hi(D_8009BF4C)
    /* 6DB70 8007D370 4CBF428C */  lw         $v0, %lo(D_8009BF4C)($v0)
    /* 6DB74 8007D374 00000000 */  nop
    /* 6DB78 8007D378 000044AC */  sw         $a0, 0x0($v0)
    /* 6DB7C 8007D37C 0A80023C */  lui        $v0, %hi(D_8009BF50)
    /* 6DB80 8007D380 50BF428C */  lw         $v0, %lo(D_8009BF50)($v0)
    /* 6DB84 8007D384 0001033C */  lui        $v1, (0x1000401 >> 16)
    /* 6DB88 8007D388 000040AC */  sw         $zero, 0x0($v0)
    /* 6DB8C 8007D38C 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6DB90 8007D390 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6DB94 8007D394 01046334 */  ori        $v1, $v1, (0x1000401 & 0xFFFF)
    /* 6DB98 8007D398 000043AC */  sw         $v1, 0x0($v0)
    /* 6DB9C 8007D39C 0800E003 */  jr         $ra
    /* 6DBA0 8007D3A0 00000000 */   nop
endlabel func_8007D358
