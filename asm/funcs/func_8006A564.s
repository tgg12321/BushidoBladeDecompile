glabel func_8006A564
    /* 5AD64 8006A564 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 5AD68 8006A568 2000B2AF */  sw         $s2, 0x20($sp)
    /* 5AD6C 8006A56C 21908000 */  addu       $s2, $a0, $zero
    /* 5AD70 8006A570 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 5AD74 8006A574 2188A000 */  addu       $s1, $a1, $zero
    /* 5AD78 8006A578 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 5AD7C 8006A57C 2800B4AF */  sw         $s4, 0x28($sp)
    /* 5AD80 8006A580 2400B3AF */  sw         $s3, 0x24($sp)
    /* 5AD84 8006A584 1800B0AF */  sw         $s0, 0x18($sp)
    /* 5AD88 8006A588 1800508E */  lw         $s0, 0x18($s2)
    /* 5AD8C 8006A58C 2198C000 */  addu       $s3, $a2, $zero
    /* 5AD90 8006A590 B4EA010C */  jal        initTile
    /* 5AD94 8006A594 21200002 */   addu      $a0, $s0, $zero
    /* 5AD98 8006A598 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5AD9C 8006A59C 00000000 */  nop
    /* 5ADA0 8006A5A0 0F004230 */  andi       $v0, $v0, 0xF
    /* 5ADA4 8006A5A4 07005314 */  bne        $v0, $s3, .L8006A5C4
    /* 5ADA8 8006A5A8 20001424 */   addiu     $s4, $zero, 0x20
    /* 5ADAC 8006A5AC 29002292 */  lbu        $v0, 0x29($s1)
    /* 5ADB0 8006A5B0 050000A2 */  sb         $zero, 0x5($s0)
    /* 5ADB4 8006A5B4 040002A2 */  sb         $v0, 0x4($s0)
    /* 5ADB8 8006A5B8 2B002292 */  lbu        $v0, 0x2B($s1)
    /* 5ADBC 8006A5BC 74A90108 */  j          .L8006A5D0
    /* 5ADC0 8006A5C0 21A00000 */   addu      $s4, $zero, $zero
  .L8006A5C4:
    /* 5ADC4 8006A5C4 50000224 */  addiu      $v0, $zero, 0x50
    /* 5ADC8 8006A5C8 040002A2 */  sb         $v0, 0x4($s0)
    /* 5ADCC 8006A5CC 050002A2 */  sb         $v0, 0x5($s0)
  .L8006A5D0:
    /* 5ADD0 8006A5D0 060002A2 */  sb         $v0, 0x6($s0)
    /* 5ADD4 8006A5D4 5F000224 */  addiu      $v0, $zero, 0x5F
    /* 5ADD8 8006A5D8 080002A6 */  sh         $v0, 0x8($s0)
    /* 5ADDC 8006A5DC 1C00228E */  lw         $v0, 0x1C($s1)
    /* 5ADE0 8006A5E0 00000000 */  nop
    /* 5ADE4 8006A5E4 0F004224 */  addiu      $v0, $v0, 0xF
    /* 5ADE8 8006A5E8 0A0002A6 */  sh         $v0, 0xA($s0)
    /* 5ADEC 8006A5EC 1800228E */  lw         $v0, 0x18($s1)
    /* 5ADF0 8006A5F0 01000324 */  addiu      $v1, $zero, 0x1
    /* 5ADF4 8006A5F4 0E0003A6 */  sh         $v1, 0xE($s0)
    /* 5ADF8 8006A5F8 19004224 */  addiu      $v0, $v0, 0x19
    /* 5ADFC 8006A5FC 0C0002A6 */  sh         $v0, 0xC($s0)
    /* 5AE00 8006A600 1000258E */  lw         $a1, 0x10($s1)
    /* 5AE04 8006A604 5AEA010C */  jal        gpu_SetSemiTransp
    /* 5AE08 8006A608 21200002 */   addu      $a0, $s0, $zero
    /* 5AE0C 8006A60C 21280002 */  addu       $a1, $s0, $zero
    /* 5AE10 8006A610 10001026 */  addiu      $s0, $s0, 0x10
    /* 5AE14 8006A614 1400248E */  lw         $a0, 0x14($s1)
    /* 5AE18 8006A618 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 5AE1C 8006A61C 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 5AE20 8006A620 80200400 */  sll        $a0, $a0, 2
    /* 5AE24 8006A624 2DEA010C */  jal        ot_Link
    /* 5AE28 8006A628 21204400 */   addu      $a0, $v0, $a0
    /* 5AE2C 8006A62C B4EA010C */  jal        initTile
    /* 5AE30 8006A630 21200002 */   addu      $a0, $s0, $zero
    /* 5AE34 8006A634 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5AE38 8006A638 00000000 */  nop
    /* 5AE3C 8006A63C 0F004230 */  andi       $v0, $v0, 0xF
    /* 5AE40 8006A640 07005314 */  bne        $v0, $s3, .L8006A660
    /* 5AE44 8006A644 20000224 */   addiu     $v0, $zero, 0x20
    /* 5AE48 8006A648 29002292 */  lbu        $v0, 0x29($s1)
    /* 5AE4C 8006A64C 050000A2 */  sb         $zero, 0x5($s0)
    /* 5AE50 8006A650 040002A2 */  sb         $v0, 0x4($s0)
    /* 5AE54 8006A654 2B002292 */  lbu        $v0, 0x2B($s1)
    /* 5AE58 8006A658 9BA90108 */  j          .L8006A66C
    /* 5AE5C 8006A65C 060002A2 */   sb        $v0, 0x6($s0)
  .L8006A660:
    /* 5AE60 8006A660 040002A2 */  sb         $v0, 0x4($s0)
    /* 5AE64 8006A664 050002A2 */  sb         $v0, 0x5($s0)
    /* 5AE68 8006A668 060002A2 */  sb         $v0, 0x6($s0)
  .L8006A66C:
    /* 5AE6C 8006A66C 21200002 */  addu       $a0, $s0, $zero
    /* 5AE70 8006A670 1800228E */  lw         $v0, 0x18($s1)
    /* 5AE74 8006A674 01000524 */  addiu      $a1, $zero, 0x1
    /* 5AE78 8006A678 080002A6 */  sh         $v0, 0x8($s0)
    /* 5AE7C 8006A67C 1C00238E */  lw         $v1, 0x1C($s1)
    /* 5AE80 8006A680 78000224 */  addiu      $v0, $zero, 0x78
    /* 5AE84 8006A684 0C0002A6 */  sh         $v0, 0xC($s0)
    /* 5AE88 8006A688 01000224 */  addiu      $v0, $zero, 0x1
    /* 5AE8C 8006A68C 0E0002A6 */  sh         $v0, 0xE($s0)
    /* 5AE90 8006A690 0E006324 */  addiu      $v1, $v1, 0xE
    /* 5AE94 8006A694 5AEA010C */  jal        gpu_SetSemiTransp
    /* 5AE98 8006A698 0A0003A6 */   sh        $v1, 0xA($s0)
    /* 5AE9C 8006A69C 21280002 */  addu       $a1, $s0, $zero
    /* 5AEA0 8006A6A0 10001026 */  addiu      $s0, $s0, 0x10
    /* 5AEA4 8006A6A4 1400248E */  lw         $a0, 0x14($s1)
    /* 5AEA8 8006A6A8 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 5AEAC 8006A6AC 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 5AEB0 8006A6B0 80200400 */  sll        $a0, $a0, 2
    /* 5AEB4 8006A6B4 2DEA010C */  jal        ot_Link
    /* 5AEB8 8006A6B8 21204400 */   addu      $a0, $v0, $a0
    /* 5AEBC 8006A6BC B4EA010C */  jal        initTile
    /* 5AEC0 8006A6C0 21200002 */   addu      $a0, $s0, $zero
    /* 5AEC4 8006A6C4 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5AEC8 8006A6C8 00000000 */  nop
    /* 5AECC 8006A6CC 0F004230 */  andi       $v0, $v0, 0xF
    /* 5AED0 8006A6D0 08005314 */  bne        $v0, $s3, .L8006A6F4
    /* 5AED4 8006A6D4 00000000 */   nop
    /* 5AED8 8006A6D8 29002292 */  lbu        $v0, 0x29($s1)
    /* 5AEDC 8006A6DC 050000A2 */  sb         $zero, 0x5($s0)
    /* 5AEE0 8006A6E0 42100200 */  srl        $v0, $v0, 1
    /* 5AEE4 8006A6E4 040002A2 */  sb         $v0, 0x4($s0)
    /* 5AEE8 8006A6E8 2B002292 */  lbu        $v0, 0x2B($s1)
    /* 5AEEC 8006A6EC C0A90108 */  j          .L8006A700
    /* 5AEF0 8006A6F0 42100200 */   srl       $v0, $v0, 1
  .L8006A6F4:
    /* 5AEF4 8006A6F4 10000224 */  addiu      $v0, $zero, 0x10
    /* 5AEF8 8006A6F8 040002A2 */  sb         $v0, 0x4($s0)
    /* 5AEFC 8006A6FC 050002A2 */  sb         $v0, 0x5($s0)
  .L8006A700:
    /* 5AF00 8006A700 060002A2 */  sb         $v0, 0x6($s0)
    /* 5AF04 8006A704 21200002 */  addu       $a0, $s0, $zero
    /* 5AF08 8006A708 1800228E */  lw         $v0, 0x18($s1)
    /* 5AF0C 8006A70C 01000524 */  addiu      $a1, $zero, 0x1
    /* 5AF10 8006A710 40004224 */  addiu      $v0, $v0, 0x40
    /* 5AF14 8006A714 080002A6 */  sh         $v0, 0x8($s0)
    /* 5AF18 8006A718 1C00238E */  lw         $v1, 0x1C($s1)
    /* 5AF1C 8006A71C 38000224 */  addiu      $v0, $zero, 0x38
    /* 5AF20 8006A720 0C0002A6 */  sh         $v0, 0xC($s0)
    /* 5AF24 8006A724 01000224 */  addiu      $v0, $zero, 0x1
    /* 5AF28 8006A728 0E0002A6 */  sh         $v0, 0xE($s0)
    /* 5AF2C 8006A72C 0D006324 */  addiu      $v1, $v1, 0xD
    /* 5AF30 8006A730 5AEA010C */  jal        gpu_SetSemiTransp
    /* 5AF34 8006A734 0A0003A6 */   sh        $v1, 0xA($s0)
    /* 5AF38 8006A738 21280002 */  addu       $a1, $s0, $zero
    /* 5AF3C 8006A73C 1400248E */  lw         $a0, 0x14($s1)
    /* 5AF40 8006A740 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 5AF44 8006A744 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 5AF48 8006A748 80200400 */  sll        $a0, $a0, 2
    /* 5AF4C 8006A74C 2DEA010C */  jal        ot_Link
    /* 5AF50 8006A750 21204400 */   addu      $a0, $v0, $a0
    /* 5AF54 8006A754 0400428E */  lw         $v0, 0x4($s2)
    /* 5AF58 8006A758 10001026 */  addiu      $s0, $s0, 0x10
    /* 5AF5C 8006A75C 180050AE */  sw         $s0, 0x18($s2)
    /* 5AF60 8006A760 1C00508C */  lw         $s0, 0x1C($v0)
    /* 5AF64 8006A764 00000000 */  nop
    /* 5AF68 8006A768 2800028E */  lw         $v0, 0x28($s0)
    /* 5AF6C 8006A76C 00000000 */  nop
    /* 5AF70 8006A770 000022AE */  sw         $v0, 0x0($s1)
    /* 5AF74 8006A774 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5AF78 8006A778 00000000 */  nop
    /* 5AF7C 8006A77C 0F004230 */  andi       $v0, $v0, 0xF
    /* 5AF80 8006A780 09005314 */  bne        $v0, $s3, .L8006A7A8
    /* 5AF84 8006A784 28000224 */   addiu     $v0, $zero, 0x28
    /* 5AF88 8006A788 29002292 */  lbu        $v0, 0x29($s1)
    /* 5AF8C 8006A78C 2B002392 */  lbu        $v1, 0x2B($s1)
    /* 5AF90 8006A790 2A0020A2 */  sb         $zero, 0x2A($s1)
    /* 5AF94 8006A794 42100200 */  srl        $v0, $v0, 1
    /* 5AF98 8006A798 42180300 */  srl        $v1, $v1, 1
    /* 5AF9C 8006A79C 290022A2 */  sb         $v0, 0x29($s1)
    /* 5AFA0 8006A7A0 EDA90108 */  j          .L8006A7B4
    /* 5AFA4 8006A7A4 2B0023A2 */   sb        $v1, 0x2B($s1)
  .L8006A7A8:
    /* 5AFA8 8006A7A8 2B0022A2 */  sb         $v0, 0x2B($s1)
    /* 5AFAC 8006A7AC 2A0022A2 */  sb         $v0, 0x2A($s1)
    /* 5AFB0 8006A7B0 290022A2 */  sb         $v0, 0x29($s1)
  .L8006A7B4:
    /* 5AFB4 8006A7B4 0000238E */  lw         $v1, 0x0($s1)
    /* 5AFB8 8006A7B8 1C00228E */  lw         $v0, 0x1C($s1)
    /* 5AFBC 8006A7BC 180020AE */  sw         $zero, 0x18($s1)
    /* 5AFC0 8006A7C0 0C006324 */  addiu      $v1, $v1, 0xC
    /* 5AFC4 8006A7C4 0F004224 */  addiu      $v0, $v0, 0xF
    /* 5AFC8 8006A7C8 1C0022AE */  sw         $v0, 0x1C($s1)
    /* 5AFCC 8006A7CC 040023AE */  sw         $v1, 0x4($s1)
    /* 5AFD0 8006A7D0 1400428E */  lw         $v0, 0x14($s2)
    /* 5AFD4 8006A7D4 21202002 */  addu       $a0, $s1, $zero
    /* 5AFD8 8006A7D8 4BCD010C */  jal        func_8007352C
    /* 5AFDC 8006A7DC 080022AE */   sw        $v0, 0x8($s1)
    /* 5AFE0 8006A7E0 140042AE */  sw         $v0, 0x14($s2)
    /* 5AFE4 8006A7E4 2C00028E */  lw         $v0, 0x2C($s0)
    /* 5AFE8 8006A7E8 00000000 */  nop
    /* 5AFEC 8006A7EC 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5AFF0 8006A7F0 000022AE */  sw         $v0, 0x0($s1)
    /* 5AFF4 8006A7F4 040023AE */  sw         $v1, 0x4($s1)
    /* 5AFF8 8006A7F8 1400428E */  lw         $v0, 0x14($s2)
    /* 5AFFC 8006A7FC 21202002 */  addu       $a0, $s1, $zero
    /* 5B000 8006A800 4BCD010C */  jal        func_8007352C
    /* 5B004 8006A804 080022AE */   sw        $v0, 0x8($s1)
    /* 5B008 8006A808 140042AE */  sw         $v0, 0x14($s2)
    /* 5B00C 8006A80C 0000248E */  lw         $a0, 0x0($s1)
    /* 5B010 8006A810 20B9010C */  jal        func_8006E480
    /* 5B014 8006A814 21288002 */   addu      $a1, $s4, $zero
    /* 5B018 8006A818 01000524 */  addiu      $a1, $zero, 0x1
    /* 5B01C 8006A81C 21300000 */  addu       $a2, $zero, $zero
    /* 5B020 8006A820 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5B024 8006A824 1C00448E */  lw         $a0, 0x1C($s2)
    /* 5B028 8006A828 92F0010C */  jal        initTexPage
    /* 5B02C 8006A82C 21384000 */   addu      $a3, $v0, $zero
    /* 5B030 8006A830 1C00458E */  lw         $a1, 0x1C($s2)
    /* 5B034 8006A834 1400248E */  lw         $a0, 0x14($s1)
    /* 5B038 8006A838 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 5B03C 8006A83C 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 5B040 8006A840 80200400 */  sll        $a0, $a0, 2
    /* 5B044 8006A844 2DEA010C */  jal        ot_Link
    /* 5B048 8006A848 21204400 */   addu      $a0, $v0, $a0
    /* 5B04C 8006A84C 1C00428E */  lw         $v0, 0x1C($s2)
    /* 5B050 8006A850 00000000 */  nop
    /* 5B054 8006A854 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B058 8006A858 1C0042AE */  sw         $v0, 0x1C($s2)
    /* 5B05C 8006A85C 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 5B060 8006A860 2800B48F */  lw         $s4, 0x28($sp)
    /* 5B064 8006A864 2400B38F */  lw         $s3, 0x24($sp)
    /* 5B068 8006A868 2000B28F */  lw         $s2, 0x20($sp)
    /* 5B06C 8006A86C 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 5B070 8006A870 1800B08F */  lw         $s0, 0x18($sp)
    /* 5B074 8006A874 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 5B078 8006A878 0800E003 */  jr         $ra
    /* 5B07C 8006A87C 00000000 */   nop
endlabel func_8006A564
