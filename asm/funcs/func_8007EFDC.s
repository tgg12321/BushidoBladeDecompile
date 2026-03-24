glabel func_8007EFDC
    /* 6F7DC 8007EFDC 00240400 */  sll        $a0, $a0, 16
    /* 6F7E0 8007EFE0 002C0500 */  sll        $a1, $a1, 16
    /* 6F7E4 8007EFE4 00C0C448 */  ctc2       $a0, $24 /* handwritten instruction */
    /* 6F7E8 8007EFE8 00C8C548 */  ctc2       $a1, $25 /* handwritten instruction */
    /* 6F7EC 8007EFEC 0800E003 */  jr         $ra
    /* 6F7F0 8007EFF0 00000000 */   nop
endlabel func_8007EFDC
