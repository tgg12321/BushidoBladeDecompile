glabel func_8004A76C
    /* 3AF6C 8004A76C FFFF8424 */  addiu      $a0, $a0, -0x1
    /* 3AF70 8004A770 000000CA */  lwc2       $0, 0x0($s0)
    /* 3AF74 8004A774 040001CA */  lwc2       $1, 0x4($s0)
    /* 3AF78 8004A778 080002CA */  lwc2       $2, 0x8($s0)
    /* 3AF7C 8004A77C 0C0003CA */  lwc2       $3, 0xC($s0)
    /* 3AF80 8004A780 100004CA */  lwc2       $4, 0x10($s0)
    /* 3AF84 8004A784 140005CA */  lwc2       $5, 0x14($s0)
    /* 3AF88 8004A788 00000000 */  nop
    /* 3AF8C 8004A78C 00000000 */  nop
    /* 3AF90 8004A790 3000284A */  rtpt
    /* 3AF94 8004A794 EC290108 */  j          .L8004A7B0
    /* 3AF98 8004A798 18001026 */   addiu     $s0, $s0, 0x18
  .L8004A79C:
    /* 3AF9C 8004A79C 3000284A */  rtpt
    /* 3AFA0 8004A7A0 0600C624 */  addiu      $a2, $a2, 0x6
    /* 3AFA4 8004A7A4 FAFFC8A4 */  sh         $t0, -0x6($a2)
    /* 3AFA8 8004A7A8 FCFFC9A4 */  sh         $t1, -0x4($a2)
    /* 3AFAC 8004A7AC FEFFCAA4 */  sh         $t2, -0x2($a2)
  .L8004A7B0:
    /* 3AFB0 8004A7B0 0000ACE8 */  swc2       $12, 0x0($a1)
    /* 3AFB4 8004A7B4 0400ADE8 */  swc2       $13, 0x4($a1)
    /* 3AFB8 8004A7B8 0800AEE8 */  swc2       $14, 0x8($a1)
    /* 3AFBC 8004A7BC 00880848 */  mfc2       $t0, $17 /* handwritten instruction */
    /* 3AFC0 8004A7C0 00900948 */  mfc2       $t1, $18 /* handwritten instruction */
    /* 3AFC4 8004A7C4 00980A48 */  mfc2       $t2, $19 /* handwritten instruction */
    /* 3AFC8 8004A7C8 0C00A524 */  addiu      $a1, $a1, 0xC
    /* 3AFCC 8004A7CC 09008010 */  beqz       $a0, .L8004A7F4
    /* 3AFD0 8004A7D0 FFFF8424 */   addiu     $a0, $a0, -0x1
    /* 3AFD4 8004A7D4 000000CA */  lwc2       $0, 0x0($s0)
    /* 3AFD8 8004A7D8 040001CA */  lwc2       $1, 0x4($s0)
    /* 3AFDC 8004A7DC 080002CA */  lwc2       $2, 0x8($s0)
    /* 3AFE0 8004A7E0 0C0003CA */  lwc2       $3, 0xC($s0)
    /* 3AFE4 8004A7E4 100004CA */  lwc2       $4, 0x10($s0)
    /* 3AFE8 8004A7E8 140005CA */  lwc2       $5, 0x14($s0)
    /* 3AFEC 8004A7EC E7290108 */  j          .L8004A79C
    /* 3AFF0 8004A7F0 18001026 */   addiu     $s0, $s0, 0x18
  .L8004A7F4:
    /* 3AFF4 8004A7F4 0000C8A4 */  sh         $t0, 0x0($a2)
    /* 3AFF8 8004A7F8 0200C9A4 */  sh         $t1, 0x2($a2)
    /* 3AFFC 8004A7FC 0400CAA4 */  sh         $t2, 0x4($a2)
    /* 3B000 8004A800 0800E003 */  jr         $ra
    /* 3B004 8004A804 00000000 */   nop
endlabel func_8004A76C
