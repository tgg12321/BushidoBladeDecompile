glabel func_8007D2F4
    /* 6DAF4 8007D2F4 0F80013C */  lui        $at, %hi(D_800F189C)
    /* 6DAF8 8007D2F8 21082400 */  addu       $at, $at, $a0
    /* 6DAFC 8007D2FC 9C182290 */  lbu        $v0, %lo(D_800F189C)($at)
    /* 6DB00 8007D300 0800E003 */  jr         $ra
    /* 6DB04 8007D304 00000000 */   nop
endlabel func_8007D2F4
