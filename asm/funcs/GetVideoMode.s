glabel GetVideoMode
    /* 73E88 80083688 0A80023C */  lui        $v0, %hi(video_mode)
    /* 73E8C 8008368C 6426428C */  lw         $v0, %lo(video_mode)($v0)
    /* 73E90 80083690 0800E003 */  jr         $ra
    /* 73E94 80083694 00000000 */   nop
endlabel GetVideoMode
