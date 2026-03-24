glabel func_8006E8CC
    /* 5F0CC 8006E8CC E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 5F0D0 8006E8D0 1800B0AF */  sw         $s0, 0x18($sp)
    /* 5F0D4 8006E8D4 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 5F0D8 8006E8D8 40DF010C */  jal        func_80077D00
    /* 5F0DC 8006E8DC 21808000 */   addu      $s0, $a0, $zero
    /* 5F0E0 8006E8E0 2000428C */  lw         $v0, 0x20($v0)
    /* 5F0E4 8006E8E4 00000000 */  nop
    /* 5F0E8 8006E8E8 01004230 */  andi       $v0, $v0, 0x1
    /* 5F0EC 8006E8EC 04004010 */  beqz       $v0, .L8006E900
    /* 5F0F0 8006E8F0 00000000 */   nop
    /* 5F0F4 8006E8F4 1000108E */  lw         $s0, 0x10($s0)
    /* 5F0F8 8006E8F8 42BA0108 */  j          .L8006E908
    /* 5F0FC 8006E8FC 21200000 */   addu      $a0, $zero, $zero
  .L8006E900:
    /* 5F100 8006E900 0C00108E */  lw         $s0, 0xC($s0)
    /* 5F104 8006E904 21200000 */  addu       $a0, $zero, $zero
  .L8006E908:
    /* 5F108 8006E908 E0010224 */  addiu      $v0, $zero, 0x1E0
    /* 5F10C 8006E90C 1200A2A7 */  sh         $v0, 0x12($sp)
    /* 5F110 8006E910 80020224 */  addiu      $v0, $zero, 0x280
    /* 5F114 8006E914 1400A2A7 */  sh         $v0, 0x14($sp)
    /* 5F118 8006E918 20000224 */  addiu      $v0, $zero, 0x20
    /* 5F11C 8006E91C 1000A0A7 */  sh         $zero, 0x10($sp)
    /* 5F120 8006E920 CFEC010C */  jal        func_8007B33C
    /* 5F124 8006E924 1600A2A7 */   sh        $v0, 0x16($sp)
    /* 5F128 8006E928 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5F12C 8006E92C 80ED010C */  jal        func_8007B600
    /* 5F130 8006E930 21280002 */   addu      $a1, $s0, $zero
    /* 5F134 8006E934 CFEC010C */  jal        func_8007B33C
    /* 5F138 8006E938 21200000 */   addu      $a0, $zero, $zero
    /* 5F13C 8006E93C 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 5F140 8006E940 1800B08F */  lw         $s0, 0x18($sp)
    /* 5F144 8006E944 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 5F148 8006E948 0800E003 */  jr         $ra
    /* 5F14C 8006E94C 00000000 */   nop
endlabel func_8006E8CC
