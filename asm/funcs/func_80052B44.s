glabel func_80052B44
    /* 43344 80052B44 0000888C */  lw         $t0, 0x0($a0)
    /* 43348 80052B48 0400898C */  lw         $t1, 0x4($a0)
    /* 4334C 80052B4C 08008A8C */  lw         $t2, 0x8($a0)
    /* 43350 80052B50 0C008B8C */  lw         $t3, 0xC($a0)
    /* 43354 80052B54 10008C8C */  lw         $t4, 0x10($a0)
    /* 43358 80052B58 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 4335C 80052B5C 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 43360 80052B60 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 43364 80052B64 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 43368 80052B68 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 4336C 80052B6C 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 43370 80052B70 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 43374 80052B74 0800E003 */  jr         $ra
    /* 43378 80052B78 0038C048 */   ctc2      $zero, $7 /* handwritten instruction */
endlabel func_80052B44
