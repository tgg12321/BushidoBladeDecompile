glabel func_8003F62C
    /* 2FE2C 8003F62C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2FE30 8003F630 1400B1AF */  sw         $s1, 0x14($sp)
    /* 2FE34 8003F634 21888000 */  addu       $s1, $a0, $zero
    /* 2FE38 8003F638 1800BFAF */  sw         $ra, 0x18($sp)
    /* 2FE3C 8003F63C 1000B0AF */  sw         $s0, 0x10($sp)
    /* 2FE40 8003F640 2400308E */  lw         $s0, 0x24($s1)
    /* 2FE44 8003F644 00000000 */  nop
    /* 2FE48 8003F648 1D000012 */  beqz       $s0, .L8003F6C0
    /* 2FE4C 8003F64C 00000000 */   nop
    /* 2FE50 8003F650 06000286 */  lh         $v0, 0x6($s0)
    /* 2FE54 8003F654 00000000 */  nop
    /* 2FE58 8003F658 07004010 */  beqz       $v0, .L8003F678
    /* 2FE5C 8003F65C 00000000 */   nop
    /* 2FE60 8003F660 04002486 */  lh         $a0, 0x4($s1)
    /* 2FE64 8003F664 5B00010C */  jal        func_8004016C
    /* 2FE68 8003F668 00000000 */   nop
    /* 2FE6C 8003F66C 21202002 */  addu       $a0, $s1, $zero
    /* 2FE70 8003F670 09FE000C */  jal        tslPrintScreen
    /* 2FE74 8003F674 21280000 */   addu      $a1, $zero, $zero
  .L8003F678:
    /* 2FE78 8003F678 02000286 */  lh         $v0, 0x2($s0)
    /* 2FE7C 8003F67C 00000000 */  nop
    /* 2FE80 8003F680 03004010 */  beqz       $v0, .L8003F690
    /* 2FE84 8003F684 00000000 */   nop
    /* 2FE88 8003F688 0700010C */  jal        func_8004001C
    /* 2FE8C 8003F68C 21200002 */   addu      $a0, $s0, $zero
  .L8003F690:
    /* 2FE90 8003F690 B6FD000C */  jal        md_game_restart_init2
    /* 2FE94 8003F694 21200002 */   addu      $a0, $s0, $zero
    /* 2FE98 8003F698 00000586 */  lh         $a1, 0x0($s0)
    /* 2FE9C 8003F69C 9364000C */  jal        func_8001924C
    /* 2FEA0 8003F6A0 18040426 */   addiu     $a0, $s0, 0x418
    /* 2FEA4 8003F6A4 02000286 */  lh         $v0, 0x2($s0)
    /* 2FEA8 8003F6A8 00000000 */  nop
    /* 2FEAC 8003F6AC 04004010 */  beqz       $v0, .L8003F6C0
    /* 2FEB0 8003F6B0 00000000 */   nop
    /* 2FEB4 8003F6B4 1A00010C */  jal        func_80040068
    /* 2FEB8 8003F6B8 21200002 */   addu      $a0, $s0, $zero
    /* 2FEBC 8003F6BC 020000A6 */  sh         $zero, 0x2($s0)
  .L8003F6C0:
    /* 2FEC0 8003F6C0 1800BF8F */  lw         $ra, 0x18($sp)
    /* 2FEC4 8003F6C4 1400B18F */  lw         $s1, 0x14($sp)
    /* 2FEC8 8003F6C8 1000B08F */  lw         $s0, 0x10($sp)
    /* 2FECC 8003F6CC 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 2FED0 8003F6D0 0800E003 */  jr         $ra
    /* 2FED4 8003F6D4 00000000 */   nop
endlabel func_8003F62C
