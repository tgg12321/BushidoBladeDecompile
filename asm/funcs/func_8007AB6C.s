glabel func_8007AB6C
    /* 6B36C 8007AB6C 5555033C */  lui        $v1, (0x55555555 >> 16)
    /* 6B370 8007AB70 55556334 */  ori        $v1, $v1, (0x55555555 & 0xFFFF)
    /* 6B374 8007AB74 09000224 */  addiu      $v0, $zero, 0x9
    /* 6B378 8007AB78 030082A0 */  sb         $v0, 0x3($a0)
    /* 6B37C 8007AB7C 5C000224 */  addiu      $v0, $zero, 0x5C
    /* 6B380 8007AB80 070082A0 */  sb         $v0, 0x7($a0)
    /* 6B384 8007AB84 0800E003 */  jr         $ra
    /* 6B388 8007AB88 240083AC */   sw        $v1, 0x24($a0)
endlabel func_8007AB6C
