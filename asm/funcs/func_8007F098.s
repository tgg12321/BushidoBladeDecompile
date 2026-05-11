glabel func_8007F098
    /* 6F898 8007F098 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6F89C 8007F09C 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6F8A0 8007F0A0 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6F8A4 8007F0A4 00408548 */  mtc2       $a1, $8 /* handwritten instruction */
    /* 6F8A8 8007F0A8 00000000 */  nop
    /* 6F8AC 8007F0AC 1100984A */  intpl
    /* 6F8B0 8007F0B0 0000D6E8 */  swc2       $22, 0x0($a2)
    /* 6F8B4 8007F0B4 0800E003 */  jr         $ra
    /* 6F8B8 8007F0B8 00000000 */   nop
endlabel func_8007F098
