glabel func_8003C42C
    /* 2CC2C 8003C42C C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 2CC30 8003C430 0A80023C */  lui        $v0, %hi(D_800A389B)
    /* 2CC34 8003C434 9B384290 */  lbu        $v0, %lo(D_800A389B)($v0)
    /* 2CC38 8003C438 21380000 */  addu       $a3, $zero, $zero
    /* 2CC3C 8003C43C 3000BFAF */  sw         $ra, 0x30($sp)
    /* 2CC40 8003C440 1800A0AF */  sw         $zero, 0x18($sp)
    /* 2CC44 8003C444 1400A0AF */  sw         $zero, 0x14($sp)
    /* 2CC48 8003C448 10004018 */  blez       $v0, .L8003C48C
    /* 2CC4C 8003C44C 1000A0AF */   sw        $zero, 0x10($sp)
    /* 2CC50 8003C450 1000A527 */  addiu      $a1, $sp, 0x10
    /* 2CC54 8003C454 21204000 */  addu       $a0, $v0, $zero
  .L8003C458:
    /* 2CC58 8003C458 0A80013C */  lui        $at, %hi(D_800A377C)
    /* 2CC5C 8003C45C 21082700 */  addu       $at, $at, $a3
    /* 2CC60 8003C460 7C372290 */  lbu        $v0, %lo(D_800A377C)($at)
    /* 2CC64 8003C464 00000000 */  nop
    /* 2CC68 8003C468 80100200 */  sll        $v0, $v0, 2
    /* 2CC6C 8003C46C 21104500 */  addu       $v0, $v0, $a1
    /* 2CC70 8003C470 0000438C */  lw         $v1, 0x0($v0)
    /* 2CC74 8003C474 0100E724 */  addiu      $a3, $a3, 0x1
    /* 2CC78 8003C478 01006324 */  addiu      $v1, $v1, 0x1
    /* 2CC7C 8003C47C 000043AC */  sw         $v1, 0x0($v0)
    /* 2CC80 8003C480 2A10E400 */  slt        $v0, $a3, $a0
    /* 2CC84 8003C484 F4FF4014 */  bnez       $v0, .L8003C458
    /* 2CC88 8003C488 00000000 */   nop
  .L8003C48C:
    /* 2CC8C 8003C48C 1000A28F */  lw         $v0, 0x10($sp)
    /* 2CC90 8003C490 1400A38F */  lw         $v1, 0x14($sp)
    /* 2CC94 8003C494 00000000 */  nop
    /* 2CC98 8003C498 21004314 */  bne        $v0, $v1, .L8003C520
    /* 2CC9C 8003C49C 2A104300 */   slt       $v0, $v0, $v1
    /* 2CCA0 8003C4A0 0A80023C */  lui        $v0, %hi(D_800A389B)
    /* 2CCA4 8003C4A4 9B384290 */  lbu        $v0, %lo(D_800A389B)($v0)
    /* 2CCA8 8003C4A8 21380000 */  addu       $a3, $zero, $zero
    /* 2CCAC 8003C4AC 1400A0AF */  sw         $zero, 0x14($sp)
    /* 2CCB0 8003C4B0 15004018 */  blez       $v0, .L8003C508
    /* 2CCB4 8003C4B4 1000A0AF */   sw        $zero, 0x10($sp)
    /* 2CCB8 8003C4B8 1000AA27 */  addiu      $t2, $sp, 0x10
    /* 2CCBC 8003C4BC 21484000 */  addu       $t1, $v0, $zero
    /* 2CCC0 8003C4C0 0F80083C */  lui        $t0, %hi(D_800F65F8)
    /* 2CCC4 8003C4C4 F8650825 */  addiu      $t0, $t0, %lo(D_800F65F8)
  .L8003C4C8:
    /* 2CCC8 8003C4C8 21300000 */  addu       $a2, $zero, $zero
    /* 2CCCC 8003C4CC 21204001 */  addu       $a0, $t2, $zero
    /* 2CCD0 8003C4D0 21280001 */  addu       $a1, $t0, $zero
  .L8003C4D4:
    /* 2CCD4 8003C4D4 0000A290 */  lbu        $v0, 0x0($a1)
    /* 2CCD8 8003C4D8 0100A524 */  addiu      $a1, $a1, 0x1
    /* 2CCDC 8003C4DC 0000838C */  lw         $v1, 0x0($a0)
    /* 2CCE0 8003C4E0 0100C624 */  addiu      $a2, $a2, 0x1
    /* 2CCE4 8003C4E4 21186200 */  addu       $v1, $v1, $v0
    /* 2CCE8 8003C4E8 000083AC */  sw         $v1, 0x0($a0)
    /* 2CCEC 8003C4EC 0200C228 */  slti       $v0, $a2, 0x2
    /* 2CCF0 8003C4F0 F8FF4014 */  bnez       $v0, .L8003C4D4
    /* 2CCF4 8003C4F4 04008424 */   addiu     $a0, $a0, 0x4
    /* 2CCF8 8003C4F8 0100E724 */  addiu      $a3, $a3, 0x1
    /* 2CCFC 8003C4FC 2A10E900 */  slt        $v0, $a3, $t1
    /* 2CD00 8003C500 F1FF4014 */  bnez       $v0, .L8003C4C8
    /* 2CD04 8003C504 02000825 */   addiu     $t0, $t0, 0x2
  .L8003C508:
    /* 2CD08 8003C508 1000A28F */  lw         $v0, 0x10($sp)
    /* 2CD0C 8003C50C 1400A38F */  lw         $v1, 0x14($sp)
    /* 2CD10 8003C510 00000000 */  nop
    /* 2CD14 8003C514 02004314 */  bne        $v0, $v1, .L8003C520
    /* 2CD18 8003C518 2A104300 */   slt       $v0, $v0, $v1
    /* 2CD1C 8003C51C 02000224 */  addiu      $v0, $zero, 0x2
  .L8003C520:
    /* 2CD20 8003C520 0A80013C */  lui        $at, %hi(D_800A382D)
    /* 2CD24 8003C524 2D3822A0 */  sb         $v0, %lo(D_800A382D)($at)
    /* 2CD28 8003C528 01000424 */  addiu      $a0, $zero, 0x1
    /* 2CD2C 8003C52C 21280000 */  addu       $a1, $zero, $zero
    /* 2CD30 8003C530 21300000 */  addu       $a2, $zero, $zero
    /* 2CD34 8003C534 DA59000C */  jal        disp_SetFramebufferMode
    /* 2CD38 8003C538 21380000 */   addu      $a3, $zero, $zero
    /* 2CD3C 8003C53C 15000224 */  addiu      $v0, $zero, 0x15
    /* 2CD40 8003C540 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2CD44 8003C544 B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* 2CD48 8003C548 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2CD4C 8003C54C 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2CD50 8003C550 3000BF8F */  lw         $ra, 0x30($sp)
    /* 2CD54 8003C554 3800BD27 */  addiu      $sp, $sp, 0x38
    /* 2CD58 8003C558 0800E003 */  jr         $ra
    /* 2CD5C 8003C55C 00000000 */   nop
endlabel func_8003C42C
