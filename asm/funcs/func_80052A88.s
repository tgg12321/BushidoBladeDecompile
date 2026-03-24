glabel func_80052A88
    /* 43288 80052A88 0000888C */  lw         $t0, 0x0($a0)
    /* 4328C 80052A8C 0400898C */  lw         $t1, 0x4($a0)
    /* 43290 80052A90 08008A8C */  lw         $t2, 0x8($a0)
    /* 43294 80052A94 0C008B8C */  lw         $t3, 0xC($a0)
    /* 43298 80052A98 10008C8C */  lw         $t4, 0x10($a0)
    /* 4329C 80052A9C 14008D8C */  lw         $t5, 0x14($a0)
    /* 432A0 80052AA0 18008E8C */  lw         $t6, 0x18($a0)
    /* 432A4 80052AA4 1C008F8C */  lw         $t7, 0x1C($a0)
    /* 432A8 80052AA8 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 432AC 80052AAC 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 432B0 80052AB0 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 432B4 80052AB4 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 432B8 80052AB8 0400A984 */  lh         $t1, 0x4($a1)
    /* 432BC 80052ABC 0000A894 */  lhu        $t0, 0x0($a1)
    /* 432C0 80052AC0 004C0900 */  sll        $t1, $t1, 16
    /* 432C4 80052AC4 25400901 */  or         $t0, $t0, $t1
    /* 432C8 80052AC8 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 432CC 80052ACC 0028CD48 */  ctc2       $t5, $5 /* handwritten instruction */
    /* 432D0 80052AD0 0030CE48 */  ctc2       $t6, $6 /* handwritten instruction */
    /* 432D4 80052AD4 0038CF48 */  ctc2       $t7, $7 /* handwritten instruction */
    /* 432D8 80052AD8 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 432DC 80052ADC 0800A1C8 */  lwc2       $1, 0x8($a1)
    /* 432E0 80052AE0 00000000 */  nop
    /* 432E4 80052AE4 00000000 */  nop
    /* 432E8 80052AE8 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 432EC 80052AEC 00000000 */  nop
    /* 432F0 80052AF0 0000C9E8 */  swc2       $9, 0x0($a2)
    /* 432F4 80052AF4 0400CAE8 */  swc2       $10, 0x4($a2)
    /* 432F8 80052AF8 0800E003 */  jr         $ra
    /* 432FC 80052AFC 0800CBE8 */   swc2      $11, 0x8($a2)
endlabel func_80052A88
