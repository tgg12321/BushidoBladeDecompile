glabel debug_printf
    /* 69A08 80079208 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 69A0C 8007920C 21108000 */  addu       $v0, $a0, $zero
    /* 69A10 80079210 01000424 */  addiu      $a0, $zero, 0x1
    /* 69A14 80079214 1C00A5AF */  sw         $a1, 0x1C($sp)
    /* 69A18 80079218 21284000 */  addu       $a1, $v0, $zero
    /* 69A1C 8007921C 2000A6AF */  sw         $a2, 0x20($sp)
    /* 69A20 80079220 1C00A627 */  addiu      $a2, $sp, 0x1C
    /* 69A24 80079224 1000BFAF */  sw         $ra, 0x10($sp)
    /* 69A28 80079228 1800A2AF */  sw         $v0, 0x18($sp)
    /* 69A2C 8007922C 91E4010C */  jal        func_80079244
    /* 69A30 80079230 2400A7AF */   sw        $a3, 0x24($sp)
    /* 69A34 80079234 1000BF8F */  lw         $ra, 0x10($sp)
    /* 69A38 80079238 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 69A3C 8007923C 0800E003 */  jr         $ra
    /* 69A40 80079240 00000000 */   nop
endlabel debug_printf
