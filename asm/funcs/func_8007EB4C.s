glabel func_8007EB4C
    /* 6F34C 8007EB4C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F350 8007EB50 0400898C */  lw         $t1, 0x4($a0)
    /* 6F354 8007EB54 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F358 8007EB58 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6F35C 8007EB5C 10008C8C */  lw         $t4, 0x10($a0)
    /* 6F360 8007EB60 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F364 8007EB64 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6F368 8007EB68 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6F36C 8007EB6C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6F370 8007EB70 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6F374 8007EB74 0000A894 */  lhu        $t0, 0x0($a1)
    /* 6F378 8007EB78 0400A98C */  lw         $t1, 0x4($a1)
    /* 6F37C 8007EB7C 0C00AA8C */  lw         $t2, 0xC($a1)
    /* 6F380 8007EB80 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6F384 8007EB84 24482101 */  and        $t1, $t1, $at
    /* 6F388 8007EB88 25400901 */  or         $t0, $t0, $t1
    /* 6F38C 8007EB8C 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F390 8007EB90 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F394 8007EB94 00000000 */  nop
    /* 6F398 8007EB98 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F39C 8007EB9C 0200A894 */  lhu        $t0, 0x2($a1)
    /* 6F3A0 8007EBA0 0800A98C */  lw         $t1, 0x8($a1)
    /* 6F3A4 8007EBA4 0E00AA84 */  lh         $t2, 0xE($a1)
    /* 6F3A8 8007EBA8 004C0900 */  sll        $t1, $t1, 16
    /* 6F3AC 8007EBAC 25400901 */  or         $t0, $t0, $t1
    /* 6F3B0 8007EBB0 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 6F3B4 8007EBB4 00500C48 */  mfc2       $t4, $10 /* handwritten instruction */
    /* 6F3B8 8007EBB8 00580D48 */  mfc2       $t5, $11 /* handwritten instruction */
    /* 6F3BC 8007EBBC 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F3C0 8007EBC0 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F3C4 8007EBC4 00000000 */  nop
    /* 6F3C8 8007EBC8 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F3CC 8007EBCC 0400A894 */  lhu        $t0, 0x4($a1)
    /* 6F3D0 8007EBD0 0800A98C */  lw         $t1, 0x8($a1)
    /* 6F3D4 8007EBD4 1000AA8C */  lw         $t2, 0x10($a1)
    /* 6F3D8 8007EBD8 FFFF013C */  lui        $at, (0xFFFF0000 >> 16)
    /* 6F3DC 8007EBDC 24482101 */  and        $t1, $t1, $at
    /* 6F3E0 8007EBE0 25400901 */  or         $t0, $t0, $t1
    /* 6F3E4 8007EBE4 00480E48 */  mfc2       $t6, $9 /* handwritten instruction */
    /* 6F3E8 8007EBE8 00500F48 */  mfc2       $t7, $10 /* handwritten instruction */
    /* 6F3EC 8007EBEC 00581848 */  mfc2       $t8, $11 /* handwritten instruction */
    /* 6F3F0 8007EBF0 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 6F3F4 8007EBF4 00088A48 */  mtc2       $t2, $1 /* handwritten instruction */
    /* 6F3F8 8007EBF8 00000000 */  nop
    /* 6F3FC 8007EBFC 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 6F400 8007EC00 FFFF6B31 */  andi       $t3, $t3, 0xFFFF
    /* 6F404 8007EC04 00740E00 */  sll        $t6, $t6, 16
    /* 6F408 8007EC08 2570CB01 */  or         $t6, $t6, $t3
    /* 6F40C 8007EC0C 00008EAC */  sw         $t6, 0x0($a0)
    /* 6F410 8007EC10 FFFFAD31 */  andi       $t5, $t5, 0xFFFF
    /* 6F414 8007EC14 00C41800 */  sll        $t8, $t8, 16
    /* 6F418 8007EC18 25C00D03 */  or         $t8, $t8, $t5
    /* 6F41C 8007EC1C 0C0098AC */  sw         $t8, 0xC($a0)
    /* 6F420 8007EC20 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6F424 8007EC24 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6F428 8007EC28 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6F42C 8007EC2C 00640C00 */  sll        $t4, $t4, 16
    /* 6F430 8007EC30 25400C01 */  or         $t0, $t0, $t4
    /* 6F434 8007EC34 040088AC */  sw         $t0, 0x4($a0)
    /* 6F438 8007EC38 FFFFEF31 */  andi       $t7, $t7, 0xFFFF
    /* 6F43C 8007EC3C 004C0900 */  sll        $t1, $t1, 16
    /* 6F440 8007EC40 25482F01 */  or         $t1, $t1, $t7
    /* 6F444 8007EC44 080089AC */  sw         $t1, 0x8($a0)
    /* 6F448 8007EC48 10008BE8 */  swc2       $11, 0x10($a0)
    /* 6F44C 8007EC4C 21108000 */  addu       $v0, $a0, $zero
    /* 6F450 8007EC50 0800E003 */  jr         $ra
    /* 6F454 8007EC54 00000000 */   nop
endlabel func_8007EB4C
