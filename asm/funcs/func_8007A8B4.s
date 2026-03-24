glabel func_8007A8B4
    /* 6B0B4 8007A8B4 FF00063C */  lui        $a2, (0xFFFFFF >> 16)
    /* 6B0B8 8007A8B8 FFFFC634 */  ori        $a2, $a2, (0xFFFFFF & 0xFFFF)
    /* 6B0BC 8007A8BC 00FF073C */  lui        $a3, (0xFF000000 >> 16)
    /* 6B0C0 8007A8C0 0000A38C */  lw         $v1, 0x0($a1)
    /* 6B0C4 8007A8C4 0000828C */  lw         $v0, 0x0($a0)
    /* 6B0C8 8007A8C8 24186700 */  and        $v1, $v1, $a3
    /* 6B0CC 8007A8CC 24104600 */  and        $v0, $v0, $a2
    /* 6B0D0 8007A8D0 25186200 */  or         $v1, $v1, $v0
    /* 6B0D4 8007A8D4 0000A3AC */  sw         $v1, 0x0($a1)
    /* 6B0D8 8007A8D8 0000828C */  lw         $v0, 0x0($a0)
    /* 6B0DC 8007A8DC 2428A600 */  and        $a1, $a1, $a2
    /* 6B0E0 8007A8E0 24104700 */  and        $v0, $v0, $a3
    /* 6B0E4 8007A8E4 25104500 */  or         $v0, $v0, $a1
    /* 6B0E8 8007A8E8 0800E003 */  jr         $ra
    /* 6B0EC 8007A8EC 000082AC */   sw        $v0, 0x0($a0)
endlabel func_8007A8B4
