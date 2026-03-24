glabel func_8008AD64
    /* 7B564 8008AD64 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7B568 8008AD68 1000B0AF */  sw         $s0, 0x10($sp)
    /* 7B56C 8008AD6C 2180A000 */  addu       $s0, $a1, $zero
    /* 7B570 8008AD70 0700023C */  lui        $v0, (0x7EFF0 >> 16)
    /* 7B574 8008AD74 F0EF4234 */  ori        $v0, $v0, (0x7EFF0 & 0xFFFF)
    /* 7B578 8008AD78 2B105000 */  sltu       $v0, $v0, $s0
    /* 7B57C 8008AD7C 03004010 */  beqz       $v0, .L8008AD8C
    /* 7B580 8008AD80 1400BFAF */   sw        $ra, 0x14($sp)
    /* 7B584 8008AD84 0700103C */  lui        $s0, (0x7EFF0 >> 16)
    /* 7B588 8008AD88 F0EF1036 */  ori        $s0, $s0, (0x7EFF0 & 0xFFFF)
  .L8008AD8C:
    /* 7B58C 8008AD8C 0924020C */  jal        func_80089024
    /* 7B590 8008AD90 21280002 */   addu      $a1, $s0, $zero
    /* 7B594 8008AD94 0A80023C */  lui        $v0, %hi(D_800A2D14)
    /* 7B598 8008AD98 142D428C */  lw         $v0, %lo(D_800A2D14)($v0)
    /* 7B59C 8008AD9C 00000000 */  nop
    /* 7B5A0 8008ADA0 03004014 */  bnez       $v0, .L8008ADB0
    /* 7B5A4 8008ADA4 21100002 */   addu      $v0, $s0, $zero
    /* 7B5A8 8008ADA8 0A80013C */  lui        $at, %hi(D_800A2D10)
    /* 7B5AC 8008ADAC 102D20AC */  sw         $zero, %lo(D_800A2D10)($at)
  .L8008ADB0:
    /* 7B5B0 8008ADB0 1400BF8F */  lw         $ra, 0x14($sp)
    /* 7B5B4 8008ADB4 1000B08F */  lw         $s0, 0x10($sp)
    /* 7B5B8 8008ADB8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7B5BC 8008ADBC 0800E003 */  jr         $ra
    /* 7B5C0 8008ADC0 00000000 */   nop
endlabel func_8008AD64
