glabel func_8005BE84
    /* 4C684 8005BE84 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 4C688 8005BE88 1000B0AF */  sw         $s0, 0x10($sp)
    /* 4C68C 8005BE8C 21808000 */  addu       $s0, $a0, $zero
    /* 4C690 8005BE90 21200000 */  addu       $a0, $zero, $zero
    /* 4C694 8005BE94 1800BFAF */  sw         $ra, 0x18($sp)
    /* 4C698 8005BE98 3416020C */  jal        func_800858D0
    /* 4C69C 8005BE9C 1400B1AF */   sw        $s1, 0x14($sp)
    /* 4C6A0 8005BEA0 0A80033C */  lui        $v1, %hi(D_8009AD1C)
    /* 4C6A4 8005BEA4 1CAD6324 */  addiu      $v1, $v1, %lo(D_8009AD1C)
    /* 4C6A8 8005BEA8 80101000 */  sll        $v0, $s0, 2
    /* 4C6AC 8005BEAC 21884300 */  addu       $s1, $v0, $v1
    /* 4C6B0 8005BEB0 00002286 */  lh         $v0, 0x0($s1)
    /* 4C6B4 8005BEB4 00000000 */  nop
    /* 4C6B8 8005BEB8 17004004 */  bltz       $v0, .L8005BF18
    /* 4C6BC 8005BEBC 40801000 */   sll       $s0, $s0, 1
    /* 4C6C0 8005BEC0 E617020C */  jal        func_80085F98
    /* 4C6C4 8005BEC4 00000000 */   nop
    /* 4C6C8 8005BEC8 B917020C */  jal        func_80085EE4
    /* 4C6CC 8005BECC 21200000 */   addu      $a0, $zero, $zero
    /* 4C6D0 8005BED0 21200000 */  addu       $a0, $zero, $zero
    /* 4C6D4 8005BED4 9317020C */  jal        func_80085E4C
    /* 4C6D8 8005BED8 21280000 */   addu      $a1, $zero, $zero
    /* 4C6DC 8005BEDC 00002486 */  lh         $a0, 0x0($s1)
    /* 4C6E0 8005BEE0 B917020C */  jal        func_80085EE4
    /* 4C6E4 8005BEE4 00000000 */   nop
    /* 4C6E8 8005BEE8 00002486 */  lh         $a0, 0x0($s1)
    /* 4C6EC 8005BEEC 4A2A020C */  jal        func_8008A928
    /* 4C6F0 8005BEF0 21884000 */   addu      $s1, $v0, $zero
    /* 4C6F4 8005BEF4 01000426 */  addiu      $a0, $s0, 0x1
    /* 4C6F8 8005BEF8 00240400 */  sll        $a0, $a0, 16
    /* 4C6FC 8005BEFC 03240400 */  sra        $a0, $a0, 16
    /* 4C700 8005BF00 9317020C */  jal        func_80085E4C
    /* 4C704 8005BF04 21288000 */   addu      $a1, $a0, $zero
    /* 4C708 8005BF08 EE17020C */  jal        func_80085FB8
    /* 4C70C 8005BF0C 00000000 */   nop
    /* 4C710 8005BF10 C86F0108 */  j          .L8005BF20
    /* 4C714 8005BF14 00141100 */   sll       $v0, $s1, 16
  .L8005BF18:
    /* 4C718 8005BF18 FFFF1124 */  addiu      $s1, $zero, -0x1
    /* 4C71C 8005BF1C 00141100 */  sll        $v0, $s1, 16
  .L8005BF20:
    /* 4C720 8005BF20 03140200 */  sra        $v0, $v0, 16
    /* 4C724 8005BF24 1800BF8F */  lw         $ra, 0x18($sp)
    /* 4C728 8005BF28 1400B18F */  lw         $s1, 0x14($sp)
    /* 4C72C 8005BF2C 1000B08F */  lw         $s0, 0x10($sp)
    /* 4C730 8005BF30 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 4C734 8005BF34 0800E003 */  jr         $ra
    /* 4C738 8005BF38 00000000 */   nop
endlabel func_8005BE84
