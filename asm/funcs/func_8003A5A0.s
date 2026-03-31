glabel func_8003A5A0
    /* 2ADA0 8003A5A0 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2ADA4 8003A5A4 1400B1AF */  sw         $s1, 0x14($sp)
    /* 2ADA8 8003A5A8 21880000 */  addu       $s1, $zero, $zero
    /* 2ADAC 8003A5AC 00F2043C */  lui        $a0, (0xF2000001 >> 16)
    /* 2ADB0 8003A5B0 01008434 */  ori        $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 2ADB4 8003A5B4 1800BFAF */  sw         $ra, 0x18($sp)
    /* 2ADB8 8003A5B8 C1E2010C */  jal        func_80078B04
    /* 2ADBC 8003A5BC 1000B0AF */   sw        $s0, 0x10($sp)
    /* 2ADC0 8003A5C0 21804000 */  addu       $s0, $v0, $zero
    /* 2ADC4 8003A5C4 0104022A */  slti       $v0, $s0, 0x401
    /* 2ADC8 8003A5C8 04004014 */  bnez       $v0, .L8003A5DC
    /* 2ADCC 8003A5CC 00F2043C */   lui       $a0, (0xF2000001 >> 16)
  .L8003A5D0:
    /* 2ADD0 8003A5D0 EAE2010C */  jal        func_80078BA8
    /* 2ADD4 8003A5D4 01008434 */   ori       $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 2ADD8 8003A5D8 21800000 */  addu       $s0, $zero, $zero
  .L8003A5DC:
    /* 2ADDC 8003A5DC 6C06848F */  lw         $a0, %gp_rel(D_800A3738)($gp)
    /* 2ADE0 8003A5E0 66E2010C */  jal        func_80078998
    /* 2ADE4 8003A5E4 00000000 */   nop
    /* 2ADE8 8003A5E8 27004014 */  bnez       $v0, .L8003A688
    /* 2ADEC 8003A5EC 00000000 */   nop
    /* 2ADF0 8003A5F0 4407848F */  lw         $a0, %gp_rel(D_800A3810)($gp)
    /* 2ADF4 8003A5F4 66E2010C */  jal        func_80078998
    /* 2ADF8 8003A5F8 00000000 */   nop
    /* 2ADFC 8003A5FC 0E004010 */  beqz       $v0, .L8003A638
    /* 2AE00 8003A600 21200000 */   addu      $a0, $zero, $zero
    /* 2AE04 8003A604 01003126 */  addiu      $s1, $s1, 0x1
    /* 2AE08 8003A608 0500222A */  slti       $v0, $s1, 0x5
    /* 2AE0C 8003A60C 2F004010 */  beqz       $v0, .L8003A6CC
    /* 2AE10 8003A610 02000424 */   addiu     $a0, $zero, 0x2
    /* 2AE14 8003A614 21280000 */  addu       $a1, $zero, $zero
    /* 2AE18 8003A618 1931020C */  jal        func_8008C464
    /* 2AE1C 8003A61C 21300000 */   addu      $a2, $zero, $zero
    /* 2AE20 8003A620 5DE9000C */  jal        func_8003A574
    /* 2AE24 8003A624 21800000 */   addu      $s0, $zero, $zero
    /* 2AE28 8003A628 00F2043C */  lui        $a0, (0xF2000001 >> 16)
    /* 2AE2C 8003A62C EAE2010C */  jal        func_80078BA8
    /* 2AE30 8003A630 01008434 */   ori       $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 2AE34 8003A634 21200000 */  addu       $a0, $zero, $zero
  .L8003A638:
    /* 2AE38 8003A638 21280000 */  addu       $a1, $zero, $zero
    /* 2AE3C 8003A63C 1931020C */  jal        func_8008C464
    /* 2AE40 8003A640 21300000 */   addu      $a2, $zero, $zero
    /* 2AE44 8003A644 C3110200 */  sra        $v0, $v0, 7
    /* 2AE48 8003A648 03004230 */  andi       $v0, $v0, 0x3
    /* 2AE4C 8003A64C 01000324 */  addiu      $v1, $zero, 0x1
    /* 2AE50 8003A650 E2FF4310 */  beq        $v0, $v1, .L8003A5DC
    /* 2AE54 8003A654 00F2043C */   lui       $a0, (0xF2000001 >> 16)
    /* 2AE58 8003A658 C1E2010C */  jal        func_80078B04
    /* 2AE5C 8003A65C 01008434 */   ori       $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 2AE60 8003A660 23105000 */  subu       $v0, $v0, $s0
    /* 2AE64 8003A664 013C4228 */  slti       $v0, $v0, 0x3C01
    /* 2AE68 8003A668 DCFF4014 */  bnez       $v0, .L8003A5DC
    /* 2AE6C 8003A66C 00000000 */   nop
    /* 2AE70 8003A670 01003126 */  addiu      $s1, $s1, 0x1
    /* 2AE74 8003A674 0500222A */  slti       $v0, $s1, 0x5
    /* 2AE78 8003A678 1A004010 */  beqz       $v0, .L8003A6E4
    /* 2AE7C 8003A67C 21100000 */   addu      $v0, $zero, $zero
    /* 2AE80 8003A680 74E90008 */  j          .L8003A5D0
    /* 2AE84 8003A684 00F2043C */   lui       $a0, (0xF2000001 >> 16)
  .L8003A688:
    /* 2AE88 8003A688 BC05858F */  lw         $a1, %gp_rel(D_800A3688)($gp)
    /* 2AE8C 8003A68C 0A80043C */  lui        $a0, %hi(D_800A368C)
    /* 2AE90 8003A690 8C36848C */  lw         $a0, %lo(D_800A368C)($a0)
    /* 2AE94 8003A694 03140500 */  sra        $v0, $a1, 16
    /* 2AE98 8003A698 2610A200 */  xor        $v0, $a1, $v0
    /* 2AE9C 8003A69C 031C0400 */  sra        $v1, $a0, 16
    /* 2AEA0 8003A6A0 26104300 */  xor        $v0, $v0, $v1
    /* 2AEA4 8003A6A4 FFFF4230 */  andi       $v0, $v0, 0xFFFF
    /* 2AEA8 8003A6A8 FFFF8330 */  andi       $v1, $a0, 0xFFFF
    /* 2AEAC 8003A6AC 09006210 */  beq        $v1, $v0, .L8003A6D4
    /* 2AEB0 8003A6B0 21100000 */   addu      $v0, $zero, $zero
    /* 2AEB4 8003A6B4 0408838F */  lw         $v1, %gp_rel(D_800A38D0)($gp)
    /* 2AEB8 8003A6B8 00000000 */  nop
    /* 2AEBC 8003A6BC 01006324 */  addiu      $v1, $v1, 0x1
    /* 2AEC0 8003A6C0 040883AF */  sw         $v1, %gp_rel(D_800A38D0)($gp)
    /* 2AEC4 8003A6C4 B9E90008 */  j          .L8003A6E4
    /* 2AEC8 8003A6C8 00000000 */   nop
  .L8003A6CC:
    /* 2AECC 8003A6CC B9E90008 */  j          .L8003A6E4
    /* 2AED0 8003A6D0 21100000 */   addu      $v0, $zero, $zero
  .L8003A6D4:
    /* 2AED4 8003A6D4 01000224 */  addiu      $v0, $zero, 0x1
    /* 2AED8 8003A6D8 F40585AF */  sw         $a1, %gp_rel(D_800A36C0)($gp)
    /* 2AEDC 8003A6DC 0A80013C */  lui        $at, %hi(D_800A36C4)
    /* 2AEE0 8003A6E0 C43624AC */  sw         $a0, %lo(D_800A36C4)($at)
  .L8003A6E4:
    /* 2AEE4 8003A6E4 1800BF8F */  lw         $ra, 0x18($sp)
    /* 2AEE8 8003A6E8 1400B18F */  lw         $s1, 0x14($sp)
    /* 2AEEC 8003A6EC 1000B08F */  lw         $s0, 0x10($sp)
    /* 2AEF0 8003A6F0 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 2AEF4 8003A6F4 0800E003 */  jr         $ra
    /* 2AEF8 8003A6F8 00000000 */   nop
endlabel func_8003A5A0
