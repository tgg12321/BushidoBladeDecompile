glabel func_8007EC5C
    /* 6F45C 8007EC5C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F460 8007EC60 0400898C */  lw         $t1, 0x4($a0)
    /* 6F464 8007EC64 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F468 8007EC68 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6F46C 8007EC6C 10008C8C */  lw         $t4, 0x10($a0)
    /* 6F470 8007EC70 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F474 8007EC74 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6F478 8007EC78 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6F47C 8007EC7C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6F480 8007EC80 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6F484 8007EC84 0000A894 */  lhu        $t0, 0x0($a1)
    /* 6F488 8007EC88 0400A98C */  lw         $t1, 0x4($a1)
    /* 6F48C 8007EC8C 0C00AA8C */  lw         $t2, 0xC($a1)
    /* 6F490 8007EC90 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6F494 8007EC94 24482101 */  and        $t1, $t1, $at
    /* 6F498 8007EC98 25400901 */  or         $t0, $t0, $t1
    /* 6F49C 8007EC9C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F4A0 8007ECA0 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F4A4 8007ECA4 00000000 */  nop
    /* 6F4A8 8007ECA8 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F4AC 8007ECAC 0200A894 */  lhu        $t0, 0x2($a1)
    /* 6F4B0 8007ECB0 0800A98C */  lw         $t1, 0x8($a1)
    /* 6F4B4 8007ECB4 0E00AA84 */  lh         $t2, 0xE($a1)
    /* 6F4B8 8007ECB8 004C0900 */  sll        $t1, $t1, 16
    /* 6F4BC 8007ECBC 25400901 */  or         $t0, $t0, $t1
    /* 6F4C0 8007ECC0 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 6F4C4 8007ECC4 00500C48 */  mfc2       $t4, $10 /* handwritten instruction */
    /* 6F4C8 8007ECC8 00580D48 */  mfc2       $t5, $11 /* handwritten instruction */
    /* 6F4CC 8007ECCC 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F4D0 8007ECD0 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F4D4 8007ECD4 00000000 */  nop
    /* 6F4D8 8007ECD8 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F4DC 8007ECDC 0400A894 */  lhu        $t0, 0x4($a1)
    /* 6F4E0 8007ECE0 0800A98C */  lw         $t1, 0x8($a1)
    /* 6F4E4 8007ECE4 1000AA8C */  lw         $t2, 0x10($a1)
    /* 6F4E8 8007ECE8 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6F4EC 8007ECEC 24482101 */  and        $t1, $t1, $at
    /* 6F4F0 8007ECF0 25400901 */  or         $t0, $t0, $t1
    /* 6F4F4 8007ECF4 00480E48 */  mfc2       $t6, $9 /* handwritten instruction */
    /* 6F4F8 8007ECF8 00500F48 */  mfc2       $t7, $10 /* handwritten instruction */
    /* 6F4FC 8007ECFC 00581848 */  mfc2       $t8, $11 /* handwritten instruction */
    /* 6F500 8007ED00 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F504 8007ED04 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F508 8007ED08 00000000 */  nop
    /* 6F50C 8007ED0C 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F510 8007ED10 FFFF6B31 */  andi       $t3, $t3, 0xFFFF
    /* 6F514 8007ED14 00740E00 */  sll        $t6, $t6, 16
    /* 6F518 8007ED18 2570CB01 */  or         $t6, $t6, $t3
    /* 6F51C 8007ED1C 0000AEAC */  sw         $t6, 0x0($a1)
    /* 6F520 8007ED20 FFFFAD31 */  andi       $t5, $t5, 0xFFFF
    /* 6F524 8007ED24 00C41800 */  sll        $t8, $t8, 16
    /* 6F528 8007ED28 25C00D03 */  or         $t8, $t8, $t5
    /* 6F52C 8007ED2C 0C00B8AC */  sw         $t8, 0xC($a1)
    /* 6F530 8007ED30 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6F534 8007ED34 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6F538 8007ED38 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6F53C 8007ED3C 00640C00 */  sll        $t4, $t4, 16
    /* 6F540 8007ED40 25400C01 */  or         $t0, $t0, $t4
    /* 6F544 8007ED44 0400A8AC */  sw         $t0, 0x4($a1)
    /* 6F548 8007ED48 FFFFEF31 */  andi       $t7, $t7, 0xFFFF
    /* 6F54C 8007ED4C 004C0900 */  sll        $t1, $t1, 16
    /* 6F550 8007ED50 25482F01 */  or         $t1, $t1, $t7
    /* 6F554 8007ED54 0800A9AC */  sw         $t1, 0x8($a1)
    /* 6F558 8007ED58 1000ABE8 */  swc2       $11, 0x10($a1)
    /* 6F55C 8007ED5C 2110A000 */  addu       $v0, $a1, $zero
    /* 6F560 8007ED60 0800E003 */  jr         $ra
    /* 6F564 8007ED64 00000000 */   nop
endlabel func_8007EC5C
