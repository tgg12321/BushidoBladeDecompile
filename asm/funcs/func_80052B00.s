glabel func_80052B00
    /* 43300 80052B00 0000888C */  lw         $t0, 0x0($a0)
    /* 43304 80052B04 0400898C */  lw         $t1, 0x4($a0)
    /* 43308 80052B08 08008A8C */  lw         $t2, 0x8($a0)
    /* 4330C 80052B0C 0C008B8C */  lw         $t3, 0xC($a0)
    /* 43310 80052B10 10008C8C */  lw         $t4, 0x10($a0)
    /* 43314 80052B14 14008D8C */  lw         $t5, 0x14($a0)
    /* 43318 80052B18 18008E8C */  lw         $t6, 0x18($a0)
    /* 4331C 80052B1C 1C008F8C */  lw         $t7, 0x1C($a0)
    /* 43320 80052B20 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 43324 80052B24 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 43328 80052B28 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 4332C 80052B2C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 43330 80052B30 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 43334 80052B34 0028CD48 */  ctc2       $t5, $5 /* handwritten instruction */
    /* 43338 80052B38 0030CE48 */  ctc2       $t6, $6 /* handwritten instruction */
    /* 4333C 80052B3C 0800E003 */  jr         $ra
    /* 43340 80052B40 0038CF48 */   ctc2      $t7, $7 /* handwritten instruction */
endlabel func_80052B00
