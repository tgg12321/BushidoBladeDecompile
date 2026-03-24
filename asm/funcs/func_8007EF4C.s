glabel func_8007EF4C
    /* 6F74C 8007EF4C 1400888C */  lw         $t0, 0x14($a0)
    /* 6F750 8007EF50 1800898C */  lw         $t1, 0x18($a0)
    /* 6F754 8007EF54 1C008A8C */  lw         $t2, 0x1C($a0)
    /* 6F758 8007EF58 0028C848 */  ctc2       $t0, $5 /* handwritten instruction */
    /* 6F75C 8007EF5C 0030C948 */  ctc2       $t1, $6 /* handwritten instruction */
    /* 6F760 8007EF60 0038CA48 */  ctc2       $t2, $7 /* handwritten instruction */
    /* 6F764 8007EF64 0800E003 */  jr         $ra
    /* 6F768 8007EF68 00000000 */   nop
endlabel func_8007EF4C
