glabel func_8007F0E4
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
endlabel func_8007F0E4
