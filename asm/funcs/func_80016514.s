glabel func_80016514
    /* 6D14 80016514 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6D18 80016518 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6D1C 8001651C 2198A000 */  addu       $s3, $a1, $zero
    /* 6D20 80016520 04008424 */  addiu      $a0, $a0, 0x4
    /* 6D24 80016524 21280000 */  addu       $a1, $zero, $zero
    /* 6D28 80016528 21300000 */  addu       $a2, $zero, $zero
    /* 6D2C 8001652C 2400BFAF */  sw         $ra, 0x24($sp)
    /* 6D30 80016530 2000B4AF */  sw         $s4, 0x20($sp)
    /* 6D34 80016534 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6D38 80016538 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6D3C 8001653C A60D020C */  jal        func_80083698
    /* 6D40 80016540 1000B0AF */   sw        $s0, 0x10($sp)
    /* 6D44 80016544 21904000 */  addu       $s2, $v0, $zero
    /* 6D48 80016548 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 6D4C 8001654C 07004216 */  bne        $s2, $v0, .L8001656C
    /* 6D50 80016550 21204002 */   addu      $a0, $s2, $zero
    /* 6D54 80016554 75590008 */  j          .L800165D4
    /* 6D58 80016558 FEFF0224 */   addiu     $v0, $zero, -0x2
  .L8001655C:
    /* 6D5C 8001655C 86E2010C */  jal        func_80078A18
    /* 6D60 80016560 21204002 */   addu      $a0, $s2, $zero
    /* 6D64 80016564 75590008 */  j          .L800165D4
    /* 6D68 80016568 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8001656C:
    /* 6D6C 8001656C 21280000 */  addu       $a1, $zero, $zero
    /* 6D70 80016570 B20D020C */  jal        func_800836C8
    /* 6D74 80016574 02000624 */   addiu     $a2, $zero, 0x2
    /* 6D78 80016578 21A04000 */  addu       $s4, $v0, $zero
    /* 6D7C 8001657C 21888002 */  addu       $s1, $s4, $zero
    /* 6D80 80016580 21204002 */  addu       $a0, $s2, $zero
    /* 6D84 80016584 21280000 */  addu       $a1, $zero, $zero
    /* 6D88 80016588 B20D020C */  jal        func_800836C8
    /* 6D8C 8001658C 21300000 */   addu      $a2, $zero, $zero
    /* 6D90 80016590 0D00801A */  blez       $s4, .L800165C8
    /* 6D94 80016594 00000000 */   nop
  .L80016598:
    /* 6D98 80016598 0140222A */  slti       $v0, $s1, 0x4001
    /* 6D9C 8001659C 02004010 */  beqz       $v0, .L800165A8
    /* 6DA0 800165A0 00401024 */   addiu     $s0, $zero, 0x4000
    /* 6DA4 800165A4 21802002 */  addu       $s0, $s1, $zero
  .L800165A8:
    /* 6DA8 800165A8 21204002 */  addu       $a0, $s2, $zero
    /* 6DAC 800165AC 21286002 */  addu       $a1, $s3, $zero
    /* 6DB0 800165B0 1F0E020C */  jal        func_8008387C
    /* 6DB4 800165B4 21300002 */   addu      $a2, $s0, $zero
    /* 6DB8 800165B8 E8FF5014 */  bne        $v0, $s0, .L8001655C
    /* 6DBC 800165BC 23883002 */   subu      $s1, $s1, $s0
    /* 6DC0 800165C0 F5FF201E */  bgtz       $s1, .L80016598
    /* 6DC4 800165C4 21987002 */   addu      $s3, $s3, $s0
  .L800165C8:
    /* 6DC8 800165C8 AE0D020C */  jal        func_800836B8
    /* 6DCC 800165CC 21204002 */   addu      $a0, $s2, $zero
    /* 6DD0 800165D0 21108002 */  addu       $v0, $s4, $zero
  .L800165D4:
    /* 6DD4 800165D4 2400BF8F */  lw         $ra, 0x24($sp)
    /* 6DD8 800165D8 2000B48F */  lw         $s4, 0x20($sp)
    /* 6DDC 800165DC 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6DE0 800165E0 1800B28F */  lw         $s2, 0x18($sp)
    /* 6DE4 800165E4 1400B18F */  lw         $s1, 0x14($sp)
    /* 6DE8 800165E8 1000B08F */  lw         $s0, 0x10($sp)
    /* 6DEC 800165EC 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6DF0 800165F0 0800E003 */  jr         $ra
    /* 6DF4 800165F4 00000000 */   nop
endlabel func_80016514
