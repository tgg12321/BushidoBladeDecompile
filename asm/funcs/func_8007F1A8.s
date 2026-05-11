glabel func_8007F1A8
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
endlabel func_8007F1A8
