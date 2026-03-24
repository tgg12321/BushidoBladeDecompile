glabel func_8007AB4C
    /* 6B34C 8007AB4C 5555033C */  lui        $v1, (0x55555555 >> 16)
    /* 6B350 8007AB50 55556334 */  ori        $v1, $v1, (0x55555555 & 0xFFFF)
    /* 6B354 8007AB54 06000224 */  addiu      $v0, $zero, 0x6
    /* 6B358 8007AB58 030082A0 */  sb         $v0, 0x3($a0)
    /* 6B35C 8007AB5C 4C000224 */  addiu      $v0, $zero, 0x4C
    /* 6B360 8007AB60 070082A0 */  sb         $v0, 0x7($a0)
    /* 6B364 8007AB64 0800E003 */  jr         $ra
    /* 6B368 8007AB68 180083AC */   sw        $v1, 0x18($a0)
endlabel func_8007AB4C
