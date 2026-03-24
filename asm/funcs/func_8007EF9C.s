glabel func_8007EF9C
    /* 6F79C 8007EF9C 00210400 */  sll        $a0, $a0, 4
    /* 6F7A0 8007EFA0 00290500 */  sll        $a1, $a1, 4
    /* 6F7A4 8007EFA4 00310600 */  sll        $a2, $a2, 4
    /* 6F7A8 8007EFA8 0068C448 */  ctc2       $a0, $13 /* handwritten instruction */
    /* 6F7AC 8007EFAC 0070C548 */  ctc2       $a1, $14 /* handwritten instruction */
    /* 6F7B0 8007EFB0 0078C648 */  ctc2       $a2, $15 /* handwritten instruction */
    /* 6F7B4 8007EFB4 0800E003 */  jr         $ra
    /* 6F7B8 8007EFB8 00000000 */   nop
endlabel func_8007EF9C
