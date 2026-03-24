glabel func_8007F0BC
    /* 6F8BC 8007F0BC 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6F8C0 8007F0C0 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6F8C4 8007F0C4 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6F8C8 8007F0C8 00000000 */  nop
    /* 6F8CC 8007F0CC 2804A84A */  sqr        1
    /* 6F8D0 8007F0D0 0000B9E8 */  swc2       $25, 0x0($a1)
    /* 6F8D4 8007F0D4 0400BAE8 */  swc2       $26, 0x4($a1) /* handwritten instruction */
    /* 6F8D8 8007F0D8 0800BBE8 */  swc2       $27, 0x8($a1) /* handwritten instruction */
    /* 6F8DC 8007F0DC 0800E003 */  jr         $ra
    /* 6F8E0 8007F0E0 2110A000 */   addu      $v0, $a1, $zero
    /* 6F8E4 8007F0E4 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6F8E8 8007F0E8 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6F8EC 8007F0EC 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6F8F0 8007F0F0 00000000 */  nop
    /* 6F8F4 8007F0F4 2804A04A */  sqr        0
    /* 6F8F8 8007F0F8 0000B9E8 */  swc2       $25, 0x0($a1)
    /* 6F8FC 8007F0FC 0400BAE8 */  swc2       $26, 0x4($a1) /* handwritten instruction */
    /* 6F900 8007F100 0800BBE8 */  swc2       $27, 0x8($a1) /* handwritten instruction */
    /* 6F904 8007F104 0800E003 */  jr         $ra
    /* 6F908 8007F108 2110A000 */   addu      $v0, $a1, $zero
    /* 6F90C 8007F10C 00888448 */  mtc2       $a0, $17 /* handwritten instruction */
    /* 6F910 8007F110 00908548 */  mtc2       $a1, $18 /* handwritten instruction */
    /* 6F914 8007F114 00988648 */  mtc2       $a2, $19 /* handwritten instruction */
    /* 6F918 8007F118 00000000 */  nop
    /* 6F91C 8007F11C 2D00584B */  avsz3
    /* 6F920 8007F120 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 6F924 8007F124 0800E003 */  jr         $ra
    /* 6F928 8007F128 00000000 */   nop
    /* 6F92C 8007F12C 00808448 */  mtc2       $a0, $16 /* handwritten instruction */
    /* 6F930 8007F130 00888548 */  mtc2       $a1, $17 /* handwritten instruction */
    /* 6F934 8007F134 00908648 */  mtc2       $a2, $18 /* handwritten instruction */
    /* 6F938 8007F138 00988748 */  mtc2       $a3, $19 /* handwritten instruction */
    /* 6F93C 8007F13C 00000000 */  nop
    /* 6F940 8007F140 2E00684B */  avsz4
    /* 6F944 8007F144 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 6F948 8007F148 0800E003 */  jr         $ra
    /* 6F94C 8007F14C 00000000 */   nop
    /* 6F950 8007F150 00004D48 */  cfc2       $t5, $0 /* handwritten instruction */
    /* 6F954 8007F154 00104E48 */  cfc2       $t6, $2 /* handwritten instruction */
    /* 6F958 8007F158 00204F48 */  cfc2       $t7, $4 /* handwritten instruction */
    /* 6F95C 8007F15C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F960 8007F160 0400898C */  lw         $t1, 0x4($a0)
    /* 6F964 8007F164 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F968 8007F168 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F96C 8007F16C 0010C948 */  ctc2       $t1, $2 /* handwritten instruction */
    /* 6F970 8007F170 0020CA48 */  ctc2       $t2, $4 /* handwritten instruction */
    /* 6F974 8007F174 0800ABC8 */  lwc2       $11, 0x8($a1)
    /* 6F978 8007F178 0000A9C8 */  lwc2       $9, 0x0($a1)
    /* 6F97C 8007F17C 0400AAC8 */  lwc2       $10, 0x4($a1)
    /* 6F980 8007F180 00000000 */  nop
    /* 6F984 8007F184 0C00784B */  op         1
    /* 6F988 8007F188 0000D9E8 */  swc2       $25, 0x0($a2)
    /* 6F98C 8007F18C 0400DAE8 */  swc2       $26, 0x4($a2) /* handwritten instruction */
    /* 6F990 8007F190 0800DBE8 */  swc2       $27, 0x8($a2) /* handwritten instruction */
    /* 6F994 8007F194 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 6F998 8007F198 0010CE48 */  ctc2       $t6, $2 /* handwritten instruction */
    /* 6F99C 8007F19C 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 6F9A0 8007F1A0 0800E003 */  jr         $ra
    /* 6F9A4 8007F1A4 00000000 */   nop
    /* 6F9A8 8007F1A8 00004D48 */  cfc2       $t5, $0 /* handwritten instruction */
    /* 6F9AC 8007F1AC 00104E48 */  cfc2       $t6, $2 /* handwritten instruction */
    /* 6F9B0 8007F1B0 00204F48 */  cfc2       $t7, $4 /* handwritten instruction */
    /* 6F9B4 8007F1B4 0000888C */  lw         $t0, 0x0($a0)
    /* 6F9B8 8007F1B8 0400898C */  lw         $t1, 0x4($a0)
    /* 6F9BC 8007F1BC 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F9C0 8007F1C0 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6F9C4 8007F1C4 0010C948 */  ctc2       $t1, $2 /* handwritten instruction */
    /* 6F9C8 8007F1C8 0020CA48 */  ctc2       $t2, $4 /* handwritten instruction */
    /* 6F9CC 8007F1CC 0800ABC8 */  lwc2       $11, 0x8($a1)
    /* 6F9D0 8007F1D0 0000A9C8 */  lwc2       $9, 0x0($a1)
    /* 6F9D4 8007F1D4 0400AAC8 */  lwc2       $10, 0x4($a1)
    /* 6F9D8 8007F1D8 00000000 */  nop
    /* 6F9DC 8007F1DC 0C00704B */  op         0
    /* 6F9E0 8007F1E0 0000D9E8 */  swc2       $25, 0x0($a2)
    /* 6F9E4 8007F1E4 0400DAE8 */  swc2       $26, 0x4($a2) /* handwritten instruction */
    /* 6F9E8 8007F1E8 0800DBE8 */  swc2       $27, 0x8($a2) /* handwritten instruction */
    /* 6F9EC 8007F1EC 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 6F9F0 8007F1F0 0010CE48 */  ctc2       $t6, $2 /* handwritten instruction */
    /* 6F9F4 8007F1F4 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 6F9F8 8007F1F8 0800E003 */  jr         $ra
    /* 6F9FC 8007F1FC 00000000 */   nop
    /* 6FA00 8007F200 00F08448 */  mtc2       $a0, $30 /* handwritten instruction */
    /* 6FA04 8007F204 00000000 */  nop
    /* 6FA08 8007F208 00000000 */  nop
    /* 6FA0C 8007F20C 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6FA10 8007F210 0800E003 */  jr         $ra
    /* 6FA14 8007F214 00000000 */   nop
endlabel func_8007F0BC
