glabel func_8007A5C4
    /* 6ADC4 8007A5C4 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6ADC8 8007A5C8 21108000 */  addu       $v0, $a0, $zero
    /* 6ADCC 8007A5CC 1800B0AF */  sw         $s0, 0x18($sp)
    /* 6ADD0 8007A5D0 2180A000 */  addu       $s0, $a1, $zero
    /* 6ADD4 8007A5D4 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 6ADD8 8007A5D8 2188C000 */  addu       $s1, $a2, $zero
    /* 6ADDC 8007A5DC 1000A427 */  addiu      $a0, $sp, 0x10
    /* 6ADE0 8007A5E0 21284000 */  addu       $a1, $v0, $zero
    /* 6ADE4 8007A5E4 00010224 */  addiu      $v0, $zero, 0x100
    /* 6ADE8 8007A5E8 1400A2A7 */  sh         $v0, 0x14($sp)
    /* 6ADEC 8007A5EC 01000224 */  addiu      $v0, $zero, 0x1
    /* 6ADF0 8007A5F0 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6ADF4 8007A5F4 1000B0A7 */  sh         $s0, 0x10($sp)
    /* 6ADF8 8007A5F8 1200B1A7 */  sh         $s1, 0x12($sp)
    /* 6ADFC 8007A5FC 80ED010C */  jal        func_8007B600
    /* 6AE00 8007A600 1600A2A7 */   sh        $v0, 0x16($sp)
    /* 6AE04 8007A604 21200002 */  addu       $a0, $s0, $zero
    /* 6AE08 8007A608 F1E9010C */  jal        func_8007A7C4
    /* 6AE0C 8007A60C 21282002 */   addu      $a1, $s1, $zero
    /* 6AE10 8007A610 FFFF4230 */  andi       $v0, $v0, 0xFFFF
    /* 6AE14 8007A614 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6AE18 8007A618 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 6AE1C 8007A61C 1800B08F */  lw         $s0, 0x18($sp)
    /* 6AE20 8007A620 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6AE24 8007A624 0800E003 */  jr         $ra
    /* 6AE28 8007A628 00000000 */   nop
endlabel func_8007A5C4
