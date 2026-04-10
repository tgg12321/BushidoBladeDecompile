glabel func_8007A4D8
    /* 6ACD8 8007A4D8 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 6ACDC 8007A4DC 2400B3AF */  sw         $s3, 0x24($sp)
    /* 6ACE0 8007A4E0 4000B38F */  lw         $s3, 0x40($sp)
    /* 6ACE4 8007A4E4 4400A38F */  lw         $v1, 0x44($sp)
    /* 6ACE8 8007A4E8 4800A28F */  lw         $v0, 0x48($sp)
    /* 6ACEC 8007A4EC 21408000 */  addu       $t0, $a0, $zero
    /* 6ACF0 8007A4F0 1800B0AF */  sw         $s0, 0x18($sp)
    /* 6ACF4 8007A4F4 2180A000 */  addu       $s0, $a1, $zero
    /* 6ACF8 8007A4F8 2000B2AF */  sw         $s2, 0x20($sp)
    /* 6ACFC 8007A4FC 2190C000 */  addu       $s2, $a2, $zero
    /* 6AD00 8007A500 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 6AD04 8007A504 2188E000 */  addu       $s1, $a3, $zero
    /* 6AD08 8007A508 2800BFAF */  sw         $ra, 0x28($sp)
    /* 6AD0C 8007A50C 1000B1A7 */  sh         $s1, 0x10($sp)
    /* 6AD10 8007A510 1600A2A7 */  sh         $v0, 0x16($sp)
    /* 6AD14 8007A514 01000224 */  addiu      $v0, $zero, 0x1
    /* 6AD18 8007A518 13000212 */  beq        $s0, $v0, .L8007A568
    /* 6AD1C 8007A51C 1200B3A7 */   sh        $s3, 0x12($sp)
    /* 6AD20 8007A520 0200022A */  slti       $v0, $s0, 0x2
    /* 6AD24 8007A524 05004010 */  beqz       $v0, .L8007A53C
    /* 6AD28 8007A528 00000000 */   nop
    /* 6AD2C 8007A52C 08000012 */  beqz       $s0, .L8007A550
    /* 6AD30 8007A530 1000A427 */   addiu     $a0, $sp, 0x10
    /* 6AD34 8007A534 61E90108 */  j          .L8007A584
    /* 6AD38 8007A538 00000000 */   nop
  .L8007A53C:
    /* 6AD3C 8007A53C 02000224 */  addiu      $v0, $zero, 0x2
    /* 6AD40 8007A540 0E000212 */  beq        $s0, $v0, .L8007A57C
    /* 6AD44 8007A544 1000A427 */   addiu     $a0, $sp, 0x10
    /* 6AD48 8007A548 61E90108 */  j          .L8007A584
    /* 6AD4C 8007A54C 00000000 */   nop
  .L8007A550:
    /* 6AD50 8007A550 02006104 */  bgez       $v1, .L8007A55C
    /* 6AD54 8007A554 21106000 */   addu      $v0, $v1, $zero
    /* 6AD58 8007A558 03006224 */  addiu      $v0, $v1, 0x3
  .L8007A55C:
    /* 6AD5C 8007A55C 83100200 */  sra        $v0, $v0, 2
    /* 6AD60 8007A560 60E90108 */  j          .L8007A580
    /* 6AD64 8007A564 1400A2A7 */   sh        $v0, 0x14($sp)
  .L8007A568:
    /* 6AD68 8007A568 C2170300 */  srl        $v0, $v1, 31
    /* 6AD6C 8007A56C 21106200 */  addu       $v0, $v1, $v0
    /* 6AD70 8007A570 43100200 */  sra        $v0, $v0, 1
    /* 6AD74 8007A574 60E90108 */  j          .L8007A580
    /* 6AD78 8007A578 1400A2A7 */   sh        $v0, 0x14($sp)
  .L8007A57C:
    /* 6AD7C 8007A57C 1400A3A7 */  sh         $v1, 0x14($sp)
  .L8007A580:
    /* 6AD80 8007A580 1000A427 */  addiu      $a0, $sp, 0x10
  .L8007A584:
    /* 6AD84 8007A584 80ED010C */  jal        gpu_LoadImage
    /* 6AD88 8007A588 21280001 */   addu      $a1, $t0, $zero
    /* 6AD8C 8007A58C 21200002 */  addu       $a0, $s0, $zero
    /* 6AD90 8007A590 21284002 */  addu       $a1, $s2, $zero
    /* 6AD94 8007A594 21302002 */  addu       $a2, $s1, $zero
    /* 6AD98 8007A598 E2E9010C */  jal        gpu_CalcTPage
    /* 6AD9C 8007A59C 21386002 */   addu      $a3, $s3, $zero
    /* 6ADA0 8007A5A0 FFFF4230 */  andi       $v0, $v0, 0xFFFF
    /* 6ADA4 8007A5A4 2800BF8F */  lw         $ra, 0x28($sp)
    /* 6ADA8 8007A5A8 2400B38F */  lw         $s3, 0x24($sp)
    /* 6ADAC 8007A5AC 2000B28F */  lw         $s2, 0x20($sp)
    /* 6ADB0 8007A5B0 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 6ADB4 8007A5B4 1800B08F */  lw         $s0, 0x18($sp)
    /* 6ADB8 8007A5B8 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 6ADBC 8007A5BC 0800E003 */  jr         $ra
    /* 6ADC0 8007A5C0 00000000 */   nop
endlabel func_8007A4D8
