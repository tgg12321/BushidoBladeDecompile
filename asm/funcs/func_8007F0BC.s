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
endlabel func_8007F0BC
