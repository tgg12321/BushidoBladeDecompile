glabel func_8007AB0C
    /* 6B30C 8007AB0C 5555033C */  lui        $v1, (0x55555555 >> 16)
    /* 6B310 8007AB10 55556334 */  ori        $v1, $v1, (0x55555555 & 0xFFFF)
    /* 6B314 8007AB14 05000224 */  addiu      $v0, $zero, 0x5
    /* 6B318 8007AB18 030082A0 */  sb         $v0, 0x3($a0)
    /* 6B31C 8007AB1C 48000224 */  addiu      $v0, $zero, 0x48
    /* 6B320 8007AB20 070082A0 */  sb         $v0, 0x7($a0)
    /* 6B324 8007AB24 0800E003 */  jr         $ra
    /* 6B328 8007AB28 140083AC */   sw        $v1, 0x14($a0)
endlabel func_8007AB0C
