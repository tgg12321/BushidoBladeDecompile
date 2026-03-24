glabel func_8007EFBC
    /* 6F7BC 8007EFBC 00210400 */  sll        $a0, $a0, 4
    /* 6F7C0 8007EFC0 00290500 */  sll        $a1, $a1, 4
    /* 6F7C4 8007EFC4 00310600 */  sll        $a2, $a2, 4
    /* 6F7C8 8007EFC8 00A8C448 */  ctc2       $a0, $21 /* handwritten instruction */
    /* 6F7CC 8007EFCC 00B0C548 */  ctc2       $a1, $22 /* handwritten instruction */
    /* 6F7D0 8007EFD0 00B8C648 */  ctc2       $a2, $23 /* handwritten instruction */
    /* 6F7D4 8007EFD4 0800E003 */  jr         $ra
    /* 6F7D8 8007EFD8 00000000 */   nop
endlabel func_8007EFBC
