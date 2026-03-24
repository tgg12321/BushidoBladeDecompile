glabel func_80052754
    /* 42F54 80052754 00488448 */  mtc2       $a0, $9 /* handwritten instruction */
    /* 42F58 80052758 00508548 */  mtc2       $a1, $10 /* handwritten instruction */
    /* 42F5C 8005275C 00588648 */  mtc2       $a2, $11 /* handwritten instruction */
    /* 42F60 80052760 00000000 */  nop
    /* 42F64 80052764 00000000 */  nop
    /* 42F68 80052768 2804A04A */  sqr        0
    /* 42F6C 8005276C 00000000 */  nop
    /* 42F70 80052770 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 42F74 80052774 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 42F78 80052778 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 42F7C 8005277C 21100901 */  addu       $v0, $t0, $t1
    /* 42F80 80052780 0800E003 */  jr         $ra
    /* 42F84 80052784 21104A00 */   addu      $v0, $v0, $t2
endlabel func_80052754
