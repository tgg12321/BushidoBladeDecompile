glabel func_8004A4E0
    /* 3ACE0 8004A4E0 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3ACE4 8004A4E4 3400BFAF */  sw         $ra, 0x34($sp)
    /* 3ACE8 8004A4E8 3000BEAF */  sw         $fp, 0x30($sp)
    /* 3ACEC 8004A4EC 2C00B7AF */  sw         $s7, 0x2C($sp)
    /* 3ACF0 8004A4F0 2800B6AF */  sw         $s6, 0x28($sp)
    /* 3ACF4 8004A4F4 2400B5AF */  sw         $s5, 0x24($sp)
    /* 3ACF8 8004A4F8 2000B4AF */  sw         $s4, 0x20($sp)
    /* 3ACFC 8004A4FC 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 3AD00 8004A500 1800B2AF */  sw         $s2, 0x18($sp)
    /* 3AD04 8004A504 1400B1AF */  sw         $s1, 0x14($sp)
    /* 3AD08 8004A508 1000B0AF */  sw         $s0, 0x10($sp)
    /* 3AD0C 8004A50C 0A80023C */  lui        $v0, %hi(D_800A3820)
    /* 3AD10 8004A510 2038428C */  lw         $v0, %lo(D_800A3820)($v0)
    /* 3AD14 8004A514 1080173C */  lui        $s7, %hi(D_80102C00)
    /* 3AD18 8004A518 002CF726 */  addiu      $s7, $s7, %lo(D_80102C00)
    /* 3AD1C 8004A51C 2C005710 */  beq        $v0, $s7, .L8004A5D0
    /* 3AD20 8004A520 801F113C */   lui       $s1, (0x1F80001C >> 16)
    /* 3AD24 8004A524 21A80000 */  addu       $s5, $zero, $zero
    /* 3AD28 8004A528 10801E3C */  lui        $fp, %hi(D_800FF610)
    /* 3AD2C 8004A52C 10F6DE27 */  addiu      $fp, $fp, %lo(D_800FF610)
    /* 3AD30 8004A530 1C00348E */  lw         $s4, (0x1F80001C & 0xFFFF)($s1)
  .L8004A534:
    /* 3AD34 8004A534 0000F28E */  lw         $s2, 0x0($s7)
    /* 3AD38 8004A538 21B0C003 */  addu       $s6, $fp, $zero
    /* 3AD3C 8004A53C 00004892 */  lbu        $t0, 0x0($s2)
    /* 3AD40 8004A540 00000000 */  nop
    /* 3AD44 8004A544 80400800 */  sll        $t0, $t0, 2
    /* 3AD48 8004A548 0180013C */  lui        $at, %hi(jtbl_8001541C)
    /* 3AD4C 8004A54C 21082800 */  addu       $at, $at, $t0
    /* 3AD50 8004A550 1C54288C */  lw         $t0, %lo(jtbl_8001541C)($at)
    /* 3AD54 8004A554 00000000 */  nop
    /* 3AD58 8004A558 08000001 */  jr         $t0
    /* 3AD5C 8004A55C 0400F726 */   addiu     $s7, $s7, 0x4
  .L8004A560:
    /* 3AD60 8004A560 04004886 */  lh         $t0, 0x4($s2)
    /* 3AD64 8004A564 02004986 */  lh         $t1, 0x2($s2)
    /* 3AD68 8004A568 80400800 */  sll        $t0, $t0, 2
    /* 3AD6C 8004A56C 10800A3C */  lui        $t2, %hi(D_80103608)
    /* 3AD70 8004A570 08364A25 */  addiu      $t2, $t2, %lo(D_80103608)
    /* 3AD74 8004A574 21400A01 */  addu       $t0, $t0, $t2
    /* 3AD78 8004A578 0000088D */  lw         $t0, 0x0($t0)
    /* 3AD7C 8004A57C 80480900 */  sll        $t1, $t1, 2
    /* 3AD80 8004A580 21400901 */  addu       $t0, $t0, $t1
    /* 3AD84 8004A584 0000108D */  lw         $s0, 0x0($t0)
    /* 3AD88 8004A588 00000000 */  nop
  .L8004A58C:
    /* 3AD8C 8004A58C 0000058E */  lw         $a1, 0x0($s0)
    /* 3AD90 8004A590 00000000 */  nop
    /* 3AD94 8004A594 FFFFA430 */  andi       $a0, $a1, 0xFFFF
    /* 3AD98 8004A598 022C0500 */  srl        $a1, $a1, 16
    /* 3AD9C 8004A59C 0C008010 */  beqz       $a0, .L8004A5D0
    /* 3ADA0 8004A5A0 04001022 */   addi      $s0, $s0, 0x4 /* handwritten instruction */
    /* 3ADA4 8004A5A4 80280500 */  sll        $a1, $a1, 2
    /* 3ADA8 8004A5A8 0180013C */  lui        $at, %hi(jtbl_8001545C)
    /* 3ADAC 8004A5AC 21082500 */  addu       $at, $at, $a1
    /* 3ADB0 8004A5B0 5C54258C */  lw         $a1, %lo(jtbl_8001545C)($at)
    /* 3ADB4 8004A5B4 00000000 */  nop
    /* 3ADB8 8004A5B8 0800A000 */  jr         $a1
    /* 3ADBC 8004A5BC 00000000 */   nop
  .L8004A5C0:
    /* 3ADC0 8004A5C0 00000896 */  lhu        $t0, 0x0($s0)
    /* 3ADC4 8004A5C4 02001026 */  addiu      $s0, $s0, 0x2
    /* 3ADC8 8004A5C8 F0FF0015 */  bnez       $t0, .L8004A58C
    /* 3ADCC 8004A5CC 00000000 */   nop
  jlabel .L8004A5D0
    /* 3ADD0 8004A5D0 0A80023C */  lui        $v0, %hi(D_800A3820)
    /* 3ADD4 8004A5D4 2038428C */  lw         $v0, %lo(D_800A3820)($v0)
    /* 3ADD8 8004A5D8 00000000 */  nop
    /* 3ADDC 8004A5DC D5FFE216 */  bne        $s7, $v0, .L8004A534
    /* 3ADE0 8004A5E0 00000000 */   nop
    /* 3ADE4 8004A5E4 3400BF8F */  lw         $ra, 0x34($sp)
    /* 3ADE8 8004A5E8 3000BE8F */  lw         $fp, 0x30($sp)
    /* 3ADEC 8004A5EC 2C00B78F */  lw         $s7, 0x2C($sp)
    /* 3ADF0 8004A5F0 2800B68F */  lw         $s6, 0x28($sp)
    /* 3ADF4 8004A5F4 2400B58F */  lw         $s5, 0x24($sp)
    /* 3ADF8 8004A5F8 2000B48F */  lw         $s4, 0x20($sp)
    /* 3ADFC 8004A5FC 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 3AE00 8004A600 1800B28F */  lw         $s2, 0x18($sp)
    /* 3AE04 8004A604 1400B18F */  lw         $s1, 0x14($sp)
    /* 3AE08 8004A608 1000B08F */  lw         $s0, 0x10($sp)
    /* 3AE0C 8004A60C 0800E003 */  jr         $ra
    /* 3AE10 8004A610 3800BD27 */   addiu     $sp, $sp, 0x38
  jlabel .L8004A614
    /* 3AE14 8004A614 03000824 */  addiu      $t0, $zero, 0x3
    /* 3AE18 8004A618 23401501 */  subu       $t0, $t0, $s5
    /* 3AE1C 8004A61C 801F013C */  lui        $at, (0x1F800008 >> 16)
    /* 3AE20 8004A620 080028AC */  sw         $t0, (0x1F800008 & 0xFFFF)($at)
    /* 3AE24 8004A624 20002526 */  addiu      $a1, $s1, 0x20
    /* 3AE28 8004A628 B4022626 */  addiu      $a2, $s1, 0x2B4
    /* 3AE2C 8004A62C DB29010C */  jal        func_8004A76C
    /* 3AE30 8004A630 00000000 */   nop
    /* 3AE34 8004A634 502A010C */  jal        func_8004A940
    /* 3AE38 8004A638 00000000 */   nop
    /* 3AE3C 8004A63C 70290108 */  j          .L8004A5C0
    /* 3AE40 8004A640 00000000 */   nop
  jlabel .L8004A644
    /* 3AE44 8004A644 03000824 */  addiu      $t0, $zero, 0x3
    /* 3AE48 8004A648 23401501 */  subu       $t0, $t0, $s5
    /* 3AE4C 8004A64C 801F013C */  lui        $at, (0x1F800008 >> 16)
    /* 3AE50 8004A650 080028AC */  sw         $t0, (0x1F800008 & 0xFFFF)($at)
    /* 3AE54 8004A654 20002526 */  addiu      $a1, $s1, 0x20
    /* 3AE58 8004A658 B4022626 */  addiu      $a2, $s1, 0x2B4
    /* 3AE5C 8004A65C 022A010C */  jal        func_8004A808
    /* 3AE60 8004A660 00000000 */   nop
    /* 3AE64 8004A664 01004992 */  lbu        $t1, 0x1($s2)
    /* 3AE68 8004A668 0980143C */  lui        $s4, %hi(D_80095328)
    /* 3AE6C 8004A66C 28539426 */  addiu      $s4, $s4, %lo(D_80095328)
    /* 3AE70 8004A670 01002931 */  andi       $t1, $t1, 0x1
    /* 3AE74 8004A674 02002011 */  beqz       $t1, .L8004A680
    /* 3AE78 8004A678 00000000 */   nop
    /* 3AE7C 8004A67C B0009422 */  addi       $s4, $s4, 0xB0 /* handwritten instruction */
  .L8004A680:
    /* 3AE80 8004A680 502A010C */  jal        func_8004A940
    /* 3AE84 8004A684 00000000 */   nop
    /* 3AE88 8004A688 1C00348E */  lw         $s4, 0x1C($s1)
    /* 3AE8C 8004A68C 70290108 */  j          .L8004A5C0
    /* 3AE90 8004A690 00000000 */   nop
  jlabel .L8004A694
    /* 3AE94 8004A694 03000824 */  addiu      $t0, $zero, 0x3
    /* 3AE98 8004A698 23401501 */  subu       $t0, $t0, $s5
    /* 3AE9C 8004A69C 801F013C */  lui        $at, (0x1F800008 >> 16)
    /* 3AEA0 8004A6A0 080028AC */  sw         $t0, (0x1F800008 & 0xFFFF)($at)
    /* 3AEA4 8004A6A4 20002526 */  addiu      $a1, $s1, 0x20
    /* 3AEA8 8004A6A8 B4022626 */  addiu      $a2, $s1, 0x2B4
    /* 3AEAC 8004A6AC 5000083C */  lui        $t0, (0x500000 >> 16)
    /* 3AEB0 8004A6B0 3C00093C */  lui        $t1, (0x3C0000 >> 16)
    /* 3AEB4 8004A6B4 00C0C848 */  ctc2       $t0, $24 /* handwritten instruction */
    /* 3AEB8 8004A6B8 00C8C948 */  ctc2       $t1, $25 /* handwritten instruction */
    /* 3AEBC 8004A6BC DB29010C */  jal        func_8004A76C
    /* 3AEC0 8004A6C0 00000000 */   nop
    /* 3AEC4 8004A6C4 4001083C */  lui        $t0, (0x1400000 >> 16)
    /* 3AEC8 8004A6C8 7800093C */  lui        $t1, (0x780000 >> 16)
    /* 3AECC 8004A6CC 00C0C848 */  ctc2       $t0, $24 /* handwritten instruction */
    /* 3AED0 8004A6D0 00C8C948 */  ctc2       $t1, $25 /* handwritten instruction */
    /* 3AED4 8004A6D4 0980143C */  lui        $s4, %hi(D_80095508)
    /* 3AED8 8004A6D8 08559426 */  addiu      $s4, $s4, %lo(D_80095508)
    /* 3AEDC 8004A6DC 502A010C */  jal        func_8004A940
    /* 3AEE0 8004A6E0 00000000 */   nop
    /* 3AEE4 8004A6E4 1C00348E */  lw         $s4, 0x1C($s1)
    /* 3AEE8 8004A6E8 70290108 */  j          .L8004A5C0
    /* 3AEEC 8004A6EC 00000000 */   nop
  jlabel .L8004A6F0
    /* 3AEF0 8004A6F0 03000824 */  addiu      $t0, $zero, 0x3
    /* 3AEF4 8004A6F4 23401501 */  subu       $t0, $t0, $s5
    /* 3AEF8 8004A6F8 801F013C */  lui        $at, (0x1F800008 >> 16)
    /* 3AEFC 8004A6FC 080028AC */  sw         $t0, (0x1F800008 & 0xFFFF)($at)
    /* 3AF00 8004A700 20002526 */  addiu      $a1, $s1, 0x20
    /* 3AF04 8004A704 B4022626 */  addiu      $a2, $s1, 0x2B4
    /* 3AF08 8004A708 4E2A010C */  jal        func_8004A938
    /* 3AF0C 8004A70C 00000000 */   nop
    /* 3AF10 8004A710 0980143C */  lui        $s4, %hi(D_80095328)
    /* 3AF14 8004A714 28539426 */  addiu      $s4, $s4, %lo(D_80095328)
    /* 3AF18 8004A718 00000000 */  nop
    /* 3AF1C 8004A71C 502A010C */  jal        func_8004A940
    /* 3AF20 8004A720 00000000 */   nop
    /* 3AF24 8004A724 1C00348E */  lw         $s4, 0x1C($s1)
    /* 3AF28 8004A728 70290108 */  j          .L8004A5C0
    /* 3AF2C 8004A72C 00000000 */   nop
  jlabel .L8004A730
    /* 3AF30 8004A730 03000824 */  addiu      $t0, $zero, 0x3
    /* 3AF34 8004A734 23401501 */  subu       $t0, $t0, $s5
    /* 3AF38 8004A738 801F013C */  lui        $at, (0x1F800008 >> 16)
    /* 3AF3C 8004A73C 080028AC */  sw         $t0, (0x1F800008 & 0xFFFF)($at)
    /* 3AF40 8004A740 20002526 */  addiu      $a1, $s1, 0x20
    /* 3AF44 8004A744 B4022626 */  addiu      $a2, $s1, 0x2B4
    /* 3AF48 8004A748 022A010C */  jal        func_8004A808
    /* 3AF4C 8004A74C 00000000 */   nop
    /* 3AF50 8004A750 0980143C */  lui        $s4, %hi(D_80095488)
    /* 3AF54 8004A754 88549426 */  addiu      $s4, $s4, %lo(D_80095488)
    /* 3AF58 8004A758 502A010C */  jal        func_8004A940
    /* 3AF5C 8004A75C 00000000 */   nop
    /* 3AF60 8004A760 1C00348E */  lw         $s4, 0x1C($s1)
    /* 3AF64 8004A764 70290108 */  j          .L8004A5C0
    /* 3AF68 8004A768 00000000 */   nop
endlabel func_8004A4E0
