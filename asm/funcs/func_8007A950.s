glabel func_8007A950
    /* 6B150 8007A950 FF00033C */  lui        $v1, (0xFFFFFF >> 16)
    /* 6B154 8007A954 0000828C */  lw         $v0, 0x0($a0)
    /* 6B158 8007A958 FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 6B15C 8007A95C 25104300 */  or         $v0, $v0, $v1
    /* 6B160 8007A960 0800E003 */  jr         $ra
    /* 6B164 8007A964 000082AC */   sw        $v0, 0x0($a0)
endlabel func_8007A950
