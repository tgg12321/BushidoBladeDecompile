glabel func_8007EEEC
    /* 6F6EC 8007EEEC 0000888C */  lw         $t0, 0x0($a0)
    /* 6F6F0 8007EEF0 0400898C */  lw         $t1, 0x4($a0)
    /* 6F6F4 8007EEF4 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F6F8 8007EEF8 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6F6FC 8007EEFC 10008C8C */  lw         $t4, 0x10($a0)
    /* 6F700 8007EF00 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F704 8007EF04 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6F708 8007EF08 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6F70C 8007EF0C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6F710 8007EF10 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6F714 8007EF14 0800E003 */  jr         $ra
    /* 6F718 8007EF18 00000000 */   nop
endlabel func_8007EEEC
