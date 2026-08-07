glabel func_80052A20
    /* 43220 80052A20 0000888C */  lw         $t0, 0x0($a0)
    /* 43224 80052A24 0400898C */  lw         $t1, 0x4($a0)
    /* 43228 80052A28 08008A8C */  lw         $t2, 0x8($a0)
    /* 4322C 80052A2C 0C008B8C */  lw         $t3, 0xC($a0)
    /* 43230 80052A30 10008C8C */  lw         $t4, 0x10($a0)
    /* 43234 80052A34 14008D8C */  lw         $t5, 0x14($a0)
    /* 43238 80052A38 18008E8C */  lw         $t6, 0x18($a0)
    /* 4323C 80052A3C 1C008F8C */  lw         $t7, 0x1C($a0)
    /* 43240 80052A40 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 43244 80052A44 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 43248 80052A48 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 4324C 80052A4C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 43250 80052A50 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 43254 80052A54 0028CD48 */  ctc2       $t5, $5 /* handwritten instruction */
    /* 43258 80052A58 0030CE48 */  ctc2       $t6, $6 /* handwritten instruction */
    /* 4325C 80052A5C 0038CF48 */  ctc2       $t7, $7 /* handwritten instruction */
    /* 43260 80052A60 0000A0C8 */  lwc2       $0, 0x0($a1)
    /* 43264 80052A64 0400A1C8 */  lwc2       $1, 0x4($a1)
    /* 43268 80052A68 00000000 */  nop
    /* 4326C 80052A6C 00000000 */  nop
    /* 43270 80052A70 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 43274 80052A74 00000000 */  nop
    /* 43278 80052A78 0000C9E8 */  swc2       $9, 0x0($a2)
    /* 4327C 80052A7C 0400CAE8 */  swc2       $10, 0x4($a2)
    /* 43280 80052A80 0800E003 */  jr         $ra
    /* 43284 80052A84 0800CBE8 */   swc2      $11, 0x8($a2)
endlabel func_80052A20
