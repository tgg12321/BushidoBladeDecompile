glabel func_8007E094
    /* 6E894 8007E094 0A80013C */  lui        $at, %hi(D_8009C798)
    /* 6E898 8007E098 98C73FAC */  sw         $ra, %lo(D_8009C798)($at)
    /* 6E89C 8007E09C B7FF010C */  jal        func_8007FEDC
    /* 6E8A0 8007E0A0 00000000 */   nop
    /* 6E8A4 8007E0A4 0A801F3C */  lui        $ra, %hi(D_8009C798)
    /* 6E8A8 8007E0A8 98C7FF8F */  lw         $ra, %lo(D_8009C798)($ra)
    /* 6E8AC 8007E0AC 00000000 */  nop
    /* 6E8B0 8007E0B0 00600240 */  mfc0       $v0, $12 /* handwritten instruction */
    /* 6E8B4 8007E0B4 0040033C */  lui        $v1, (0x40000000 >> 16)
    /* 6E8B8 8007E0B8 25104300 */  or         $v0, $v0, $v1
    /* 6E8BC 8007E0BC 00608240 */  mtc0       $v0, $12 /* handwritten instruction */
    /* 6E8C0 8007E0C0 00000000 */  nop
    /* 6E8C4 8007E0C4 55010824 */  addiu      $t0, $zero, 0x155
    /* 6E8C8 8007E0C8 00E8C848 */  ctc2       $t0, $29 /* handwritten instruction */
    /* 6E8CC 8007E0CC 00000000 */  nop
    /* 6E8D0 8007E0D0 00010824 */  addiu      $t0, $zero, 0x100
    /* 6E8D4 8007E0D4 00F0C848 */  ctc2       $t0, $30 /* handwritten instruction */
    /* 6E8D8 8007E0D8 00000000 */  nop
    /* 6E8DC 8007E0DC E8030824 */  addiu      $t0, $zero, 0x3E8
    /* 6E8E0 8007E0E0 00D0C848 */  ctc2       $t0, $26 /* handwritten instruction */
    /* 6E8E4 8007E0E4 00000000 */  nop
    /* 6E8E8 8007E0E8 9EEF0824 */  addiu      $t0, $zero, -0x1062
    /* 6E8EC 8007E0EC 00D8C848 */  ctc2       $t0, $27 /* handwritten instruction */
    /* 6E8F0 8007E0F0 00000000 */  nop
    /* 6E8F4 8007E0F4 4001083C */  lui        $t0, (0x1400000 >> 16)
    /* 6E8F8 8007E0F8 00E0C848 */  ctc2       $t0, $28 /* handwritten instruction */
    /* 6E8FC 8007E0FC 00000000 */  nop
    /* 6E900 8007E100 00C0C048 */  ctc2       $zero, $24 /* handwritten instruction */
    /* 6E904 8007E104 00C8C048 */  ctc2       $zero, $25 /* handwritten instruction */
    /* 6E908 8007E108 00000000 */  nop
    /* 6E90C 8007E10C 0800E003 */  jr         $ra
    /* 6E910 8007E110 00000000 */   nop
endlabel func_8007E094
