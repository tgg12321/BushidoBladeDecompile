glabel func_8007A92C
    /* 6B12C 8007A92C FF00063C */  lui        $a2, (0xFFFFFF >> 16)
    /* 6B130 8007A930 FFFFC634 */  ori        $a2, $a2, (0xFFFFFF & 0xFFFF)
    /* 6B134 8007A934 00FF033C */  lui        $v1, (0xFF000000 >> 16)
    /* 6B138 8007A938 0000828C */  lw         $v0, 0x0($a0)
    /* 6B13C 8007A93C 2428A600 */  and        $a1, $a1, $a2
    /* 6B140 8007A940 24104300 */  and        $v0, $v0, $v1
    /* 6B144 8007A944 25104500 */  or         $v0, $v0, $a1
    /* 6B148 8007A948 0800E003 */  jr         $ra
    /* 6B14C 8007A94C 000082AC */   sw        $v0, 0x0($a0)
endlabel func_8007A92C
