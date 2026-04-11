glabel func_8003ACB8
    /* 2B4B8 8003ACB8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2B4BC 8003ACBC 1400BFAF */  sw         $ra, 0x14($sp)
    /* 2B4C0 8003ACC0 B8DE010C */  jal        func_80077AE0
    /* 2B4C4 8003ACC4 1000B0AF */   sw        $s0, 0x10($sp)
    /* 2B4C8 8003ACC8 A8EC010C */  jal        gpu_SetDispMask
    /* 2B4CC 8003ACCC 21200000 */   addu      $a0, $zero, $zero
    /* 2B4D0 8003ACD0 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2B4D4 8003ACD4 B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* 2B4D8 8003ACD8 E00780A3 */  sb         $zero, %gp_rel(D_800A38AC)($gp)
    /* 2B4DC 8003ACDC 040880AF */  sw         $zero, %gp_rel(D_800A38D0)($gp)
    /* 2B4E0 8003ACE0 3C0880AF */  sw         $zero, %gp_rel(D_800A3908)($gp)
    /* 2B4E4 8003ACE4 300880AF */  sw         $zero, %gp_rel(D_800A38FC)($gp)
    /* 2B4E8 8003ACE8 5DE8000C */  jal        func_8003A174
    /* 2B4EC 8003ACEC 00000000 */   nop
  .L8003ACF0:
    /* 2B4F0 8003ACF0 5A65000C */  jal        single_game_VoiceContorol
    /* 2B4F4 8003ACF4 01000424 */   addiu     $a0, $zero, 0x1
    /* 2B4F8 8003ACF8 B471010C */  jal        func_8005C6D0
    /* 2B4FC 8003ACFC 00000000 */   nop
    /* 2B500 8003AD00 D1EA000C */  jal        func_8003AB44
    /* 2B504 8003AD04 00000000 */   nop
    /* 2B508 8003AD08 02000424 */  addiu      $a0, $zero, 0x2
    /* 2B50C 8003AD0C 330A020C */  jal        sys_VSync
    /* 2B510 8003AD10 21804000 */   addu      $s0, $v0, $zero
    /* 2B514 8003AD14 F6FF0012 */  beqz       $s0, .L8003ACF0
    /* 2B518 8003AD18 00000000 */   nop
    /* 2B51C 8003AD1C C0DE010C */  jal        func_80077B00
    /* 2B520 8003AD20 00000000 */   nop
    /* 2B524 8003AD24 3D65000C */  jal        func_800194F4
    /* 2B528 8003AD28 00000000 */   nop
    /* 2B52C 8003AD2C 1080043C */  lui        $a0, %hi(D_80106A50)
    /* 2B530 8003AD30 506A848C */  lw         $a0, %lo(D_80106A50)($a0)
    /* 2B534 8003AD34 A2D7000C */  jal        func_80035E88
    /* 2B538 8003AD38 00000000 */   nop
    /* 2B53C 8003AD3C F80682A7 */  sh         $v0, %gp_rel(D_800A37C4)($gp)
    /* 2B540 8003AD40 92EA000C */  jal        func_8003AA48
    /* 2B544 8003AD44 00000000 */   nop
    /* 2B548 8003AD48 330A020C */  jal        sys_VSync
    /* 2B54C 8003AD4C 01000424 */   addiu     $a0, $zero, 0x1
    /* 2B550 8003AD50 92EA000C */  jal        func_8003AA48
    /* 2B554 8003AD54 00000000 */   nop
    /* 2B558 8003AD58 0A80043C */  lui        $a0, %hi(D_800A36C6)
    /* 2B55C 8003AD5C C6368484 */  lh         $a0, %lo(D_800A36C6)($a0)
    /* 2B560 8003AD60 B7D7000C */  jal        func_80035EDC
    /* 2B564 8003AD64 00000000 */   nop
    /* 2B568 8003AD68 0A80013C */  lui        $at, %hi(D_800A38E4)
    /* 2B56C 8003AD6C E43822AC */  sw         $v0, %lo(D_800A38E4)($at)
    /* 2B570 8003AD70 2265000C */  jal        func_80019488
    /* 2B574 8003AD74 00000000 */   nop
    /* 2B578 8003AD78 F80682A7 */  sh         $v0, %gp_rel(D_800A37C4)($gp)
    /* 2B57C 8003AD7C 330A020C */  jal        sys_VSync
    /* 2B580 8003AD80 01000424 */   addiu     $a0, $zero, 0x1
    /* 2B584 8003AD84 92EA000C */  jal        func_8003AA48
    /* 2B588 8003AD88 00000000 */   nop
    /* 2B58C 8003AD8C 330A020C */  jal        sys_VSync
    /* 2B590 8003AD90 01000424 */   addiu     $a0, $zero, 0x1
    /* 2B594 8003AD94 92EA000C */  jal        func_8003AA48
    /* 2B598 8003AD98 00000000 */   nop
    /* 2B59C 8003AD9C 330A020C */  jal        sys_VSync
    /* 2B5A0 8003ADA0 01000424 */   addiu     $a0, $zero, 0x1
    /* 2B5A4 8003ADA4 92EA000C */  jal        func_8003AA48
    /* 2B5A8 8003ADA8 00000000 */   nop
    /* 2B5AC 8003ADAC 0A80043C */  lui        $a0, %hi(D_800A36C6)
    /* 2B5B0 8003ADB0 C6368484 */  lh         $a0, %lo(D_800A36C6)($a0)
    /* 2B5B4 8003ADB4 3065000C */  jal        func_800194C0
    /* 2B5B8 8003ADB8 00000000 */   nop
    /* 2B5BC 8003ADBC 55E4010C */  jal        func_80079154
    /* 2B5C0 8003ADC0 00000000 */   nop
    /* 2B5C4 8003ADC4 F80682A7 */  sh         $v0, %gp_rel(D_800A37C4)($gp)
    /* 2B5C8 8003ADC8 330A020C */  jal        sys_VSync
    /* 2B5CC 8003ADCC 01000424 */   addiu     $a0, $zero, 0x1
    /* 2B5D0 8003ADD0 92EA000C */  jal        func_8003AA48
    /* 2B5D4 8003ADD4 00000000 */   nop
    /* 2B5D8 8003ADD8 330A020C */  jal        sys_VSync
    /* 2B5DC 8003ADDC 01000424 */   addiu     $a0, $zero, 0x1
    /* 2B5E0 8003ADE0 92EA000C */  jal        func_8003AA48
    /* 2B5E4 8003ADE4 00000000 */   nop
    /* 2B5E8 8003ADE8 330A020C */  jal        sys_VSync
    /* 2B5EC 8003ADEC 01000424 */   addiu     $a0, $zero, 0x1
    /* 2B5F0 8003ADF0 92EA000C */  jal        func_8003AA48
    /* 2B5F4 8003ADF4 00000000 */   nop
    /* 2B5F8 8003ADF8 0A80023C */  lui        $v0, %hi(D_800A38A0)
    /* 2B5FC 8003ADFC A038428C */  lw         $v0, %lo(D_800A38A0)($v0)
    /* 2B600 8003AE00 00000000 */  nop
    /* 2B604 8003AE04 04004014 */  bnez       $v0, .L8003AE18
    /* 2B608 8003AE08 00000000 */   nop
    /* 2B60C 8003AE0C F8068297 */  lhu        $v0, %gp_rel(D_800A37C4)($gp)
    /* 2B610 8003AE10 88EB0008 */  j          .L8003AE20
    /* 2B614 8003AE14 00000000 */   nop
  .L8003AE18:
    /* 2B618 8003AE18 0A80023C */  lui        $v0, %hi(D_800A36C6)
    /* 2B61C 8003AE1C C6364284 */  lh         $v0, %lo(D_800A36C6)($v0)
  .L8003AE20:
    /* 2B620 8003AE20 0A80013C */  lui        $at, %hi(D_800A3904)
    /* 2B624 8003AE24 043922A4 */  sh         $v0, %lo(D_800A3904)($at)
    /* 2B628 8003AE28 225A000C */  jal        gpu_InitDisplay
    /* 2B62C 8003AE2C 00000000 */   nop
    /* 2B630 8003AE30 345A000C */  jal        gpu_DisableDisplay
    /* 2B634 8003AE34 00000000 */   nop
    /* 2B638 8003AE38 00F2043C */  lui        $a0, (0xF2000001 >> 16)
    /* 2B63C 8003AE3C EAE2010C */  jal        func_80078BA8
    /* 2B640 8003AE40 01008434 */   ori       $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 2B644 8003AE44 21100002 */  addu       $v0, $s0, $zero
    /* 2B648 8003AE48 1400BF8F */  lw         $ra, 0x14($sp)
    /* 2B64C 8003AE4C 1000B08F */  lw         $s0, 0x10($sp)
    /* 2B650 8003AE50 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2B654 8003AE54 0800E003 */  jr         $ra
    /* 2B658 8003AE58 00000000 */   nop
endlabel func_8003ACB8
