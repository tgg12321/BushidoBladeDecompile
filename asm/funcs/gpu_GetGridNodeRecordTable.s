glabel gpu_GetGridNodeRecordTable
    /* 2F9D4 8003F1D4 0A80023C */  lui        $v0, %hi(g_gpu_grid_node_records)
    /* 2F9D8 8003F1D8 90664224 */  addiu      $v0, $v0, %lo(g_gpu_grid_node_records)
    /* 2F9DC 8003F1DC 0800E003 */  jr         $ra
    /* 2F9E0 8003F1E0 00000000 */   nop
endlabel gpu_GetGridNodeRecordTable
