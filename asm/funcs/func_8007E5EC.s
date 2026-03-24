glabel func_8007E5EC
    /* 6EDEC 8007E5EC 0000888C */  lw         $t0, 0x0($a0)
    /* 6EDF0 8007E5F0 0400898C */  lw         $t1, 0x4($a0)
    /* 6EDF4 8007E5F4 08008A8C */  lw         $t2, 0x8($a0)
    /* 6EDF8 8007E5F8 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6EDFC 8007E5FC 10008C8C */  lw         $t4, 0x10($a0)
    /* 6EE00 8007E600 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6EE04 8007E604 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6EE08 8007E608 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6EE0C 8007E60C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6EE10 8007E610 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6EE14 8007E614 0000A894 */  lhu        $t0, 0x0($a1)
    /* 6EE18 8007E618 0400A98C */  lw         $t1, 0x4($a1)
    /* 6EE1C 8007E61C 0C00AA8C */  lw         $t2, 0xC($a1)
    /* 6EE20 8007E620 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6EE24 8007E624 24482101 */  and        $t1, $t1, $at
    /* 6EE28 8007E628 25400901 */  or         $t0, $t0, $t1
    /* 6EE2C 8007E62C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6EE30 8007E630 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6EE34 8007E634 00000000 */  nop
    /* 6EE38 8007E638 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6EE3C 8007E63C 0200A894 */  lhu        $t0, 0x2($a1)
    /* 6EE40 8007E640 0800A98C */  lw         $t1, 0x8($a1)
    /* 6EE44 8007E644 0E00AA84 */  lh         $t2, 0xE($a1)
    /* 6EE48 8007E648 004C0900 */  sll        $t1, $t1, 16
    /* 6EE4C 8007E64C 25400901 */  or         $t0, $t0, $t1
    /* 6EE50 8007E650 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 6EE54 8007E654 00500C48 */  mfc2       $t4, $10 /* handwritten instruction */
    /* 6EE58 8007E658 00580D48 */  mfc2       $t5, $11 /* handwritten instruction */
    /* 6EE5C 8007E65C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6EE60 8007E660 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6EE64 8007E664 00000000 */  nop
    /* 6EE68 8007E668 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6EE6C 8007E66C 0400A894 */  lhu        $t0, 0x4($a1)
    /* 6EE70 8007E670 0800A98C */  lw         $t1, 0x8($a1)
    /* 6EE74 8007E674 1000AA8C */  lw         $t2, 0x10($a1)
    /* 6EE78 8007E678 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6EE7C 8007E67C 24482101 */  and        $t1, $t1, $at
    /* 6EE80 8007E680 25400901 */  or         $t0, $t0, $t1
    /* 6EE84 8007E684 00480E48 */  mfc2       $t6, $9 /* handwritten instruction */
    /* 6EE88 8007E688 00500F48 */  mfc2       $t7, $10 /* handwritten instruction */
    /* 6EE8C 8007E68C 00581848 */  mfc2       $t8, $11 /* handwritten instruction */
    /* 6EE90 8007E690 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6EE94 8007E694 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6EE98 8007E698 00000000 */  nop
    /* 6EE9C 8007E69C 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6EEA0 8007E6A0 FFFF6B31 */  andi       $t3, $t3, 0xFFFF
    /* 6EEA4 8007E6A4 00740E00 */  sll        $t6, $t6, 16
    /* 6EEA8 8007E6A8 2570CB01 */  or         $t6, $t6, $t3
    /* 6EEAC 8007E6AC 0000CEAC */  sw         $t6, 0x0($a2)
    /* 6EEB0 8007E6B0 FFFFAD31 */  andi       $t5, $t5, 0xFFFF
    /* 6EEB4 8007E6B4 00C41800 */  sll        $t8, $t8, 16
    /* 6EEB8 8007E6B8 25C00D03 */  or         $t8, $t8, $t5
    /* 6EEBC 8007E6BC 0C00D8AC */  sw         $t8, 0xC($a2)
    /* 6EEC0 8007E6C0 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6EEC4 8007E6C4 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6EEC8 8007E6C8 1000CBE8 */  swc2       $11, 0x10($a2)
    /* 6EECC 8007E6CC 1400AD94 */  lhu        $t5, 0x14($a1)
    /* 6EED0 8007E6D0 1800AE8C */  lw         $t6, 0x18($a1)
    /* 6EED4 8007E6D4 1C00AA8C */  lw         $t2, 0x1C($a1)
    /* 6EED8 8007E6D8 00740E00 */  sll        $t6, $t6, 16
    /* 6EEDC 8007E6DC 2568AE01 */  or         $t5, $t5, $t6
    /* 6EEE0 8007E6E0 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 6EEE4 8007E6E4 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6EEE8 8007E6E8 00000000 */  nop
    /* 6EEEC 8007E6EC 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6EEF0 8007E6F0 00640C00 */  sll        $t4, $t4, 16
    /* 6EEF4 8007E6F4 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EEF8 8007E6F8 25400C01 */  or         $t0, $t0, $t4
    /* 6EEFC 8007E6FC 0400C8AC */  sw         $t0, 0x4($a2)
    /* 6EF00 8007E700 FFFFEF31 */  andi       $t7, $t7, 0xFFFF
    /* 6EF04 8007E704 004C0900 */  sll        $t1, $t1, 16
    /* 6EF08 8007E708 25482F01 */  or         $t1, $t1, $t7
    /* 6EF0C 8007E70C 0800C9AC */  sw         $t1, 0x8($a2)
    /* 6EF10 8007E710 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6EF14 8007E714 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6EF18 8007E718 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 6EF1C 8007E71C 14008B8C */  lw         $t3, 0x14($a0)
    /* 6EF20 8007E720 18008C8C */  lw         $t4, 0x18($a0)
    /* 6EF24 8007E724 1C008D8C */  lw         $t5, 0x1C($a0)
    /* 6EF28 8007E728 20400B01 */  add        $t0, $t0, $t3 /* handwritten instruction */
    /* 6EF2C 8007E72C 20482C01 */  add        $t1, $t1, $t4 /* handwritten instruction */
    /* 6EF30 8007E730 20504D01 */  add        $t2, $t2, $t5 /* handwritten instruction */
    /* 6EF34 8007E734 1400C8AC */  sw         $t0, 0x14($a2)
    /* 6EF38 8007E738 1800C9AC */  sw         $t1, 0x18($a2)
    /* 6EF3C 8007E73C 1C00CAAC */  sw         $t2, 0x1C($a2)
    /* 6EF40 8007E740 2110C000 */  addu       $v0, $a2, $zero
    /* 6EF44 8007E744 0800E003 */  jr         $ra
    /* 6EF48 8007E748 00000000 */   nop
endlabel func_8007E5EC
