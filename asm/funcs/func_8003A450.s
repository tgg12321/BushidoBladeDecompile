glabel func_8003A450
    /* 2AC50 8003A450 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2AC54 8003A454 00F2043C */  lui        $a0, (0xF2000001 >> 16)
    /* 2AC58 8003A458 01008434 */  ori        $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 2AC5C 8003A45C 1800BFAF */  sw         $ra, 0x18($sp)
    /* 2AC60 8003A460 1400B1AF */  sw         $s1, 0x14($sp)
    /* 2AC64 8003A464 C1E2010C */  jal        func_80078B04
    /* 2AC68 8003A468 1000B0AF */   sw        $s0, 0x10($sp)
    /* 2AC6C 8003A46C 21884000 */  addu       $s1, $v0, $zero
    /* 2AC70 8003A470 0104222A */  slti       $v0, $s1, 0x401
    /* 2AC74 8003A474 04004014 */  bnez       $v0, .L8003A488
    /* 2AC78 8003A478 00F2043C */   lui       $a0, (0xF2000001 >> 16)
    /* 2AC7C 8003A47C EAE2010C */  jal        func_80078BA8
    /* 2AC80 8003A480 01008434 */   ori       $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 2AC84 8003A484 21880000 */  addu       $s1, $zero, $zero
  .L8003A488:
    /* 2AC88 8003A488 03000424 */  addiu      $a0, $zero, 0x3
  .L8003A48C:
    /* 2AC8C 8003A48C 01000524 */  addiu      $a1, $zero, 0x1
    /* 2AC90 8003A490 1931020C */  jal        func_8008C464
    /* 2AC94 8003A494 21300000 */   addu      $a2, $zero, $zero
    /* 2AC98 8003A498 09004014 */  bnez       $v0, .L8003A4C0
    /* 2AC9C 8003A49C 00F2043C */   lui       $a0, (0xF2000001 >> 16)
    /* 2ACA0 8003A4A0 C1E2010C */  jal        func_80078B04
    /* 2ACA4 8003A4A4 01008434 */   ori       $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 2ACA8 8003A4A8 23105100 */  subu       $v0, $v0, $s1
    /* 2ACAC 8003A4AC 01784228 */  slti       $v0, $v0, 0x7801
    /* 2ACB0 8003A4B0 F6FF4014 */  bnez       $v0, .L8003A48C
    /* 2ACB4 8003A4B4 03000424 */   addiu     $a0, $zero, 0x3
    /* 2ACB8 8003A4B8 57E90008 */  j          .L8003A55C
    /* 2ACBC 8003A4BC 21100000 */   addu      $v0, $zero, $zero
  .L8003A4C0:
    /* 2ACC0 8003A4C0 21800000 */  addu       $s0, $zero, $zero
    /* 2ACC4 8003A4C4 03000424 */  addiu      $a0, $zero, 0x3
  .L8003A4C8:
    /* 2ACC8 8003A4C8 01000524 */  addiu      $a1, $zero, 0x1
    /* 2ACCC 8003A4CC 1931020C */  jal        func_8008C464
    /* 2ACD0 8003A4D0 21300000 */   addu      $a2, $zero, $zero
    /* 2ACD4 8003A4D4 06004010 */  beqz       $v0, .L8003A4F0
    /* 2ACD8 8003A4D8 E803022A */   slti      $v0, $s0, 0x3E8
    /* 2ACDC 8003A4DC 01001026 */  addiu      $s0, $s0, 0x1
    /* 2ACE0 8003A4E0 E803022A */  slti       $v0, $s0, 0x3E8
    /* 2ACE4 8003A4E4 F8FF4014 */  bnez       $v0, .L8003A4C8
    /* 2ACE8 8003A4E8 03000424 */   addiu     $a0, $zero, 0x3
    /* 2ACEC 8003A4EC E803022A */  slti       $v0, $s0, 0x3E8
  .L8003A4F0:
    /* 2ACF0 8003A4F0 E6FF4014 */  bnez       $v0, .L8003A48C
    /* 2ACF4 8003A4F4 03000424 */   addiu     $a0, $zero, 0x3
    /* 2ACF8 8003A4F8 01000424 */  addiu      $a0, $zero, 0x1
    /* 2ACFC 8003A4FC 01000524 */  addiu      $a1, $zero, 0x1
    /* 2AD00 8003A500 1931020C */  jal        func_8008C464
    /* 2AD04 8003A504 01000624 */   addiu     $a2, $zero, 0x1
    /* 2AD08 8003A508 04000424 */  addiu      $a0, $zero, 0x4
    /* 2AD0C 8003A50C 0480063C */  lui        $a2, %hi(func_8003A42C)
    /* 2AD10 8003A510 2CA4C624 */  addiu      $a2, $a2, %lo(func_8003A42C)
    /* 2AD14 8003A514 01000224 */  addiu      $v0, $zero, 0x1
    /* 2AD18 8003A518 600782A3 */  sb         $v0, %gp_rel(D_800A382C)($gp)
    /* 2AD1C 8003A51C 1931020C */  jal        func_8008C464
    /* 2AD20 8003A520 21280000 */   addu      $a1, $zero, $zero
    /* 2AD24 8003A524 0A80053C */  lui        $a1, %hi(D_800A3698)
    /* 2AD28 8003A528 9836A524 */  addiu      $a1, $a1, %lo(D_800A3698)
    /* 2AD2C 8003A52C 7006848F */  lw         $a0, %gp_rel(D_800A373C)($gp)
    /* 2AD30 8003A530 82E2010C */  jal        func_80078A08
    /* 2AD34 8003A534 08000624 */   addiu     $a2, $zero, 0x8
    /* 2AD38 8003A538 04000424 */  addiu      $a0, $zero, 0x4
    /* 2AD3C 8003A53C 21280000 */  addu       $a1, $zero, $zero
    /* 2AD40 8003A540 1931020C */  jal        func_8008C464
    /* 2AD44 8003A544 21300000 */   addu      $a2, $zero, $zero
    /* 2AD48 8003A548 01000424 */  addiu      $a0, $zero, 0x1
    /* 2AD4C 8003A54C 01000524 */  addiu      $a1, $zero, 0x1
    /* 2AD50 8003A550 1931020C */  jal        func_8008C464
    /* 2AD54 8003A554 21300000 */   addu      $a2, $zero, $zero
    /* 2AD58 8003A558 60078293 */  lbu        $v0, %gp_rel(D_800A382C)($gp)
  .L8003A55C:
    /* 2AD5C 8003A55C 1800BF8F */  lw         $ra, 0x18($sp)
    /* 2AD60 8003A560 1400B18F */  lw         $s1, 0x14($sp)
    /* 2AD64 8003A564 1000B08F */  lw         $s0, 0x10($sp)
    /* 2AD68 8003A568 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 2AD6C 8003A56C 0800E003 */  jr         $ra
    /* 2AD70 8003A570 00000000 */   nop
endlabel func_8003A450
