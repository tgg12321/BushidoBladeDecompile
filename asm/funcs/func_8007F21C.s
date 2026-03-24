glabel func_8007F21C
    /* 6FA1C 8007F21C 000080C8 */  lwc2       $0, 0x0($a0)
    /* 6FA20 8007F220 040081C8 */  lwc2       $1, 0x4($a0)
    /* 6FA24 8007F224 00000000 */  nop
    /* 6FA28 8007F228 0100184A */  rtps
    /* 6FA2C 8007F22C 0000AEE8 */  swc2       $14, 0x0($a1)
    /* 6FA30 8007F230 0000C8E8 */  swc2       $8, 0x0($a2)
    /* 6FA34 8007F234 00F84348 */  cfc2       $v1, $31 /* handwritten instruction */
    /* 6FA38 8007F238 00980248 */  mfc2       $v0, $19 /* handwritten instruction */
    /* 6FA3C 8007F23C 0000E3AC */  sw         $v1, 0x0($a3)
    /* 6FA40 8007F240 0800E003 */  jr         $ra
    /* 6FA44 8007F244 83100200 */   sra       $v0, $v0, 2
endlabel func_8007F21C
