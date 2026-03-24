glabel func_8007AB2C
    /* 6B32C 8007AB2C 5555033C */  lui        $v1, (0x55555555 >> 16)
    /* 6B330 8007AB30 55556334 */  ori        $v1, $v1, (0x55555555 & 0xFFFF)
    /* 6B334 8007AB34 07000224 */  addiu      $v0, $zero, 0x7
    /* 6B338 8007AB38 030082A0 */  sb         $v0, 0x3($a0)
    /* 6B33C 8007AB3C 58000224 */  addiu      $v0, $zero, 0x58
    /* 6B340 8007AB40 070082A0 */  sb         $v0, 0x7($a0)
    /* 6B344 8007AB44 0800E003 */  jr         $ra
    /* 6B348 8007AB48 1C0083AC */   sw        $v1, 0x1C($a0)
endlabel func_8007AB2C
