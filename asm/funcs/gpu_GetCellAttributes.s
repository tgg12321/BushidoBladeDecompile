glabel gpu_GetCellAttributes
    /* 2FD4C 8003F54C 40290500 */  sll        $a1, $a1, 5
    /* 2FD50 8003F550 2128A400 */  addu       $a1, $a1, $a0
    /* 2FD54 8003F554 0B80013C */  lui        $at, %hi(g_gpu_cell_draw_attributes)
    /* 2FD58 8003F558 21082500 */  addu       $at, $at, $a1
    /* 2FD5C 8003F55C B08F2290 */  lbu        $v0, %lo(g_gpu_cell_draw_attributes)($at)
    /* 2FD60 8003F560 0800E003 */  jr         $ra
    /* 2FD64 8003F564 00000000 */   nop
endlabel gpu_GetCellAttributes
