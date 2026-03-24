glabel func_8001A538
    /* AD38 8001A538 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* AD3C 8001A53C 3000B0AF */  sw         $s0, 0x30($sp)
    /* AD40 8001A540 21808000 */  addu       $s0, $a0, $zero
    /* AD44 8001A544 3400B1AF */  sw         $s1, 0x34($sp)
    /* AD48 8001A548 2188A000 */  addu       $s1, $a1, $zero
    /* AD4C 8001A54C 00100224 */  addiu      $v0, $zero, 0x1000
    /* AD50 8001A550 3800BFAF */  sw         $ra, 0x38($sp)
    /* AD54 8001A554 1000A2A7 */  sh         $v0, 0x10($sp)
    /* AD58 8001A558 1200A0A7 */  sh         $zero, 0x12($sp)
    /* AD5C 8001A55C 1400A0A7 */  sh         $zero, 0x14($sp)
    /* AD60 8001A560 1600A0A7 */  sh         $zero, 0x16($sp)
    /* AD64 8001A564 1800A2A7 */  sh         $v0, 0x18($sp)
    /* AD68 8001A568 1A00A0A7 */  sh         $zero, 0x1A($sp)
    /* AD6C 8001A56C 1C00A0A7 */  sh         $zero, 0x1C($sp)
    /* AD70 8001A570 1E00A0A7 */  sh         $zero, 0x1E($sp)
    /* AD74 8001A574 2000A2A7 */  sh         $v0, 0x20($sp)
    /* AD78 8001A578 10000486 */  lh         $a0, 0x10($s0)
    /* AD7C 8001A57C 1000A527 */  addiu      $a1, $sp, 0x10
    /* AD80 8001A580 1FFE010C */  jal        func_8007F87C
    /* AD84 8001A584 23200400 */   negu      $a0, $a0
    /* AD88 8001A588 12000486 */  lh         $a0, 0x12($s0)
    /* AD8C 8001A58C 1000A527 */  addiu      $a1, $sp, 0x10
    /* AD90 8001A590 87FE010C */  jal        func_8007FA1C
    /* AD94 8001A594 23200400 */   negu      $a0, $a0
    /* AD98 8001A598 14000486 */  lh         $a0, 0x14($s0)
    /* AD9C 8001A59C 1000A527 */  addiu      $a1, $sp, 0x10
    /* ADA0 8001A5A0 EFFE010C */  jal        func_8007FBBC
    /* ADA4 8001A5A4 23200400 */   negu      $a0, $a0
    /* ADA8 8001A5A8 1400A387 */  lh         $v1, 0x14($sp)
    /* ADAC 8001A5AC 1800028E */  lw         $v0, 0x18($s0)
    /* ADB0 8001A5B0 00000000 */  nop
    /* ADB4 8001A5B4 18006200 */  mult       $v1, $v0
    /* ADB8 8001A5B8 0000028E */  lw         $v0, 0x0($s0)
    /* ADBC 8001A5BC 12300000 */  mflo       $a2
    /* ADC0 8001A5C0 031B0600 */  sra        $v1, $a2, 12
    /* ADC4 8001A5C4 23104300 */  subu       $v0, $v0, $v1
    /* ADC8 8001A5C8 000022AE */  sw         $v0, 0x0($s1)
    /* ADCC 8001A5CC 1A00A387 */  lh         $v1, 0x1A($sp)
    /* ADD0 8001A5D0 1800028E */  lw         $v0, 0x18($s0)
    /* ADD4 8001A5D4 00000000 */  nop
    /* ADD8 8001A5D8 18006200 */  mult       $v1, $v0
    /* ADDC 8001A5DC 0400028E */  lw         $v0, 0x4($s0)
    /* ADE0 8001A5E0 12300000 */  mflo       $a2
    /* ADE4 8001A5E4 031B0600 */  sra        $v1, $a2, 12
    /* ADE8 8001A5E8 23104300 */  subu       $v0, $v0, $v1
    /* ADEC 8001A5EC 040022AE */  sw         $v0, 0x4($s1)
    /* ADF0 8001A5F0 2000A387 */  lh         $v1, 0x20($sp)
    /* ADF4 8001A5F4 1800028E */  lw         $v0, 0x18($s0)
    /* ADF8 8001A5F8 00000000 */  nop
    /* ADFC 8001A5FC 18006200 */  mult       $v1, $v0
    /* AE00 8001A600 0800028E */  lw         $v0, 0x8($s0)
    /* AE04 8001A604 12300000 */  mflo       $a2
    /* AE08 8001A608 031B0600 */  sra        $v1, $a2, 12
    /* AE0C 8001A60C 23104300 */  subu       $v0, $v0, $v1
    /* AE10 8001A610 080022AE */  sw         $v0, 0x8($s1)
    /* AE14 8001A614 3800BF8F */  lw         $ra, 0x38($sp)
    /* AE18 8001A618 3400B18F */  lw         $s1, 0x34($sp)
    /* AE1C 8001A61C 3000B08F */  lw         $s0, 0x30($sp)
    /* AE20 8001A620 4000BD27 */  addiu      $sp, $sp, 0x40
    /* AE24 8001A624 0800E003 */  jr         $ra
    /* AE28 8001A628 00000000 */   nop
endlabel func_8001A538
