glabel func_8007F150
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
endlabel func_8007F150
