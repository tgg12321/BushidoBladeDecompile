glabel func_8007EF1C
    /* 6F71C 8007EF1C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F720 8007EF20 0400898C */  lw         $t1, 0x4($a0)
    /* 6F724 8007EF24 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F728 8007EF28 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6F72C 8007EF2C 10008C8C */  lw         $t4, 0x10($a0)
    /* 6F730 8007EF30 0080C848 */  ctc2       $t0, $16 /* handwritten instruction */
    /* 6F734 8007EF34 0088C948 */  ctc2       $t1, $17 /* handwritten instruction */
    /* 6F738 8007EF38 0090CA48 */  ctc2       $t2, $18 /* handwritten instruction */
    /* 6F73C 8007EF3C 0098CB48 */  ctc2       $t3, $19 /* handwritten instruction */
    /* 6F740 8007EF40 00A0CC48 */  ctc2       $t4, $20 /* handwritten instruction */
    /* 6F744 8007EF44 0800E003 */  jr         $ra
    /* 6F748 8007EF48 00000000 */   nop
endlabel func_8007EF1C
