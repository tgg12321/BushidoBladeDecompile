glabel func_80052B7C
    /* 4337C 80052B7C 0000888C */  lw         $t0, 0x0($a0)
    /* 43380 80052B80 0400898C */  lw         $t1, 0x4($a0)
    /* 43384 80052B84 08008A8C */  lw         $t2, 0x8($a0)
    /* 43388 80052B88 0C008B8C */  lw         $t3, 0xC($a0)
    /* 4338C 80052B8C 10008C8C */  lw         $t4, 0x10($a0)
    /* 43390 80052B90 0000AD84 */  lh         $t5, 0x0($a1)
    /* 43394 80052B94 0200AE84 */  lh         $t6, 0x2($a1)
    /* 43398 80052B98 0400AF84 */  lh         $t7, 0x4($a1)
    /* 4339C 80052B9C 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 433A0 80052BA0 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 433A4 80052BA4 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 433A8 80052BA8 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 433AC 80052BAC 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 433B0 80052BB0 0028CD48 */  ctc2       $t5, $5 /* handwritten instruction */
    /* 433B4 80052BB4 0030CE48 */  ctc2       $t6, $6 /* handwritten instruction */
    /* 433B8 80052BB8 0038CF48 */  ctc2       $t7, $7 /* handwritten instruction */
    /* 433BC 80052BBC 0000C0C8 */  lwc2       $0, 0x0($a2)
    /* 433C0 80052BC0 0400C1C8 */  lwc2       $1, 0x4($a2)
    /* 433C4 80052BC4 00000000 */  nop
    /* 433C8 80052BC8 00000000 */  nop
    /* 433CC 80052BCC 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 433D0 80052BD0 00000000 */  nop
    /* 433D4 80052BD4 0000E9E8 */  swc2       $9, 0x0($a3)
    /* 433D8 80052BD8 0400EAE8 */  swc2       $10, 0x4($a3)
    /* 433DC 80052BDC 0800E003 */  jr         $ra
    /* 433E0 80052BE0 0800EBE8 */   swc2      $11, 0x8($a3)
endlabel func_80052B7C
