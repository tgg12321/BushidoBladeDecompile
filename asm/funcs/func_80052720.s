glabel func_80052720
    /* 42F20 80052720 00488448 */  mtc2       $a0, $9 /* handwritten instruction */
    /* 42F24 80052724 00508548 */  mtc2       $a1, $10 /* handwritten instruction */
    /* 42F28 80052728 00588648 */  mtc2       $a2, $11 /* handwritten instruction */
    /* 42F2C 8005272C 00000000 */  nop
    /* 42F30 80052730 00000000 */  nop
    /* 42F34 80052734 2804A04A */  sqr        0
    /* 42F38 80052738 00000000 */  nop
    /* 42F3C 8005273C 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 42F40 80052740 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 42F44 80052744 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 42F48 80052748 20200901 */  add        $a0, $t0, $t1 /* handwritten instruction */
    /* 42F4C 8005274C A8490108 */  j          func_800526A0
    /* 42F50 80052750 20208A00 */   add       $a0, $a0, $t2 /* handwritten instruction */
endlabel func_80052720
