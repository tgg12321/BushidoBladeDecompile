glabel func_80052BE4
    /* 433E4 80052BE4 00A84848 */  cfc2       $t0, $21 /* handwritten instruction */
    /* 433E8 80052BE8 00B04948 */  cfc2       $t1, $22 /* handwritten instruction */
    /* 433EC 80052BEC 00B84A48 */  cfc2       $t2, $23 /* handwritten instruction */
    /* 433F0 80052BF0 02410800 */  srl        $t0, $t0, 4
    /* 433F4 80052BF4 02490900 */  srl        $t1, $t1, 4
    /* 433F8 80052BF8 02510A00 */  srl        $t2, $t2, 4
    /* 433FC 80052BFC 000088A0 */  sb         $t0, 0x0($a0)
    /* 43400 80052C00 010089A0 */  sb         $t1, 0x1($a0)
    /* 43404 80052C04 02008AA0 */  sb         $t2, 0x2($a0)
    /* 43408 80052C08 0800E003 */  jr         $ra
    /* 4340C 80052C0C 00000000 */   nop
endlabel func_80052BE4
