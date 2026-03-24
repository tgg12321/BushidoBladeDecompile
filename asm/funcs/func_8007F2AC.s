glabel func_8007F2AC
    /* 6FAAC 8007F2AC 000080C8 */  lwc2       $0, 0x0($a0)
    /* 6FAB0 8007F2B0 040081C8 */  lwc2       $1, 0x4($a0)
    /* 6FAB4 8007F2B4 00000000 */  nop
    /* 6FAB8 8007F2B8 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 6FABC 8007F2BC 0000B9E8 */  swc2       $25, 0x0($a1)
    /* 6FAC0 8007F2C0 0400BAE8 */  swc2       $26, 0x4($a1) /* handwritten instruction */
    /* 6FAC4 8007F2C4 0800BBE8 */  swc2       $27, 0x8($a1) /* handwritten instruction */
    /* 6FAC8 8007F2C8 00F84248 */  cfc2       $v0, $31 /* handwritten instruction */
    /* 6FACC 8007F2CC 0800E003 */  jr         $ra
    /* 6FAD0 8007F2D0 0000C2AC */   sw        $v0, 0x0($a2)
endlabel func_8007F2AC
