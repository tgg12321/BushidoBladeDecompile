glabel func_8007B6C8
    /* 6BEC8 8007B6C8 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6BECC 8007B6CC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BED0 8007B6D0 21808000 */  addu       $s0, $a0, $zero
    /* 6BED4 8007B6D4 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6BED8 8007B6D8 2190A000 */  addu       $s2, $a1, $zero
    /* 6BEDC 8007B6DC 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BEE0 8007B6E0 2188C000 */  addu       $s1, $a2, $zero
    /* 6BEE4 8007B6E4 0180043C */  lui        $a0, %hi(D_80015F74)
    /* 6BEE8 8007B6E8 745F8424 */  addiu      $a0, $a0, %lo(D_80015F74)
    /* 6BEEC 8007B6EC 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 6BEF0 8007B6F0 EAEC010C */  jal        func_8007B3A8
    /* 6BEF4 8007B6F4 21280002 */   addu      $a1, $s0, $zero
    /* 6BEF8 8007B6F8 04000286 */  lh         $v0, 0x4($s0)
    /* 6BEFC 8007B6FC 00000000 */  nop
    /* 6BF00 8007B700 1B004010 */  beqz       $v0, .L8007B770
    /* 6BF04 8007B704 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 6BF08 8007B708 06000286 */  lh         $v0, 0x6($s0)
    /* 6BF0C 8007B70C 00000000 */  nop
    /* 6BF10 8007B710 03004014 */  bnez       $v0, .L8007B720
    /* 6BF14 8007B714 00141100 */   sll       $v0, $s1, 16
    /* 6BF18 8007B718 DCED0108 */  j          .L8007B770
    /* 6BF1C 8007B71C FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8007B720:
    /* 6BF20 8007B720 FFFF4332 */  andi       $v1, $s2, 0xFFFF
    /* 6BF24 8007B724 25104300 */  or         $v0, $v0, $v1
    /* 6BF28 8007B728 0A80053C */  lui        $a1, %hi(D_8009BF24)
    /* 6BF2C 8007B72C 24BFA524 */  addiu      $a1, $a1, %lo(D_8009BF24)
    /* 6BF30 8007B730 0000048E */  lw         $a0, 0x0($s0)
    /* 6BF34 8007B734 0A80033C */  lui        $v1, %hi(D_8009BE6C)
    /* 6BF38 8007B738 6CBE638C */  lw         $v1, %lo(D_8009BE6C)($v1)
    /* 6BF3C 8007B73C 14000624 */  addiu      $a2, $zero, 0x14
    /* 6BF40 8007B740 0A80013C */  lui        $at, %hi(D_8009BF28)
    /* 6BF44 8007B744 28BF22AC */  sw         $v0, %lo(D_8009BF28)($at)
    /* 6BF48 8007B748 0000A4AC */  sw         $a0, 0x0($a1)
    /* 6BF4C 8007B74C 0400028E */  lw         $v0, 0x4($s0)
    /* 6BF50 8007B750 21380000 */  addu       $a3, $zero, $zero
    /* 6BF54 8007B754 0A80013C */  lui        $at, %hi(D_8009BF2C)
    /* 6BF58 8007B758 2CBF22AC */  sw         $v0, %lo(D_8009BF2C)($at)
    /* 6BF5C 8007B75C 1800648C */  lw         $a0, 0x18($v1)
    /* 6BF60 8007B760 0800628C */  lw         $v0, 0x8($v1)
    /* 6BF64 8007B764 00000000 */  nop
    /* 6BF68 8007B768 09F84000 */  jalr       $v0
    /* 6BF6C 8007B76C F8FFA524 */   addiu     $a1, $a1, -0x8
  .L8007B770:
    /* 6BF70 8007B770 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 6BF74 8007B774 1800B28F */  lw         $s2, 0x18($sp)
    /* 6BF78 8007B778 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BF7C 8007B77C 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BF80 8007B780 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6BF84 8007B784 0800E003 */  jr         $ra
    /* 6BF88 8007B788 00000000 */   nop
endlabel func_8007B6C8
