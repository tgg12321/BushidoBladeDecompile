glabel func_8003D478
    /* 2DC78 8003D478 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 2DC7C 8003D47C 1000B0AF */  sw         $s0, 0x10($sp)
    /* 2DC80 8003D480 21808000 */  addu       $s0, $a0, $zero
    /* 2DC84 8003D484 1800B2AF */  sw         $s2, 0x18($sp)
    /* 2DC88 8003D488 2190A000 */  addu       $s2, $a1, $zero
    /* 2DC8C 8003D48C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 2DC90 8003D490 2188C000 */  addu       $s1, $a2, $zero
    /* 2DC94 8003D494 2000B4AF */  sw         $s4, 0x20($sp)
    /* 2DC98 8003D498 21A0E000 */  addu       $s4, $a3, $zero
    /* 2DC9C 8003D49C 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 2DCA0 8003D4A0 2800B6AF */  sw         $s6, 0x28($sp)
    /* 2DCA4 8003D4A4 2400B5AF */  sw         $s5, 0x24($sp)
    /* 2DCA8 8003D4A8 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 2DCAC 8003D4AC 00002692 */  lbu        $a2, 0x0($s1)
    /* 2DCB0 8003D4B0 01003126 */  addiu      $s1, $s1, 0x1
    /* 2DCB4 8003D4B4 1200C010 */  beqz       $a2, .L8003D500
    /* 2DCB8 8003D4B8 21980002 */   addu      $s3, $s0, $zero
    /* 2DCBC 8003D4BC 20001624 */  addiu      $s6, $zero, 0x20
    /* 2DCC0 8003D4C0 0A001524 */  addiu      $s5, $zero, 0xA
  .L8003D4C4:
    /* 2DCC4 8003D4C4 0900D610 */  beq        $a2, $s6, .L8003D4EC
    /* 2DCC8 8003D4C8 00000000 */   nop
    /* 2DCCC 8003D4CC 0400D514 */  bne        $a2, $s5, .L8003D4E0
    /* 2DCD0 8003D4D0 21200002 */   addu      $a0, $s0, $zero
    /* 2DCD4 8003D4D4 21806002 */  addu       $s0, $s3, $zero
    /* 2DCD8 8003D4D8 3CF50008 */  j          .L8003D4F0
    /* 2DCDC 8003D4DC 08005226 */   addiu     $s2, $s2, 0x8
  .L8003D4E0:
    /* 2DCE0 8003D4E0 21284002 */  addu       $a1, $s2, $zero
    /* 2DCE4 8003D4E4 E7F4000C */  jal        _McAccessSection
    /* 2DCE8 8003D4E8 21388002 */   addu      $a3, $s4, $zero
  .L8003D4EC:
    /* 2DCEC 8003D4EC 08001026 */  addiu      $s0, $s0, 0x8
  .L8003D4F0:
    /* 2DCF0 8003D4F0 00002692 */  lbu        $a2, 0x0($s1)
    /* 2DCF4 8003D4F4 00000000 */  nop
    /* 2DCF8 8003D4F8 F2FFC014 */  bnez       $a2, .L8003D4C4
    /* 2DCFC 8003D4FC 01003126 */   addiu     $s1, $s1, 0x1
  .L8003D500:
    /* 2DD00 8003D500 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 2DD04 8003D504 2800B68F */  lw         $s6, 0x28($sp)
    /* 2DD08 8003D508 2400B58F */  lw         $s5, 0x24($sp)
    /* 2DD0C 8003D50C 2000B48F */  lw         $s4, 0x20($sp)
    /* 2DD10 8003D510 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 2DD14 8003D514 1800B28F */  lw         $s2, 0x18($sp)
    /* 2DD18 8003D518 1400B18F */  lw         $s1, 0x14($sp)
    /* 2DD1C 8003D51C 1000B08F */  lw         $s0, 0x10($sp)
    /* 2DD20 8003D520 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 2DD24 8003D524 0800E003 */  jr         $ra
    /* 2DD28 8003D528 00000000 */   nop
endlabel func_8003D478
