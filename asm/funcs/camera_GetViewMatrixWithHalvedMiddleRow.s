glabel camera_GetViewMatrixWithHalvedMiddleRow
    /* 37AB0 800472B0 0F80023C */  lui        $v0, %hi(g_camera_halved_row_view_matrix)
    /* 37AB4 800472B4 D0ED4224 */  addiu      $v0, $v0, %lo(g_camera_halved_row_view_matrix)
    /* 37AB8 800472B8 0800E003 */  jr         $ra
    /* 37ABC 800472BC 00000000 */   nop
endlabel camera_GetViewMatrixWithHalvedMiddleRow
