glabel func_80052CD4
    /* 434D4 80052CD4 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 434D8 80052CD8 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 434DC 80052CDC 83400800 */  sra        $t0, $t0, 2
    /* 434E0 80052CE0 83480900 */  sra        $t1, $t1, 2
    /* 434E4 80052CE4 000088AC */  sw         $t0, 0x0($a0)
    /* 434E8 80052CE8 0000A9AC */  sw         $t1, 0x0($a1)
    /* 434EC 80052CEC 0800E003 */  jr         $ra
    /* 434F0 80052CF0 00000000 */   nop
endlabel func_80052CD4
