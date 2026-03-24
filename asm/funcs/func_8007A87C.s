glabel func_8007A87C
    /* 6B07C 8007A87C FF00033C */  lui        $v1, (0xFFFFFF >> 16)
    /* 6B080 8007A880 0000828C */  lw         $v0, 0x0($a0)
    /* 6B084 8007A884 FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 6B088 8007A888 24104300 */  and        $v0, $v0, $v1
    /* 6B08C 8007A88C 0080033C */  lui        $v1, (0x80000000 >> 16)
    /* 6B090 8007A890 0800E003 */  jr         $ra
    /* 6B094 8007A894 25104300 */   or        $v0, $v0, $v1
endlabel func_8007A87C
