glabel func_8007ACBC
    /* 6B4BC 8007ACBC E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6B4C0 8007ACC0 1800B0AF */  sw         $s0, 0x18($sp)
    /* 6B4C4 8007ACC4 21808000 */  addu       $s0, $a0, $zero
    /* 6B4C8 8007ACC8 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 6B4CC 8007ACCC 00000586 */  lh         $a1, 0x0($s0)
    /* 6B4D0 8007ACD0 02000686 */  lh         $a2, 0x2($s0)
    /* 6B4D4 8007ACD4 04000786 */  lh         $a3, 0x4($s0)
    /* 6B4D8 8007ACD8 06000286 */  lh         $v0, 0x6($s0)
    /* 6B4DC 8007ACDC 0A80033C */  lui        $v1, %hi(D_8009BE70)
    /* 6B4E0 8007ACE0 70BE638C */  lw         $v1, %lo(D_8009BE70)($v1)
    /* 6B4E4 8007ACE4 0180043C */  lui        $a0, %hi(D_80015D80)
    /* 6B4E8 8007ACE8 805D8424 */  addiu      $a0, $a0, %lo(D_80015D80)
    /* 6B4EC 8007ACEC 09F86000 */  jalr       $v1
    /* 6B4F0 8007ACF0 1000A2AF */   sw        $v0, 0x10($sp)
    /* 6B4F4 8007ACF4 08000586 */  lh         $a1, 0x8($s0)
    /* 6B4F8 8007ACF8 0A000686 */  lh         $a2, 0xA($s0)
    /* 6B4FC 8007ACFC 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6B500 8007AD00 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6B504 8007AD04 0180043C */  lui        $a0, %hi(D_80015D98)
    /* 6B508 8007AD08 985D8424 */  addiu      $a0, $a0, %lo(D_80015D98)
    /* 6B50C 8007AD0C 09F84000 */  jalr       $v0
    /* 6B510 8007AD10 00000000 */   nop
    /* 6B514 8007AD14 0C000586 */  lh         $a1, 0xC($s0)
    /* 6B518 8007AD18 0E000686 */  lh         $a2, 0xE($s0)
    /* 6B51C 8007AD1C 10000786 */  lh         $a3, 0x10($s0)
    /* 6B520 8007AD20 12000286 */  lh         $v0, 0x12($s0)
    /* 6B524 8007AD24 0A80033C */  lui        $v1, %hi(D_8009BE70)
    /* 6B528 8007AD28 70BE638C */  lw         $v1, %lo(D_8009BE70)($v1)
    /* 6B52C 8007AD2C 0180043C */  lui        $a0, %hi(D_80015DA8)
    /* 6B530 8007AD30 A85D8424 */  addiu      $a0, $a0, %lo(D_80015DA8)
    /* 6B534 8007AD34 09F86000 */  jalr       $v1
    /* 6B538 8007AD38 1000A2AF */   sw        $v0, 0x10($sp)
    /* 6B53C 8007AD3C 16000592 */  lbu        $a1, 0x16($s0)
    /* 6B540 8007AD40 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6B544 8007AD44 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6B548 8007AD48 0180043C */  lui        $a0, %hi(D_80015DC0)
    /* 6B54C 8007AD4C C05D8424 */  addiu      $a0, $a0, %lo(D_80015DC0)
    /* 6B550 8007AD50 09F84000 */  jalr       $v0
    /* 6B554 8007AD54 00000000 */   nop
    /* 6B558 8007AD58 17000592 */  lbu        $a1, 0x17($s0)
    /* 6B55C 8007AD5C 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6B560 8007AD60 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6B564 8007AD64 0180043C */  lui        $a0, %hi(D_80015DCC)
    /* 6B568 8007AD68 CC5D8424 */  addiu      $a0, $a0, %lo(D_80015DCC)
    /* 6B56C 8007AD6C 09F84000 */  jalr       $v0
    /* 6B570 8007AD70 00000000 */   nop
    /* 6B574 8007AD74 0180043C */  lui        $a0, %hi(D_80015D58)
    /* 6B578 8007AD78 585D8424 */  addiu      $a0, $a0, %lo(D_80015D58)
    /* 6B57C 8007AD7C 14000296 */  lhu        $v0, 0x14($s0)
    /* 6B580 8007AD80 0A80083C */  lui        $t0, %hi(D_8009BE70)
    /* 6B584 8007AD84 70BE088D */  lw         $t0, %lo(D_8009BE70)($t0)
    /* 6B588 8007AD88 C2290200 */  srl        $a1, $v0, 7
    /* 6B58C 8007AD8C 0300A530 */  andi       $a1, $a1, 0x3
    /* 6B590 8007AD90 42310200 */  srl        $a2, $v0, 5
    /* 6B594 8007AD94 0300C630 */  andi       $a2, $a2, 0x3
    /* 6B598 8007AD98 80390200 */  sll        $a3, $v0, 6
    /* 6B59C 8007AD9C C007E730 */  andi       $a3, $a3, 0x7C0
    /* 6B5A0 8007ADA0 00190200 */  sll        $v1, $v0, 4
    /* 6B5A4 8007ADA4 00016330 */  andi       $v1, $v1, 0x100
    /* 6B5A8 8007ADA8 82100200 */  srl        $v0, $v0, 2
    /* 6B5AC 8007ADAC 00024230 */  andi       $v0, $v0, 0x200
    /* 6B5B0 8007ADB0 21186200 */  addu       $v1, $v1, $v0
    /* 6B5B4 8007ADB4 09F80001 */  jalr       $t0
    /* 6B5B8 8007ADB8 1000A3AF */   sw        $v1, 0x10($sp)
    /* 6B5BC 8007ADBC 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 6B5C0 8007ADC0 1800B08F */  lw         $s0, 0x18($sp)
    /* 6B5C4 8007ADC4 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6B5C8 8007ADC8 0800E003 */  jr         $ra
    /* 6B5CC 8007ADCC 00000000 */   nop
endlabel func_8007ACBC
