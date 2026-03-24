glabel func_8007A898
    /* 6B098 8007A898 FF00033C */  lui        $v1, (0xFFFFFF >> 16)
    /* 6B09C 8007A89C 0000828C */  lw         $v0, 0x0($a0)
    /* 6B0A0 8007A8A0 FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 6B0A4 8007A8A4 24104300 */  and        $v0, $v0, $v1
    /* 6B0A8 8007A8A8 26104300 */  xor        $v0, $v0, $v1
    /* 6B0AC 8007A8AC 0800E003 */  jr         $ra
    /* 6B0B0 8007A8B0 0100422C */   sltiu     $v0, $v0, 0x1
endlabel func_8007A898
