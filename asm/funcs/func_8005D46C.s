glabel func_8005D46C
    /* 4DC6C 8005D46C A8FFBD27 */  addiu      $sp, $sp, -0x58
    /* 4DC70 8005D470 21188000 */  addu       $v1, $a0, $zero
    /* 4DC74 8005D474 2110A000 */  addu       $v0, $a1, $zero
    /* 4DC78 8005D478 5000BFAF */  sw         $ra, 0x50($sp)
    /* 4DC7C 8005D47C 4C00B3AF */  sw         $s3, 0x4C($sp)
    /* 4DC80 8005D480 4800B2AF */  sw         $s2, 0x48($sp)
    /* 4DC84 8005D484 4400B1AF */  sw         $s1, 0x44($sp)
    /* 4DC88 8005D488 02004018 */  blez       $v0, .L8005D494
    /* 4DC8C 8005D48C 4000B0AF */   sw        $s0, 0x40($sp)
    /* 4DC90 8005D490 FFFF4224 */  addiu      $v0, $v0, -0x1
  .L8005D494:
    /* 4DC94 8005D494 1000A427 */  addiu      $a0, $sp, 0x10
    /* 4DC98 8005D498 21280000 */  addu       $a1, $zero, $zero
    /* 4DC9C 8005D49C 00810200 */  sll        $s0, $v0, 4
    /* 4DCA0 8005D4A0 23800202 */  subu       $s0, $s0, $v0
    /* 4DCA4 8005D4A4 80801000 */  sll        $s0, $s0, 2
    /* 4DCA8 8005D4A8 0A80113C */  lui        $s1, %hi(D_8009B2C8)
    /* 4DCAC 8005D4AC C8B23126 */  addiu      $s1, $s1, %lo(D_8009B2C8)
    /* 4DCB0 8005D4B0 21101102 */  addu       $v0, $s0, $s1
    /* 4DCB4 8005D4B4 1000A2AF */  sw         $v0, 0x10($sp)
    /* 4DCB8 8005D4B8 0A80023C */  lui        $v0, %hi(D_8009B340)
    /* 4DCBC 8005D4BC 40B34224 */  addiu      $v0, $v0, %lo(D_8009B340)
    /* 4DCC0 8005D4C0 00011224 */  addiu      $s2, $zero, 0x100
    /* 4DCC4 8005D4C4 01001324 */  addiu      $s3, $zero, 0x1
    /* 4DCC8 8005D4C8 3800A0A3 */  sb         $zero, 0x38($sp)
    /* 4DCCC 8005D4CC 1400A2AF */  sw         $v0, 0x14($sp)
    /* 4DCD0 8005D4D0 3400B2AF */  sw         $s2, 0x34($sp)
    /* 4DCD4 8005D4D4 3000B2AF */  sw         $s2, 0x30($sp)
    /* 4DCD8 8005D4D8 2C00A0AF */  sw         $zero, 0x2C($sp)
    /* 4DCDC 8005D4DC 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4DCE0 8005D4E0 2000A0AF */  sw         $zero, 0x20($sp)
    /* 4DCE4 8005D4E4 2400B3AF */  sw         $s3, 0x24($sp)
    /* 4DCE8 8005D4E8 CACD010C */  jal        func_80073728
    /* 4DCEC 8005D4EC 1C00A3AF */   sw        $v1, 0x1C($sp)
    /* 4DCF0 8005D4F0 1000A427 */  addiu      $a0, $sp, 0x10
    /* 4DCF4 8005D4F4 21280000 */  addu       $a1, $zero, $zero
    /* 4DCF8 8005D4F8 0C003126 */  addiu      $s1, $s1, 0xC
    /* 4DCFC 8005D4FC 21801102 */  addu       $s0, $s0, $s1
    /* 4DD00 8005D500 0A80033C */  lui        $v1, %hi(D_8009B358)
    /* 4DD04 8005D504 58B36324 */  addiu      $v1, $v1, %lo(D_8009B358)
    /* 4DD08 8005D508 3800A0A3 */  sb         $zero, 0x38($sp)
    /* 4DD0C 8005D50C 1000B0AF */  sw         $s0, 0x10($sp)
    /* 4DD10 8005D510 1400A3AF */  sw         $v1, 0x14($sp)
    /* 4DD14 8005D514 3400B2AF */  sw         $s2, 0x34($sp)
    /* 4DD18 8005D518 3000B2AF */  sw         $s2, 0x30($sp)
    /* 4DD1C 8005D51C 2C00A0AF */  sw         $zero, 0x2C($sp)
    /* 4DD20 8005D520 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4DD24 8005D524 2000A0AF */  sw         $zero, 0x20($sp)
    /* 4DD28 8005D528 2400B3AF */  sw         $s3, 0x24($sp)
    /* 4DD2C 8005D52C CACD010C */  jal        func_80073728
    /* 4DD30 8005D530 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 4DD34 8005D534 5000BF8F */  lw         $ra, 0x50($sp)
    /* 4DD38 8005D538 4C00B38F */  lw         $s3, 0x4C($sp)
    /* 4DD3C 8005D53C 4800B28F */  lw         $s2, 0x48($sp)
    /* 4DD40 8005D540 4400B18F */  lw         $s1, 0x44($sp)
    /* 4DD44 8005D544 4000B08F */  lw         $s0, 0x40($sp)
    /* 4DD48 8005D548 5800BD27 */  addiu      $sp, $sp, 0x58
    /* 4DD4C 8005D54C 0800E003 */  jr         $ra
    /* 4DD50 8005D550 00000000 */   nop
endlabel func_8005D46C
