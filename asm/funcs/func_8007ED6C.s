glabel func_8007ED6C
    /* 6F56C 8007ED6C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F570 8007ED70 0400898C */  lw         $t1, 0x4($a0)
    /* 6F574 8007ED74 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F578 8007ED78 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6F57C 8007ED7C 10008C8C */  lw         $t4, 0x10($a0)
    /* 6F580 8007ED80 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F584 8007ED84 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6F588 8007ED88 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6F58C 8007ED8C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6F590 8007ED90 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6F594 8007ED94 0000A0C8 */  lwc2       $0, 0x0($a1)
    /* 6F598 8007ED98 0400A1C8 */  lwc2       $1, 0x4($a1)
    /* 6F59C 8007ED9C 00000000 */  nop
    /* 6F5A0 8007EDA0 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F5A4 8007EDA4 0000D9E8 */  swc2       $25, 0x0($a2)
    /* 6F5A8 8007EDA8 0400DAE8 */  swc2       $26, 0x4($a2) /* handwritten instruction */
    /* 6F5AC 8007EDAC 0800DBE8 */  swc2       $27, 0x8($a2) /* handwritten instruction */
    /* 6F5B0 8007EDB0 2110C000 */  addu       $v0, $a2, $zero
    /* 6F5B4 8007EDB4 0800E003 */  jr         $ra
    /* 6F5B8 8007EDB8 00000000 */   nop
endlabel func_8007ED6C
