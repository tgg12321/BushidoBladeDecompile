glabel func_8007E4DC
    /* 6ECDC 8007E4DC 0000888C */  lw         $t0, 0x0($a0)
    /* 6ECE0 8007E4E0 0400898C */  lw         $t1, 0x4($a0)
    /* 6ECE4 8007E4E4 08008A8C */  lw         $t2, 0x8($a0)
    /* 6ECE8 8007E4E8 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6ECEC 8007E4EC 10008C8C */  lw         $t4, 0x10($a0)
    /* 6ECF0 8007E4F0 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6ECF4 8007E4F4 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6ECF8 8007E4F8 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6ECFC 8007E4FC 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6ED00 8007E500 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6ED04 8007E504 0000A894 */  lhu        $t0, 0x0($a1)
    /* 6ED08 8007E508 0400A98C */  lw         $t1, 0x4($a1)
    /* 6ED0C 8007E50C 0C00AA8C */  lw         $t2, 0xC($a1)
    /* 6ED10 8007E510 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6ED14 8007E514 24482101 */  and        $t1, $t1, $at
    /* 6ED18 8007E518 25400901 */  or         $t0, $t0, $t1
    /* 6ED1C 8007E51C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6ED20 8007E520 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6ED24 8007E524 00000000 */  nop
    /* 6ED28 8007E528 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6ED2C 8007E52C 0200A894 */  lhu        $t0, 0x2($a1)
    /* 6ED30 8007E530 0800A98C */  lw         $t1, 0x8($a1)
    /* 6ED34 8007E534 0E00AA84 */  lh         $t2, 0xE($a1)
    /* 6ED38 8007E538 004C0900 */  sll        $t1, $t1, 16
    /* 6ED3C 8007E53C 25400901 */  or         $t0, $t0, $t1
    /* 6ED40 8007E540 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 6ED44 8007E544 00500C48 */  mfc2       $t4, $10 /* handwritten instruction */
    /* 6ED48 8007E548 00580D48 */  mfc2       $t5, $11 /* handwritten instruction */
    /* 6ED4C 8007E54C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6ED50 8007E550 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6ED54 8007E554 00000000 */  nop
    /* 6ED58 8007E558 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6ED5C 8007E55C 0400A894 */  lhu        $t0, 0x4($a1)
    /* 6ED60 8007E560 0800A98C */  lw         $t1, 0x8($a1)
    /* 6ED64 8007E564 1000AA8C */  lw         $t2, 0x10($a1)
    /* 6ED68 8007E568 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6ED6C 8007E56C 24482101 */  and        $t1, $t1, $at
    /* 6ED70 8007E570 25400901 */  or         $t0, $t0, $t1
    /* 6ED74 8007E574 00480E48 */  mfc2       $t6, $9 /* handwritten instruction */
    /* 6ED78 8007E578 00500F48 */  mfc2       $t7, $10 /* handwritten instruction */
    /* 6ED7C 8007E57C 00581848 */  mfc2       $t8, $11 /* handwritten instruction */
    /* 6ED80 8007E580 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6ED84 8007E584 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6ED88 8007E588 00000000 */  nop
    /* 6ED8C 8007E58C 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6ED90 8007E590 FFFF6B31 */  andi       $t3, $t3, 0xFFFF
    /* 6ED94 8007E594 00740E00 */  sll        $t6, $t6, 16
    /* 6ED98 8007E598 2570CB01 */  or         $t6, $t6, $t3
    /* 6ED9C 8007E59C 0000CEAC */  sw         $t6, 0x0($a2)
    /* 6EDA0 8007E5A0 FFFFAD31 */  andi       $t5, $t5, 0xFFFF
    /* 6EDA4 8007E5A4 00C41800 */  sll        $t8, $t8, 16
    /* 6EDA8 8007E5A8 25C00D03 */  or         $t8, $t8, $t5
    /* 6EDAC 8007E5AC 0C00D8AC */  sw         $t8, 0xC($a2)
    /* 6EDB0 8007E5B0 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6EDB4 8007E5B4 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6EDB8 8007E5B8 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EDBC 8007E5BC 00640C00 */  sll        $t4, $t4, 16
    /* 6EDC0 8007E5C0 25400C01 */  or         $t0, $t0, $t4
    /* 6EDC4 8007E5C4 0400C8AC */  sw         $t0, 0x4($a2)
    /* 6EDC8 8007E5C8 FFFFEF31 */  andi       $t7, $t7, 0xFFFF
    /* 6EDCC 8007E5CC 004C0900 */  sll        $t1, $t1, 16
    /* 6EDD0 8007E5D0 25482F01 */  or         $t1, $t1, $t7
    /* 6EDD4 8007E5D4 0800C9AC */  sw         $t1, 0x8($a2)
    /* 6EDD8 8007E5D8 1000CBE8 */  swc2       $11, 0x10($a2)
    /* 6EDDC 8007E5DC 2110C000 */  addu       $v0, $a2, $zero
    /* 6EDE0 8007E5E0 0800E003 */  jr         $ra
    /* 6EDE4 8007E5E4 00000000 */   nop
endlabel func_8007E4DC
