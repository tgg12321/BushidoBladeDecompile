glabel node_GetRootNodeMatrixTranslation
    /* 4588C 8005508C 1080023C */  lui        $v0, %hi(g_view_camera_root_node + 0x2C)
    /* 45890 80055090 1C1E4224 */  addiu      $v0, $v0, %lo(g_view_camera_root_node + 0x2C)
    /* 45894 80055094 0800E003 */  jr         $ra
    /* 45898 80055098 00000000 */   nop
endlabel node_GetRootNodeMatrixTranslation
