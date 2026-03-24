glabel func_8007B4D0
    /* 6BCD0 8007B4D0 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6BCD4 8007B4D4 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6BCD8 8007B4D8 21988000 */  addu       $s3, $a0, $zero
    /* 6BCDC 8007B4DC 0180043C */  lui        $a0, %hi(D_80015F50)
    /* 6BCE0 8007B4E0 505F8424 */  addiu      $a0, $a0, %lo(D_80015F50)
    /* 6BCE4 8007B4E4 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6BCE8 8007B4E8 2190A000 */  addu       $s2, $a1, $zero
    /* 6BCEC 8007B4EC 21286002 */  addu       $a1, $s3, $zero
    /* 6BCF0 8007B4F0 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BCF4 8007B4F4 2188C000 */  addu       $s1, $a2, $zero
    /* 6BCF8 8007B4F8 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BCFC 8007B4FC 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6BD00 8007B500 EAEC010C */  jal        func_8007B3A8
    /* 6BD04 8007B504 2180E000 */   addu      $s0, $a3, $zero
    /* 6BD08 8007B508 21286002 */  addu       $a1, $s3, $zero
    /* 6BD0C 8007B50C FF001032 */  andi       $s0, $s0, 0xFF
    /* 6BD10 8007B510 00841000 */  sll        $s0, $s0, 16
    /* 6BD14 8007B514 FF003132 */  andi       $s1, $s1, 0xFF
    /* 6BD18 8007B518 008A1100 */  sll        $s1, $s1, 8
    /* 6BD1C 8007B51C 25801102 */  or         $s0, $s0, $s1
    /* 6BD20 8007B520 FF005232 */  andi       $s2, $s2, 0xFF
    /* 6BD24 8007B524 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6BD28 8007B528 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6BD2C 8007B52C 08000624 */  addiu      $a2, $zero, 0x8
    /* 6BD30 8007B530 0C00448C */  lw         $a0, 0xC($v0)
    /* 6BD34 8007B534 0800428C */  lw         $v0, 0x8($v0)
    /* 6BD38 8007B538 00000000 */  nop
    /* 6BD3C 8007B53C 09F84000 */  jalr       $v0
    /* 6BD40 8007B540 25381202 */   or        $a3, $s0, $s2
    /* 6BD44 8007B544 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6BD48 8007B548 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6BD4C 8007B54C 1800B28F */  lw         $s2, 0x18($sp)
    /* 6BD50 8007B550 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BD54 8007B554 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BD58 8007B558 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6BD5C 8007B55C 0800E003 */  jr         $ra
    /* 6BD60 8007B560 00000000 */   nop
endlabel func_8007B4D0
