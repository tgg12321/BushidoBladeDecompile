glabel func_80052788
    /* 42F88 80052788 00008884 */  lh         $t0, 0x0($a0)
    /* 42F8C 8005278C 02008984 */  lh         $t1, 0x2($a0)
    /* 42F90 80052790 04008A84 */  lh         $t2, 0x4($a0)
    /* 42F94 80052794 00100B34 */  ori        $t3, $zero, 0x1000
    /* 42F98 80052798 22586601 */  sub        $t3, $t3, $a2 /* handwritten instruction */
    /* 42F9C 8005279C 00408B48 */  mtc2       $t3, $8 /* handwritten instruction */
    /* 42FA0 800527A0 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 42FA4 800527A4 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 42FA8 800527A8 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 42FAC 800527AC 0000A884 */  lh         $t0, 0x0($a1)
    /* 42FB0 800527B0 00000000 */  nop
    /* 42FB4 800527B4 3D00984B */  gpf        1
    /* 42FB8 800527B8 0200A984 */  lh         $t1, 0x2($a1)
    /* 42FBC 800527BC 0400AA84 */  lh         $t2, 0x4($a1)
    /* 42FC0 800527C0 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 42FC4 800527C4 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 42FC8 800527C8 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 42FCC 800527CC 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 42FD0 800527D0 00000000 */  nop
    /* 42FD4 800527D4 00000000 */  nop
    /* 42FD8 800527D8 3E00A84B */  gpl        1
    /* 42FDC 800527DC 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 42FE0 800527E0 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 42FE4 800527E4 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 42FE8 800527E8 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 42FEC 800527EC 0000E8A4 */  sh         $t0, 0x0($a3)
    /* 42FF0 800527F0 0200E9A4 */  sh         $t1, 0x2($a3)
    /* 42FF4 800527F4 0800E003 */  jr         $ra
    /* 42FF8 800527F8 0400EAA4 */   sh        $t2, 0x4($a3)
endlabel func_80052788
