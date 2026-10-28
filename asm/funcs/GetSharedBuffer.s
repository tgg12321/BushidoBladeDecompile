glabel GetSharedBuffer
    /* 29AB8 800392B8 0F80023C */  lui        $v0, %hi(g_shared_buffer)
    /* 29ABC 800392BC D8334224 */  addiu      $v0, $v0, %lo(g_shared_buffer)
    /* 29AC0 800392C0 0800E003 */  jr         $ra
    /* 29AC4 800392C4 00000000 */   nop
endlabel GetSharedBuffer
