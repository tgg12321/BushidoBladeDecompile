glabel menu_ResetConfirmCursor
    /* 5DFFC 8006D7FC 600480AF */  sw         $zero, %gp_rel(D_800A352C)($gp)
    /* 5E000 8006D800 0800E003 */  jr         $ra
    /* 5E004 8006D804 01000224 */   addiu     $v0, $zero, 0x1
endlabel menu_ResetConfirmCursor
