glabel func_800457E8
    /* 35FE8 800457E8 0400033C */  lui        $v1, (0x45000 >> 16)
    /* 35FEC 800457EC DC02828F */  lw         $v0, %gp_rel(D_800A33A8)($gp)
    /* 35FF0 800457F0 00506334 */  ori        $v1, $v1, (0x45000 & 0xFFFF)
    /* 35FF4 800457F4 0800E003 */  jr         $ra
    /* 35FF8 800457F8 23106200 */   subu      $v0, $v1, $v0
endlabel func_800457E8
