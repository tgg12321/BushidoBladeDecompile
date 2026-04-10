glabel func_8007AE7C
    /* 6B67C 8007AE7C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6B680 8007AE80 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6B684 8007AE84 21888000 */  addu       $s1, $a0, $zero
    /* 6B688 8007AE88 07002332 */  andi       $v1, $s1, 0x7
    /* 6B68C 8007AE8C 03000224 */  addiu      $v0, $zero, 0x3
    /* 6B690 8007AE90 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6B694 8007AE94 0D006210 */  beq        $v1, $v0, .L8007AECC
    /* 6B698 8007AE98 1000B0AF */   sw        $s0, 0x10($sp)
    /* 6B69C 8007AE9C 04006228 */  slti       $v0, $v1, 0x4
    /* 6B6A0 8007AEA0 05004010 */  beqz       $v0, .L8007AEB8
    /* 6B6A4 8007AEA4 00000000 */   nop
    /* 6B6A8 8007AEA8 08006010 */  beqz       $v1, .L8007AECC
    /* 6B6AC 8007AEAC 00000000 */   nop
    /* 6B6B0 8007AEB0 E6EB0108 */  j          .L8007AF98
    /* 6B6B4 8007AEB4 00000000 */   nop
  .L8007AEB8:
    /* 6B6B8 8007AEB8 05000224 */  addiu      $v0, $zero, 0x5
    /* 6B6BC 8007AEBC 0B006210 */  beq        $v1, $v0, .L8007AEEC
    /* 6B6C0 8007AEC0 00000000 */   nop
    /* 6B6C4 8007AEC4 E6EB0108 */  j          .L8007AF98
    /* 6B6C8 8007AEC8 00000000 */   nop
  .L8007AECC:
    /* 6B6CC 8007AECC 0180043C */  lui        $a0, %hi(D_80015E5C)
    /* 6B6D0 8007AED0 5C5E8424 */  addiu      $a0, $a0, %lo(D_80015E5C)
    /* 6B6D4 8007AED4 0A80053C */  lui        $a1, %hi(D_8009BE2C)
    /* 6B6D8 8007AED8 2CBEA524 */  addiu      $a1, $a1, %lo(D_8009BE2C)
    /* 6B6DC 8007AEDC 0A80063C */  lui        $a2, %hi(D_8009BE74)
    /* 6B6E0 8007AEE0 74BEC624 */  addiu      $a2, $a2, %lo(D_8009BE74)
    /* 6B6E4 8007AEE4 82E4010C */  jal        debug_printf
    /* 6B6E8 8007AEE8 00000000 */   nop
  .L8007AEEC:
    /* 6B6EC 8007AEEC 0A80103C */  lui        $s0, %hi(D_8009BE74)
    /* 6B6F0 8007AEF0 74BE1026 */  addiu      $s0, $s0, %lo(D_8009BE74)
    /* 6B6F4 8007AEF4 21200002 */  addu       $a0, $s0, $zero
    /* 6B6F8 8007AEF8 21280000 */  addu       $a1, $zero, $zero
    /* 6B6FC 8007AEFC B9F7010C */  jal        func_8007DEE4
    /* 6B700 8007AF00 80000624 */   addiu     $a2, $zero, 0x80
    /* 6B704 8007AF04 B00A020C */  jal        irq_DisableInterrupts
    /* 6B708 8007AF08 00000000 */   nop
    /* 6B70C 8007AF0C FF00023C */  lui        $v0, (0xFFFFFF >> 16)
    /* 6B710 8007AF10 0A80043C */  lui        $a0, %hi(D_8009BE6C)
    /* 6B714 8007AF14 6CBE848C */  lw         $a0, %lo(D_8009BE6C)($a0)
    /* 6B718 8007AF18 FFFF4234 */  ori        $v0, $v0, (0xFFFFFF & 0xFFFF)
    /* 6B71C 8007AF1C C4F7010C */  jal        func_8007DF10
    /* 6B720 8007AF20 24208200 */   and       $a0, $a0, $v0
    /* 6B724 8007AF24 71F6010C */  jal        func_8007D9C4
    /* 6B728 8007AF28 21202002 */   addu      $a0, $s1, $zero
    /* 6B72C 8007AF2C 10000426 */  addiu      $a0, $s0, 0x10
    /* 6B730 8007AF30 01000324 */  addiu      $v1, $zero, 0x1
    /* 6B734 8007AF34 000002A2 */  sb         $v0, 0x0($s0)
    /* 6B738 8007AF38 FF004230 */  andi       $v0, $v0, 0xFF
    /* 6B73C 8007AF3C 80100200 */  sll        $v0, $v0, 2
    /* 6B740 8007AF40 0A80013C */  lui        $at, %hi(D_8009BE75)
    /* 6B744 8007AF44 75BE23A0 */  sb         $v1, %lo(D_8009BE75)($at)
    /* 6B748 8007AF48 0A80013C */  lui        $at, %hi(D_8009BEF4)
    /* 6B74C 8007AF4C 21082200 */  addu       $at, $at, $v0
    /* 6B750 8007AF50 F4BE238C */  lw         $v1, %lo(D_8009BEF4)($at)
    /* 6B754 8007AF54 FFFF0524 */  addiu      $a1, $zero, -0x1
    /* 6B758 8007AF58 0A80013C */  lui        $at, %hi(D_8009BE78)
    /* 6B75C 8007AF5C 78BE23A4 */  sh         $v1, %lo(D_8009BE78)($at)
    /* 6B760 8007AF60 0A80013C */  lui        $at, %hi(D_8009BF08)
    /* 6B764 8007AF64 21082200 */  addu       $at, $at, $v0
    /* 6B768 8007AF68 08BF228C */  lw         $v0, %lo(D_8009BF08)($at)
    /* 6B76C 8007AF6C 0A80013C */  lui        $at, %hi(D_8009BE7A)
    /* 6B770 8007AF70 7ABE22A4 */  sh         $v0, %lo(D_8009BE7A)($at)
    /* 6B774 8007AF74 B9F7010C */  jal        func_8007DEE4
    /* 6B778 8007AF78 5C000624 */   addiu     $a2, $zero, 0x5C
    /* 6B77C 8007AF7C 6C000426 */  addiu      $a0, $s0, 0x6C
    /* 6B780 8007AF80 FFFF0524 */  addiu      $a1, $zero, -0x1
    /* 6B784 8007AF84 B9F7010C */  jal        func_8007DEE4
    /* 6B788 8007AF88 14000624 */   addiu     $a2, $zero, 0x14
    /* 6B78C 8007AF8C 00000292 */  lbu        $v0, 0x0($s0)
    /* 6B790 8007AF90 FAEB0108 */  j          .L8007AFE8
    /* 6B794 8007AF94 00000000 */   nop
  .L8007AF98:
    /* 6B798 8007AF98 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6B79C 8007AF9C 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6B7A0 8007AFA0 00000000 */  nop
    /* 6B7A4 8007AFA4 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6B7A8 8007AFA8 08004014 */  bnez       $v0, .L8007AFCC
    /* 6B7AC 8007AFAC 00000000 */   nop
    /* 6B7B0 8007AFB0 0180043C */  lui        $a0, %hi(D_80015E7C)
    /* 6B7B4 8007AFB4 7C5E8424 */  addiu      $a0, $a0, %lo(D_80015E7C)
    /* 6B7B8 8007AFB8 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6B7BC 8007AFBC 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6B7C0 8007AFC0 00000000 */  nop
    /* 6B7C4 8007AFC4 09F84000 */  jalr       $v0
    /* 6B7C8 8007AFC8 21282002 */   addu      $a1, $s1, $zero
  .L8007AFCC:
    /* 6B7CC 8007AFCC 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6B7D0 8007AFD0 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6B7D4 8007AFD4 00000000 */  nop
    /* 6B7D8 8007AFD8 3400428C */  lw         $v0, 0x34($v0)
    /* 6B7DC 8007AFDC 00000000 */  nop
    /* 6B7E0 8007AFE0 09F84000 */  jalr       $v0
    /* 6B7E4 8007AFE4 01000424 */   addiu     $a0, $zero, 0x1
  .L8007AFE8:
    /* 6B7E8 8007AFE8 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6B7EC 8007AFEC 1400B18F */  lw         $s1, 0x14($sp)
    /* 6B7F0 8007AFF0 1000B08F */  lw         $s0, 0x10($sp)
    /* 6B7F4 8007AFF4 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6B7F8 8007AFF8 0800E003 */  jr         $ra
    /* 6B7FC 8007AFFC 00000000 */   nop
endlabel func_8007AE7C
