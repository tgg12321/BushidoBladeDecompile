glabel func_8001A484
    /* AC84 8001A484 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* AC88 8001A488 1400B1AF */  sw         $s1, 0x14($sp)
    /* AC8C 8001A48C 21888000 */  addu       $s1, $a0, $zero
    /* AC90 8001A490 1800B2AF */  sw         $s2, 0x18($sp)
    /* AC94 8001A494 21900000 */  addu       $s2, $zero, $zero
    /* AC98 8001A498 1000B0AF */  sw         $s0, 0x10($sp)
    /* AC9C 8001A49C 04003026 */  addiu      $s0, $s1, 0x4
    /* ACA0 8001A4A0 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* ACA4 8001A4A4 01005226 */  addiu      $s2, $s2, 0x1
  .L8001A4A8:
    /* ACA8 8001A4A8 FEFF0696 */  lhu        $a2, -0x2($s0)
    /* ACAC 8001A4AC 00000796 */  lhu        $a3, 0x0($s0)
    /* ACB0 8001A4B0 06001026 */  addiu      $s0, $s0, 0x6
    /* ACB4 8001A4B4 00002596 */  lhu        $a1, 0x0($s1)
    /* ACB8 8001A4B8 0180043C */  lui        $a0, %hi(D_800100A4)
    /* ACBC 8001A4BC A4008424 */  addiu      $a0, $a0, %lo(D_800100A4)
    /* ACC0 8001A4C0 4BF5000C */  jal        DispSleepMenuTex
    /* ACC4 8001A4C4 06003126 */   addiu     $s1, $s1, 0x6
    /* ACC8 8001A4C8 1600422A */  slti       $v0, $s2, 0x16
    /* ACCC 8001A4CC F6FF4014 */  bnez       $v0, .L8001A4A8
    /* ACD0 8001A4D0 01005226 */   addiu     $s2, $s2, 0x1
    /* ACD4 8001A4D4 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* ACD8 8001A4D8 1800B28F */  lw         $s2, 0x18($sp)
    /* ACDC 8001A4DC 1400B18F */  lw         $s1, 0x14($sp)
    /* ACE0 8001A4E0 1000B08F */  lw         $s0, 0x10($sp)
    /* ACE4 8001A4E4 2000BD27 */  addiu      $sp, $sp, 0x20
    /* ACE8 8001A4E8 0800E003 */  jr         $ra
    /* ACEC 8001A4EC 00000000 */   nop
endlabel func_8001A484
